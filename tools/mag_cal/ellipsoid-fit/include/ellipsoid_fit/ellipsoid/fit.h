#pragma once

#include "common.h"
#include <eigen3/Eigen/Dense>

namespace ellipsoid {

enum class EllipsoidType {
	Arbitrary,
	XYEqual,
	XZEqual,
	Sphere,
	Aligned,
	AlignedXYEqual,
	AlignedXZEqual,
};

/**
 * Fit an ellipsoid on the given data
 * @param  data 3xN matrix with the cartesian coordinates to fit the ellipsoid on
 * @return      ellipsoid's parameters
 */
Parameters fit(const Eigen::Matrix<double, Eigen::Dynamic, 3>& data, EllipsoidType type = EllipsoidType::Arbitrary);

}
