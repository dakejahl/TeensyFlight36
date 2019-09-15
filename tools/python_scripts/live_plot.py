#importing libraries
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.animation as animation

mpl.use("TkAgg")

fig = plt.figure()
#creating a subplot
ax1 = fig.add_subplot(1,1,1)

def animate(i):
    path = "/home/jake/code/jake/TeensyFlight36/tools/raw_data/3_axis_data.csv"
    data = open(path,'r').read()
    lines = data.split('\n')
    xs = []
    ys = []

    i = 0
    index = 1
    for line in lines[1:]:
        comma_exists = 0
        i = i + 1
        for item in line:
            if item == ',':
                comma_exists = 1

        if comma_exists:
            x, y, z = line.split(',') # Delimiter is comma
            index = index + 1

        xs.append(float(index))
        ys.append(float(y))


    ax1.clear()
    ax1.plot(xs, ys)

    plt.xlabel('index')
    plt.ylabel('data')
    plt.title('Live graph with matplotlib')


ani = animation.FuncAnimation(fig, animate, interval=100)
plt.show()
