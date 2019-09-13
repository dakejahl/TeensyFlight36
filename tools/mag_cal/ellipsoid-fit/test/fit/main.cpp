#include <ellipsoid/fit.h>
#include <ellipsoid/generate.h>

#include <time.h>
#include <sstream>

int main(int argc, char const *argv[]) {
	auto type_name = std::string(argv[1]);
	const double tol = 1e-2;

	for (size_t i = 0; i < 1000; ++i) {
		ellipsoid::Parameters parameters;
		parameters.center = 10. * Eigen::Vector3d::Random();
		parameters.radii = 10. * Eigen::Vector3d::Random().cwiseAbs();

		if(type_name == "aligned-xy-equal") {
			parameters.radii.y() = parameters.radii.x();
		}
		else if(type_name == "aligned-xz-equal") {
			parameters.radii.z() = parameters.radii.x();
		}
		else if(type_name == "sphere") {
			parameters.radii.y() = parameters.radii.x();
			parameters.radii.z() = parameters.radii.x();
		}

		auto points = ellipsoid::generate(parameters, 10000);

		auto check_vector3d =
			[tol](const Eigen::Vector3d& identified, const Eigen::Vector3d& expected, const std::string& name) {
				Eigen::Vector3d rel_error = (expected - identified).cwiseQuotient(expected).cwiseAbs();
				if( (rel_error(0) > tol) or
				    (rel_error(1) > tol) or
				    (rel_error(2) > tol))
				{
					std::stringstream ss;
					ss << "Wrong ellipsoid " << name << ": " << identified.transpose() << ", expecting " << expected.transpose();
					throw std::runtime_error(ss.str());
				}
			};

		ellipsoid::Parameters identified_parameters;
		if(type_name == "aligned") {
			identified_parameters = ellipsoid::fit(points, ellipsoid::EllipsoidType::Aligned);
		}
		else if(type_name == "aligned-xy-equal") {
			identified_parameters = ellipsoid::fit(points, ellipsoid::EllipsoidType::AlignedXYEqual);
		}
		else if(type_name == "aligned-xz-equal") {
			identified_parameters = ellipsoid::fit(points, ellipsoid::EllipsoidType::AlignedXZEqual);
		}
		else if(type_name == "sphere") {
			identified_parameters = ellipsoid::fit(points, ellipsoid::EllipsoidType::Sphere);
		}

		check_vector3d(identified_parameters.center, parameters.center, "center");
		check_vector3d(identified_parameters.radii, parameters.radii, "radii");
	}

	return 0;
}
