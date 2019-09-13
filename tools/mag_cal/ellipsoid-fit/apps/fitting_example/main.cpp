#include <ellipsoid/fit.h>
#include <ellipsoid/generate.h>

#include <iostream>
#include <time.h>

int main(int argc, char const *argv[]) {

	std::srand(time(nullptr));

	ellipsoid::Parameters parameters;
	parameters.center = Eigen::Vector3d::Random();
	parameters.radii = Eigen::Vector3d::Random().cwiseAbs();

	auto points = ellipsoid::generate(parameters, 10000);

	auto identified_parameters = ellipsoid::fit(points, ellipsoid::EllipsoidType::Aligned);

	std::cout << "Parameters used for generation:\n";
	parameters.print();
	std::cout << "Identified parameters:\n";
	identified_parameters.print();

	return 0;
}
