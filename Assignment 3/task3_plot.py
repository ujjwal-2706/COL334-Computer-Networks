from matplotlib import pyplot as plt
import numpy as np
data = np.loadtxt("task3.txt",delimiter=' ')
plt.plot(data[:,0],data[:,1])
plt.xlabel("Time of Simulation")
plt.ylabel("Congestion Window Size")
plt.title("Congestion Window Size Vs Time of Simulation")
plt.show()