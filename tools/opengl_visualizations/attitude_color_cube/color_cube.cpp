// This program is a flyby around the RGB color cube.  One intersting note
// is that because the cube is a convex polyhedron and it is the only thing
// in the scene, we can render it using backface culling only. i.e., there
// is no need for a depth buffer.


#include <GL/glut.h>
#include <cmath>

#include <libserial/SerialPort.h>

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <string>

// Serial port
using namespace LibSerial;
SerialPort serial_port;
constexpr const char* const SERIAL_PORT_1 = "/dev/ttyUSB0";
std::vector<float> rpy_data;

unsigned elem_index = 0;
std::string float_as_string;
volatile bool rpy_data_ready = true;

int setup_serial(void);
void wait_for_serial_data(void);
void serial_parse_data(void);

const int NUM_VERTICES = 8;
const int NUM_FACES = 6;

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0, 0.0, -7.0);

	// I can't get this camera
	// eye (x, y ,z) -- center (x, y, z) -- up (x, y, z)
	// gluLookAt(-1, 1, 0, 0, 0, 0, 1, 0, 0);

	float roll = rpy_data.at(0);
	float pitch = rpy_data.at(1);

	glRotatef(-roll, 0.00, 0.00, 1.00);
	glRotatef(pitch, 1.00, 0.00, 0.00);

	glBegin(GL_QUADS);
	glColor3f(0.0, 0.0, 0.0);
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(1.0, 0.2, -1.0);
	glVertex3f(-1.0, 0.2, -1.0);
	glVertex3f(-1.0, 0.2, 1.0);
	glVertex3f(1.0, 0.2, 1.0);

	glColor3f(0.0, 0.0, 1.0);
	glColor3f(1.0, 0.5, 0.0);
	glVertex3f(1.0, -0.2, 1.0);
	glVertex3f(-1.0, -0.2, 1.0);
	glVertex3f(-1.0, -0.2, -1.0);
	glVertex3f(1.0, -0.2, -1.0);

	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(1.0, 0.2, 1.0);
	glVertex3f(-1.0, 0.2, 1.0);
	glVertex3f(-1.0, -0.2, 1.0);
	glVertex3f(1.0, -0.2, 1.0);

	glColor3f(1.0, 1.0, 0.0);
	glVertex3f(1.0, -0.2, -1.0);
	glVertex3f(-1.0, -0.2, -1.0);
	glVertex3f(-1.0, 0.2, -1.0);
	glVertex3f(1.0, 0.2, -1.0);

	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(-1.0, 0.2, 1.0);
	glVertex3f(-1.0, 0.2, -1.0);
	glVertex3f(-1.0, -0.2, -1.0);
	glVertex3f(-1.0, -0.2, 1.0);

	glColor3f(1.0, 0.0, 1.0);
	glVertex3f(1.0, 0.2, -1.0);
	glVertex3f(1.0, 0.2, 1.0);
	glVertex3f(1.0, -0.2, 1.0);
	glVertex3f(1.0, -0.2, -1.0);

	glEnd();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	draw();
	glFlush();
	glutSwapBuffers();
}

void timer(int v)
{
	// Read in data from serial port
	serial_parse_data();

	// Redraw the cube if we have a new orientation
	if (rpy_data_ready)
	{
		// Calls the display function
		glutPostRedisplay();

		glPopMatrix(); // Pop the old matrix without the transformations.

		// clear flag
		rpy_data_ready = false;
	}

	// reschedule timer
	glutTimerFunc(1, timer, v);
}

void reshape(int w, int h)
{
	if (h == 0)
		h = 1;

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1.0*w/h, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void init()
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

int main(int argc, char** argv)
{
	rpy_data.reserve(3);
	rpy_data.push_back(0);
	rpy_data.push_back(0);
	rpy_data.push_back(0);

	// enable our serial port
	if (setup_serial() != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	wait_for_serial_data();

	// Start open GL stuff
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutCreateWindow("The RGB Color Cube");
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer, 0);
	glutDisplayFunc(display);
	init();

	glutMainLoop();
}


int setup_serial()
{
	// Instantiate a SerialPort object.

	try
	{
		// Open the Serial Port at the desired hardware port.
		serial_port.Open(SERIAL_PORT_1);
	}
	catch (const OpenFailed&)
	{
		std::cerr << "The serial port did not open correctly." << std::endl;
		return EXIT_FAILURE;
	}

	// Set the baud rate of the serial port.
	serial_port.SetBaudRate(BaudRate::BAUD_9600);

	// Set the number of data bits.
	serial_port.SetCharacterSize(CharacterSize::CHAR_SIZE_8);

	// Turn off hardware flow control.
	serial_port.SetFlowControl(FlowControl::FLOW_CONTROL_NONE);

	// Disable parity.
	serial_port.SetParity(Parity::PARITY_NONE);

	// Set the number of stop bits.
	serial_port.SetStopBits(StopBits::STOP_BITS_1);

	std::cerr << "Serial port configured" << std::endl;

	return EXIT_SUCCESS;
}

void wait_for_serial_data(void)
{
	size_t ms_timeout = 250;
	char data_byte  = 0;

	// Wait for data to be available at the serial port.
	std::cerr << "\nWaiting for data...." << std::endl;
	while(!serial_port.IsDataAvailable())
	{
		usleep(1000);
	}
	// Get to the next newline
	while(data_byte != '\n')
	{
		try
		{
			// Read a single byte of data from the serial port.
			serial_port.ReadByte(data_byte, ms_timeout) ;
		}
		catch (const ReadTimeout&)
		{
			std::cerr << "\nThe ReadByte() call has timed out." << std::endl ;
		}
	}

	std::cerr << "\nStarting!" << std::endl;
}

void serial_parse_data(void)
{
	size_t ms_timeout = 250;
	char data_byte  = 0;
	try
	{
		serial_port.ReadByte(data_byte, ms_timeout) ;

		// We are starting a new row
		if (data_byte == '\n')
		{
			rpy_data[elem_index] = std::stof(float_as_string);

			float_as_string.clear();
			elem_index = 0;
			rpy_data_ready = true;
		}
		// We are moving to the next element in the array
		else if (data_byte == ',')
		{
			rpy_data[elem_index] = std::stof(float_as_string);

			float_as_string.clear();
			elem_index++;

		}
		// We are still parsing the current element
		else
		{
			float_as_string += data_byte;
		}
	}
	catch (const ReadTimeout&)
	{
		std::cerr << "\nThe ReadByte() call has timed out." << std::endl ;
	}
}
