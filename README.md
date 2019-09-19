<!-- ![](images/teensyflight.png =50x50) -->
<img src="images/teensyflight.png " width="80%">

## What's the point?
Challenging myself to learn, improve, and produce practical high quality embedded systems and software.

## Features
- Makefile + SWD GDB upload and debug
- FreeRTOS
- BSP is essentially `cores/teensy3`
- C++ Dispatch queue for asynchronous and interval scheduling (100us tick interrupt)
- Publish / Subscribe communication framework (very light weight)
- MPU9250 @ 1kHz gyro / accel / mag on 10MHz SPI
- FrSky XM+ mini on UART SBUS
- PWM driver at 400Hz for actuator control signals
- Attitude estimation. Complimentary filter, Kalman filter, EKF
- Host side serial communications with onboard "shell" task
- Interactive plotting and visualizations with python and OpenGL

## Building with make
I started with a framework I had been hearing about called `platformio`. I figured it sounded cool and I'd try it out. It became severely limiting as soon as I wanted to do anything more than what was supported from it natively, so I switched to a makefile. Using a hodgepodge of references from the interwebs, I created a makefile based build system that can be easily invoked via the command line or from an IDE.

## SWD Debugging: JLink + GDB
Now this is a hardware hack. I soldered the reset line on the companion microcontroller (KL02) to ground, this holds the KL02 in reset and thus prevents its activity on the SWD lines.<newline/>
https://mcuoneclipse.com/2017/04/29/modifying-the-teensy-3-5-and-3-6-for-arm-swd-debugging/

### Thread aware debug with MCUXpresso
I nice tool built into MCUXpresso, only requiring a somewhat poorly explained process for configuring it. I was able to piece it together from scouring the internet, and it is really useful! You can view your stack sizes, heap usage, and % cpu at any breakpoint.
![](images/freertosTAD.png)

### Segger SystemView
An amazing tool if you really want to take a detailed look "inside" of your machine. You can instrument any section of code you'd like using the API, however I chose to only verify ISR / Task interaction and timing characteristics.
![](images/systemview.png)<br/>
https://www.segger.com/products/development-tools/systemview/

### Interactive data visualization
I created a tool in C++ that reads 3-axis data from the serial port and optionally writes it to a file. I then run an ellipsoid fit algorithm on the data set and plot the data in 3D with the caclulated offsets and scales for each axis.
https://plot.ly/python/3d-scatter-plots/
![](images/mag_data.gif)

### Live plotting
I use the 3-axis data stream and plot the results in real time. I can choose the data stream I am interested in with a simple shell command via the USB port.<br/>
_i.e_ `stream accel_data`
![](images/live_plot.gif)

### Attitude visualization
I created a tool using OpenGL to visualize attitude data coming from the flight controller in real time.
![](images/attitude.gif)

<br/>

***Thank you to...***

The PX4 team, you guys rock https://github.com/PX4/Firmware<br/>
Phillip Johnston https://embeddedartistry.com/<br/>
Erich Styger https://mcuoneclipse.com/<br/>
Paul Stoffregen https://www.pjrc.com/store/teensy36.html<br/>
<br/>

_"If I have seen further it is by standing on the shoulders of Giants."_ -- Isaac Newton

