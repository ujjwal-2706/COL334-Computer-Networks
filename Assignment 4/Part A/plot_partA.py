from matplotlib import pyplot as plt
import numpy as np
data_congestion_window1 = np.loadtxt("socket1.txt",delimiter=' ')
data_congestion_window2 =  np.loadtxt("socket2.txt",delimiter=' ')
data_congestion_window3 = np.loadtxt("socket3.txt",delimiter=' ')
max_window_size1 =  np.max(data_congestion_window1[:,1])
max_window_size2 =  np.max(data_congestion_window2[:,1])
max_window_size3 =  np.max(data_congestion_window3[:,1])

print(f"The maximum congestion window size at source 1 is : {max_window_size1}")
print(f"The maximum congestion window size at source 2 is : {max_window_size2}")
print(f"The maximum congestion window size at source 3 is : {max_window_size3}")

plt.plot(data_congestion_window1[:,0],data_congestion_window1[:,1])
plt.xlabel("Time of Simulation")
plt.ylabel("Congestion Window Size")
plt.title("TCP Congestion Window Vs Time of Simulation")
plt.savefig("tcp-connection1.png")
plt.clf()
plt.cla()
plt.plot(data_congestion_window2[:,0],data_congestion_window2[:,1])
plt.xlabel("Time of Simulation")
plt.ylabel("Congestion Window Size")
plt.title("TCP Congestion Window Vs Time of Simulation")
plt.savefig("tcp-connection2.png")
plt.clf()
plt.cla()
plt.plot(data_congestion_window3[:,0],data_congestion_window3[:,1])
plt.xlabel("Time of Simulation")
plt.ylabel("Congestion Window Size")
plt.title("TCP Congestion Window Vs Time of Simulation")
plt.savefig("tcp-connection3.png")

# for TcpNewReno we get socket1 -> 14749, socket2 -> 13500, socket3 -> 16958
# for TcpNewRenoPlus we get socket1 ->105358 , socket2 ->77512  and socket3 -> 139210
