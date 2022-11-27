import matplotlib.pyplot as plt
import numpy as np

path = "times2/"

def average(lst):
    return sum(lst) / len(lst)

def loadAverageTime(filename):
    with open(path+filename, "r") as f:
        lines = f.readlines()
    times = [float(x.strip()) for x in lines]
    return average(times)

def getData(method, cnts):
    x = []
    y = [] 
    for procs in cnts:
        avg = loadAverageTime("time_{}{}.txt".format(method, procs))
        x.append(procs)
        y.append(avg)
    return x,y


serialTime = loadAverageTime("time_serial.txt")

x,y = getData("openmp", [1,2,4,8])
y = [serialTime/el for el in y]
plt.scatter(x, y)
plt.ylabel("Speedup")
plt.xlabel("Number of threads")
plt.savefig('openmp.png')
plt.clf()

x,y = getData("mpi", [1,2,4,8])
y = [serialTime/el for el in y]
plt.ylabel("Speedup")
plt.xlabel("Number of processors")
plt.scatter(x, y)
plt.savefig('mpi.png')
plt.clf()

for pcs in [1,2,4]:
    x,y = getData("openmp_mpi{}_".format(pcs), [1,2,4,8])
    y = [serialTime/el for el in y]
    plt.scatter(x,y, label="{} processors".format(pcs))
plt.legend()
plt.ylabel("Speedup")
plt.xlabel("Number of threads")
plt.savefig('openmp_mpi.png')
plt.clf()

