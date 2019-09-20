#importing libraries
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.animation as animation

mpl.use("TkAgg")

fig = plt.figure()
#creating a subplot
# ax1 = fig.add_subplot(3,1,1)
# ay1 = fig.add_subplot(3,1,1)
# az1 = fig.add_subplot(3,1,3)

def animate(i):
    path = "/home/jake/code/jake/TeensyFlight36/tools/raw_data/3_axis_data.csv"
    data = open(path,'r').read()
    lines = data.split('\n')
    sample_index = []
    x_data = []
    y_data = []
    z_data = []

    index = 1
    for line in lines[1:]:
        comma_count = 0

        for item in line:
            if item == ',':
                comma_count = comma_count + 1

        if comma_count == 2:
            x, y, z = line.split(',') # Delimiter is comma
            index = index + 1

            sample_index.append(float(index))
            x_data.append(float(x))
            y_data.append(float(y))
            z_data.append(float(z))


    plt.title('Live graph with matplotlib')

    plt.plot(sample_index, x_data, 'r')

    # ay1.clear()
    plt.plot(sample_index, y_data, 'b')

    # az1.clear()
    # az1.plot(sample_index, z_data)


ani = animation.FuncAnimation(fig, animate, interval=100)
plt.show()
