#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <mpi.h>


#define INPUT_IMAGE  "../vit_normal.ppm"
#define OUTPUT_IMAGE "../out_mpi.ppm"
#define MASTER 0


void readPPM(unsigned char* &input, int& width, int& height){
    std::ifstream inputFile(INPUT_IMAGE, std::ios::in | std::ios::binary );
    std::string line;
    std::getline(inputFile,line);
    int precision;

    inputFile >> width;
    inputFile >> height;
    inputFile >> precision;
    std::getline(inputFile,line);

    std::cout << width << "x" << height << "\n";

    int widerWidth = width + 2;
    input = new unsigned char[(width+2)*(height+2)*3];


    for (int i = 1; i <= height; i++){
        inputFile.read((char *) input+3*i*widerWidth+3, width*3);
    }
    inputFile.close();

    for (int i = 1;i<=height;i++){  //bocni
        for (int k = 0; k < 3; k++) {
            input[3 * (i * widerWidth) + k] = input[3 * (i * widerWidth + 1) + k];
            input[3 * (i * widerWidth + width + 1) + k] = input[3 * (i * widerWidth + width) + k];
        }
    }
    for (int j = 1;j<=width;j++){
        for (int k = 0; k < 3; k++) {
            input[3 * (j) + k] = input[3 * (1*width + j) + k];
            input[3 * ((height+1)*widerWidth + j) + k] = input[3 * (height*widerWidth + j) + k];
        }
    }
}

void writePPM(unsigned char* output, int width, int height){
    std::ofstream outputFile(OUTPUT_IMAGE, std::ofstream::out | std::ios::binary);
    outputFile << "P6\n" << width << "\n" << height << "\n" << "255\n";
    outputFile.write((char *) output, 3*width*height);
    outputFile.close();
}

void sharpen(unsigned char* input, unsigned char* &output, int& width, int& height){
    output = new unsigned char[width*height*3];
    int widerWidth = width + 2;
    int tmp;
    for (int i = 1;i<=height;i++){
        for (int j = 1;j<=width;j++){
            for (int k = 0; k < 3; k++){
                tmp = 5*input[3*(i*widerWidth+j)+k]
                    - input[3*(i*widerWidth+j+1)+k]
                    - input[3*(i*widerWidth+j-1)+k]
                    - input[3*((i+1)*widerWidth+j)+k]
                    - input[3*((i-1)*widerWidth+j)+k];
                output[3*((i-1)*width+j-1)+k] = tmp>0? (tmp<255? (unsigned char)tmp:255):0;
            }
        }
    }
}

int distributeInput(
        unsigned char* input,
        unsigned char* &load,
        int worldSize,
        int width, int height) {

    std::vector<int> chunkSizes(worldSize, 0);
    std::vector<int> bases(worldSize, 0);
    int wideWidth = width + 2;
    int childChunkHeight = height / worldSize;
    int rootChunkHeight = height - (childChunkHeight * (worldSize - 1));
    int rootChunkSize=rootChunkHeight*wideWidth;
    int childChunkSize=childChunkHeight*wideWidth;
    chunkSizes[0] = 3*(rootChunkSize + 2 * wideWidth);
    for (int i = 1; i < worldSize; i++) {
        chunkSizes[i] = 3*(childChunkSize + 2 * wideWidth);
        bases[i] = 3*(rootChunkSize + (i - 1) * childChunkSize);
    }

    int sizes[] = {width, height, childChunkHeight};
    MPI_Bcast(&sizes, 3, MPI_INT, MASTER, MPI_COMM_WORLD);

    load = new unsigned char[chunkSizes[0]];
    MPI_Scatterv(
            input,
            chunkSizes.data(),
            bases.data(),
            MPI_UNSIGNED_CHAR,
            load,
            chunkSizes[0],
            MPI_UNSIGNED_CHAR,
            MASTER,
            MPI_COMM_WORLD);
    return rootChunkHeight;
}
int receiveInput(unsigned char* &load, int& width, int& height) {
    // Get the chunk size.
    int sizes[3];
    MPI_Bcast(sizes, 3, MPI_INT, MASTER, MPI_COMM_WORLD);
    width = sizes[0];
    height = sizes[1];
    int chunkHeight = sizes[2];
    int chunkSize = (chunkHeight + 2) * (width + 2) * 3;

    // Get the chunk of the input vector.
    load = new unsigned char[chunkSize];
    MPI_Scatterv(
            NULL,
            NULL,
            NULL,
            MPI_UNSIGNED_CHAR,
            load,
            chunkSize,
            MPI_UNSIGNED_CHAR,
            MASTER,
            MPI_COMM_WORLD);
    return chunkHeight;
}
void sendOutput(unsigned char* resultChunk, int chunkSize) {
    MPI_Gatherv(
            resultChunk,
            chunkSize,
            MPI_UNSIGNED_CHAR,
            NULL,
            NULL,
            NULL,
            MPI_UNSIGNED_CHAR,
            MASTER,
            MPI_COMM_WORLD);
}
void receiveOutput(
        unsigned char* resultChunk,
        unsigned char* output,
        int worldSize, int width, int height) {
    std::vector<int> chunkSizes(worldSize, 0);
    std::vector<int> bases(worldSize, 0);
    int childChunkSize = height / worldSize;
    int rootChunkSize = height - (childChunkSize * (worldSize - 1));
    rootChunkSize*=(3*width);
    childChunkSize*=(3*width);
    chunkSizes[0] = rootChunkSize;
    for (int i = 1; i < worldSize; i++) {
        chunkSizes[i] = childChunkSize;
        bases[i] = rootChunkSize + (i - 1) * childChunkSize;
    }

    MPI_Gatherv(
            resultChunk,
            rootChunkSize,
            MPI_UNSIGNED_CHAR,
            output,
            chunkSizes.data(),
            bases.data(),
            MPI_UNSIGNED_CHAR,
            MASTER,
            MPI_COMM_WORLD);
}


int main(int argc, char *argv[]) {
    int numprocs, rank, namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    double startwtime, endwtime;

    int width, height;
    unsigned char *chunkIn;
    unsigned char *chunkOut;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(processor_name, &namelen);

    if (rank == MASTER) {
        std::cout << "Reading input\n";
        unsigned char *input;
        unsigned char *output;
        readPPM(input, width, height);
        output = new unsigned char[width*height*3];
        startwtime = MPI_Wtime();

        int chunkHeight = distributeInput(input,chunkIn,numprocs,width,height);
        //std::cout << "(MASTER) SCATTERED " << chunkHeight << "\n";
        sharpen(chunkIn, chunkOut, width, chunkHeight);
        //std::cout << "COMPUTED\n";
        receiveOutput(chunkOut,output,numprocs,width,height);

        endwtime = MPI_Wtime();
        std::cout << "Writing result\n";
        writePPM(output, width, height);
        printf("spent time: %f s\n", endwtime - startwtime); fflush(stdout);
    }else{
        int chunkHeight = receiveInput(chunkIn, width, height);
        //std::cout << "(SLAVE) SCATTERED " << chunkHeight << ":"<< width << ":"<< height << "\n";
        sharpen(chunkIn, chunkOut, width, chunkHeight);
        //std::cout << "COMPUTED\n";
        sendOutput(chunkOut, chunkHeight*width*3);
    }

    MPI_Finalize();
    return 0;
}
