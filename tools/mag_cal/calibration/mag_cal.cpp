#include <libserial/SerialPort.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <unistd.h>

#include <ellipsoid_fit/ellipsoid/fit.h>
#include <ellipsoid_fit/ellipsoid/generate.h>

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

    unsigned bytes_read = 0;
    static constexpr unsigned rows = 10;
    static constexpr unsigned bytes_per_row = 4*3 + 2 + 1; // x , y , z \ n --- THERES 10 BYTES IN A ROW!

    // Read 1000 mag data points
    ellipsoid::Parameters parameters;
    parameters.center = {0,0,0};
    parameters.radii = {1,1,1};
    Eigen::Matrix<double, rows, 3> points;
    Eigen::Vector3d point;

    unsigned row_count = 0;
    unsigned index = 0;
    while(bytes_read < rows * bytes_per_row)
    {
	    try
	    {
	        // Read a single byte of data from the serial port.
	        serial_port.ReadByte(data_byte, ms_timeout) ;
            bytes_read++;

            // write data to file
	        file << data_byte;

            // store coordinate
            if (data_byte != ',' && data_byte != '\n' && data_byte != '.')
            {
                // point(index) = data_byte;
                // std::cerr << "\nbyte is " << data_byte << std::endl ;
                printf("byte: %c\n", data_byte);
                // std::cerr << "\nindex is " << index << std::endl ;

                index++;
            }
            else if (data_byte == '\n')
            {
                printf("NEW LINE\n");
            }
            else if (data_byte == ',')
            {
                printf(",,,,,,,,,,,,,,,,,,,,\n");
            }
            else if (data_byte == '.')
            {
                printf("DOT\n");
            }

            // push vector into matrix if we've read xyz
            if (data_byte == '\n')
            {
                index = 0;
                // points.row(row_count) = point;
                // point = {0,0,0};
                row_count++;
            }


	        // Show the user what is being read from the serial port.
	        // std::cout << data_byte << std::flush ;

	    }
	    catch (const ReadTimeout&)
	    {
	        std::cerr << "\nThe ReadByte() call has timed out." << std::endl ;
	    }
	}

	file.flush();

	file.close();

	std::cerr << "\nRead " << bytes_read << "bytes" << std::endl ;
    std::cerr << "\nRead " << row_count << "rows" << std::endl ;


    // Run the ellipsoid fit algorithm now

    auto identified_parameters = ellipsoid::fit(points, ellipsoid::EllipsoidType::Aligned);

    std::cout << "Parameters used for generation:\n";
    parameters.print();
    std::cout << "Identified parameters:\n";
    identified_parameters.print();

}

