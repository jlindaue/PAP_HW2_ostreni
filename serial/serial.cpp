#include <stdio.h>
#include <iostream>
#include <fstream>

#define INPUT_IMAGE  "../vit_normal.ppm"
#define OUTPUT_IMAGE "../out_serial.ppm"


void readPPM(unsigned char* &input, unsigned char* &output, int& width, int& height){
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
    output = new unsigned char[width*height*3];


    for (int i = 1; i <= height; i++){
        inputFile.read((char *) input+3*i*widerWidth+3, width*3);
    }
    for (int i = 1;i<=height;i++){
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
    inputFile.close();
}

void writePPM(unsigned char* output, int width, int height){
    std::ofstream outputFile(OUTPUT_IMAGE, std::ofstream::out | std::ios::binary);
    outputFile << "P6\n" << width << "\n" << height << "\n" << "255\n";
    outputFile.write((char *) output, 3*width*height);
    outputFile.close();
}

void sharpen(unsigned char* input, unsigned char* output, int& width, int& height){
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

int main(int argc, char *argv[]) {
    struct timespec start, stop;
    double elapsed;
    int width, height;
    unsigned char* input;
    unsigned char* output;
    std::cout << "Reading input\n";
    readPPM(input, output, width, height);

    clock_gettime(CLOCK_REALTIME, &start);
    sharpen(input, output, width, height);
    clock_gettime(CLOCK_REALTIME, &stop);

    writePPM(output, width, height);
    elapsed = ( stop.tv_sec - start.tv_sec )*1000
              + (double)( stop.tv_nsec - start.tv_nsec )/(double)1000000;
    printf( " Execution time of observed program portion is %lf ms\n", elapsed);
    return 0;
}
