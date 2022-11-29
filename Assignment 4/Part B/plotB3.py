from matplotlib import pyplot as plt
x_value = ['119 Seconds','121 Seconds','140 Seconds' ,'200 Seconds']
reachablity = [0,0,1,1]
plt.bar(x_value,reachablity,color='orange',width = 0.4)
plt.xlabel("Time of Simulation")
plt.ylabel("Destination Reachable")
plt.title("Plot of Destination Reachable From Source Vs Time of Simulation")
plt.show()