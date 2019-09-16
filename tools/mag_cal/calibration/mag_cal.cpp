#include <libserial/SerialPort.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>

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

    // Wait for data to be available at the serial port.
    std::cerr << "\nWaiting for data...." << std::endl ;
    while(!serial_port.IsDataAvailable())
    {
        usleep(1000) ;
    }

    // Specify a timeout value (in milliseconds).
    size_t ms_timeout = 250 ;

    // Char variable to store data coming from the serial port.
    char data_byte  = 0;

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

    static constexpr unsigned rows = 1000;

    // Read 1000 mag data points
    Eigen::Matrix<double, rows, 3> points;
    Eigen::Vector3d point;

    std::string float_as_string;
    unsigned row_index = 0;
    unsigned elem_index = 0;

    std::cerr << "\nStarting mag calibration." << std::endl ;


    while(row_index < rows - 1)
    {
	    try
	    {
	        // Read a single byte of data from the serial port.
	        serial_port.ReadByte(data_byte, ms_timeout) ;

            // We are starting a new row
            if (data_byte == '\n')
            {
                point[elem_index] = std::stof(float_as_string);
                points.row(row_index) = point;

                float_as_string.clear();
                row_index++;
                elem_index = 0;
                continue;
            }
            // We are moving to the next element in the array
            else if (data_byte == ',')
            {
                point[elem_index] = std::stof(float_as_string);

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

    std::cerr << "\nRead " << row_index + 1<< "rows" << std::endl ;

    // Run the ellipsoid fit algorithm
    auto params = ellipsoid::fit(points, ellipsoid::EllipsoidType::Aligned);

    std::cout << "Identified parameters:\n";
    params.print();

    // Create a file that we'd like to write to
    std::ofstream file;
    const char* path = "/home/jake/code/jake/TeensyFlight36/tools/raw_data/mag_data.csv";
    file.open(path);

    file << "x,y,z\n";

    // write calibrated data to file
    for (unsigned i = 0; i < rows - 1; i++)
    {
        auto p = points.row(i);

        // We use the center point per axis as the offset, and the radii as the scale factor
        float x = (p.x() - params.center.x()) / params.radii.x();
        float y = (p.y() - params.center.y()) / params.radii.y();
        float z = (p.z() - params.center.z()) / params.radii.z();

        file << x << ',' << y<< ',' << z << '\n';
    }

    file.flush();

    file.close();
}

