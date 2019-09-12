#include <libserial/SerialPort.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <unistd.h>

constexpr const char* const SERIAL_PORT_1 = "/dev/ttyUSB0" ;

int main(void)
{
    using namespace LibSerial ;

    // Instantiate a SerialPort object.
    SerialPort serial_port ;

    try
    {
        // Open the Serial Port at the desired hardware port.
        serial_port.Open(SERIAL_PORT_1) ;
    }
    catch (const OpenFailed&)
    {
        std::cerr << "The serial port did not open correctly." << std::endl ;
        return EXIT_FAILURE ;
    }

    // Set the baud rate of the serial port.
    serial_port.SetBaudRate(BaudRate::BAUD_9600) ;

    // Set the number of data bits.
    serial_port.SetCharacterSize(CharacterSize::CHAR_SIZE_8) ;

    // Turn off hardware flow control.
    serial_port.SetFlowControl(FlowControl::FLOW_CONTROL_NONE) ;

    // Disable parity.
    serial_port.SetParity(Parity::PARITY_NONE) ;

    // Set the number of stop bits.
    serial_port.SetStopBits(StopBits::STOP_BITS_1) ;

    // Create a fild that we'd like to write to
    std::ofstream file;
    file.open("mag_data.csv");

    file << "x,y,z\n";

    // Wait for data to be available at the serial port.
    while(!serial_port.IsDataAvailable())
    {
        usleep(1000) ;
    }

    // Specify a timeout value (in milliseconds).
    size_t ms_timeout = 250 ;

    // Char variable to store data coming from the serial port.
    char data_byte ;

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

    // Read one byte from the serial port and print it to the terminal.
    unsigned bytes_read = 0;
    unsigned rows = 1000;
    unsigned bytes_per_row = 4*3 + 2 + 1; // x , y , z \ n


    while(bytes_read < rows * bytes_per_row)
    {
	    try
	    {
	        // Read a single byte of data from the serial port.
	        serial_port.ReadByte(data_byte, ms_timeout) ;
	        file << data_byte;

	        // Show the user what is being read from the serial port.
	        // std::cout << data_byte << std::flush ;

	        bytes_read++;
	    }
	    catch (const ReadTimeout&)
	    {
	        std::cerr << "\nThe ReadByte() call has timed out." << std::endl ;
	    }
	}

	file.flush();

	file.close();

	std::cerr << "\nRead " << bytes_read << "bytes" << std::endl ;

}

