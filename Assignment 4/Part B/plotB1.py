from matplotlib import pyplot as plt
x_value = ['0 Seconds','10 Seconds','50 Seconds' ,'100 Seconds','500 Seconds']
y_value_100 = [0,1,1,1,1]
y_value_1000 = [0,0,1,1,1]
y_value_20000 = [0,0,0,0,1]
y_value_80000 = [0,0,0,0,0]
fig = plt.figure(figsize = (10, 5))
# plt.bar(x_value,y_value_100,color='orange',width = 0.4)
# plt.bar(x_value,y_value_1000,color='orange',width = 0.4)
# plt.bar(x_value,y_value_20000,color='orange',width = 0.4)
plt.bar(x_value,y_value_80000,color='orange',width = 0.4)
plt.xlabel("Time of Simulation")
plt.ylabel("Convergence Reached")
plt.title("Plot of Convergence Vs Time of Simulation")
plt.show()