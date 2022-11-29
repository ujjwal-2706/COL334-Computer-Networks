from matplotlib import pyplot as plt
x_value = ['49 Seconds','51 Seconds','60 Seconds' ,'119 Seconds','121 Seconds', '150 Seconds']
reachablity = [1,1,1,1,0,0]
plt.bar(x_value,reachablity,color='orange',width = 0.4)
plt.xlabel("Time of Simulation")
plt.ylabel("Destination Reachable")
plt.title("Plot of Destination Reachable From Source Vs Time of Simulation")
plt.show()