#include <ellipsoid/generate.h>

namespace ellipsoid {

Eigen::Matrix<double, Eigen::Dynamic, 3> generate(const Parameters& parameters, size_t samples) {
	Eigen::Matrix<double, Eigen::Dynamic, 3> points;
	points.resize(samples, 3);

	for (size_t i = 0; i < samples; ++i) {
		Eigen::Vector3d point;

		// Generate two pseudo-random angular coordinates
		double theta = ((std::rand() - RAND_MAX/2) / double(RAND_MAX)) * M_PI;    // -pi/2 < theta < pi/2
		double phi = ((std::rand() - RAND_MAX/2) / double(RAND_MAX)) * 2. * M_PI; // -pi < phi < pi

		point.x() = parameters.center.x() + parameters.radii.x() * std::cos(theta) * std::cos(phi);
		point.y() = parameters.center.y() + parameters.radii.y() * std::cos(theta) * std::sin(phi);
		point.z() = parameters.center.z() + parameters.radii.z() * std::sin(theta);

		points.row(i) = point.transpose();
	}

	return points;
}

}
