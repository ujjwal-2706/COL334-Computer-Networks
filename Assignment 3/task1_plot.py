from matplotlib import pyplot as plt
import numpy as np
data_congestion_window = np.loadtxt("task1.txt",delimiter=' ')
max_window_size =  np.max(data_congestion_window[:,1])
print(f"The maximum congestion window size is : {max_window_size}")
plt.plot(data_congestion_window[:,0],data_congestion_window[:,1])
plt.xlabel("Time of Simulation")
plt.ylabel("Congestion Window Size")
plt.title("TCP Congestion Window Vs Time of Simulation")
plt.show()


#vegas 11252 size and 58 drops of packet
#new reno 58 drops of packet and 15462 size 
#veno 59 drops of packet and 15462 size
#west wood 60 drops of packet and 15471 size