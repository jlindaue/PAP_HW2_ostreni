#!/bin/bash

cd mpi
make
cd ../openmp
make
cd ../mpi_openmp
make
cd ../serial
make
cd ..

rm time_*.txt
rm out_*.ppm

TEST_COUNT=1

#mpi
for i in 1 2 4
do
  echo "mpirun -np $i ./mpi/mpi >> time_mpi.txt"
  for j in {1..$TEST_COUNT}
  do
     mpirun -np $i ./mpi/mpi >> time_mpi${i}.txt
  done
done

#combination
for i in 1 2 4
do
  for k in 1 2 4 8
  do
    echo "OMP_NUM_THREADS=$k mpirun -np $i -x OMP_NUM_THREADS ./mpi_openmp/mpi_openmp >> time_openmp_mpi${i}_${k}.txt"
    for j in {1..$TEST_COUNT}
    do
       OMP_NUM_THREADS=$k mpirun -np $i -x OMP_NUM_THREADS ./mpi_openmp/mpi_openmp >> time_openmp_mpi${i}_${k}.txt
    done
  done
done

#openmp
for i in 1 2 4 8
do
  echo "OMP_NUM_THREADS=$i ./openmp/prog >> time_openmp${i}.txt"
  for j in {1..$TEST_COUNT}
  do
     OMP_NUM_THREADS=$i ./openmp/prog >> time_openmp${i}.txt
  done
done

#serial
echo "OMP_NUM_THREADS=$i ./serial/prog >> time_serial.txt"
for j in {1..$TEST_COUNT}
do
   ./serial/prog >> time_serial.txt
done
