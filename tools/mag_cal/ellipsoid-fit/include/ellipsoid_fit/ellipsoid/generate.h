#pragma once

#include "common.h"
#include <eigen3/Eigen/Dense>

namespace ellipsoid {

/**
 * Generate 3D cartesian points laying on an ellipsoid.
 * @param parameters ellipsoid's parameters (center and radii)
 * @param samples    number of points to generate
 * @return           3xN Matrix containing the generated points
 */
Eigen::Matrix<double, Eigen::Dynamic, 3> generate(const Parameters& parameters, size_t samples = 1000);

}
