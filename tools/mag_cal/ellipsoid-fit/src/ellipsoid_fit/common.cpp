#include <ellipsoid/common.h>
#include <iostream>

namespace ellipsoid {

void Parameters::print() {
	std::cout << "\tcenter: "  << center.transpose() << std::endl;
	std::cout << "\tradii: "  << radii.transpose() << std::endl;
}

}
