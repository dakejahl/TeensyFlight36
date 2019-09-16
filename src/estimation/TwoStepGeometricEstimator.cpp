// MIT License

// Copyright (c) 2019 Jacob Dahl

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <TwoStepGeometricEstimator.hpp>

using namespace Eigen;



// ----- ATTITUDE QUATERNION FANCY PANTS ESTIMATOR -----//
void TwoStepGeometricEstimator::estimate_attitude(void)
{
	auto q = estimate_quat_1st_step();

	// SYS_INFO("q.w(): %f", q.w());
	// SYS_INFO("q.x(): %f", q.x());
	// SYS_INFO("q.y(): %f", q.y());
	// SYS_INFO("q.z(): %f", q.z());

	_roll_est = roll_from_quat(q);
	_pitch_est = pitch_from_quat(q);
}

Quaternionf TwoStepGeometricEstimator::estimate_quat_1st_step(void)
{
	// Get data from system
	Vector3f accel = _accel_xyz;

	// Rotate accel into reference frame (XYZ = NUE) ---> accel::XYZ = -X -Z Y
	auto a = accel;
	accel.x() =  a.x();
	accel.y() =  a.z();
	accel.z() = -a.y();

	// ----- Eq(18) ----- //

	// Firstly, estimate vector of gravity using previous measurement of Q.
	Vector3f g_vect_ref = Vector3f(0,1,0); // Gravity field points down (NUE)
	Vector3f g_vect_est = direction_cosine_matrix(_q_estimated) * g_vect_ref;

	// Second, calculate measured vector of earths gravity
	Vector3f g_vect_meas = accel / accel.norm();

	// ----- Eq(20) && Eq(21) ----- //

	// Correct the estimated direction of gravity using accel readings
	static constexpr float SCALAR_THING = 1;

	// Cross product of measured and estimated vector fields
	Vector3f N_a = g_vect_meas.cross(g_vect_est);
	// Rotation theta around axis N_a which is defined by the cross product of Vg_meas and Vg_est
	float theta_err = std::acos(g_vect_meas.dot(g_vect_est));

	float real = std::cos(SCALAR_THING * theta_err / 2);
	Vector3f imaginary = N_a * std::sin(SCALAR_THING * theta_err / 2);

	// Error quaternion
	Quaternionf Q_ae;
	Q_ae.w() = real;
	Q_ae.x() = imaginary.x();
	Q_ae.y() = imaginary.y();
	Q_ae.z() = imaginary.z();

	// Calculate estimated quaternion
	auto q_est =  Q_ae * _q_estimated;

	return q_est;
}

Quaternionf TwoStepGeometricEstimator::estimate_quat_2nd_step(const Quaternionf& q_est)
{
	auto mag_data = _mag_sub.get();
	Vector3f mag(mag_data.x, mag_data.y, mag_data.z);

	mag.x() = (mag.x() - MAG_OFFSET_X) / MAG_SCALE_X;
	mag.y() = (mag.y() - MAG_OFFSET_Y) / MAG_SCALE_Y;
	mag.z() = (mag.z() - MAG_OFFSET_Z) / MAG_SCALE_Z;
	// Rotate mag into reference frame -- XYZ = NUE
	// accel XYZ = -X Z -Y     -- TODO: verify this.. I used the px4 driver as a reference
	mag.x() = -mag.x();
	mag.y() =  mag.z();
	mag.z() = -mag.y();

	// Mag estimation and correction
	Vector3f m_vect_ref(0,0,0); // TODO: figure out what this is
	Vector3f m_vect_meas = mag / mag.norm();
	Vector3f m_vect_est = direction_cosine_matrix(_q_estimated) * m_vect_ref; // Gravity field is only in one direction on earth (NUE)

	// ----- Eq(22) ----- //
	Vector3f V_mxz = {mag.x(), mag.y(), mag.z()};
	V_mxz = q_est.conjugate() * (q_est * V_mxz);

	// ----- Eq(23) ----- //
	float bx = V_mxz.x();
	float bz = V_mxz.z();
	V_mxz = {std::sqrt(bx*bx + bz*bz), 0, 0};

	// ----- Eq(24) ----- //
	Vector3f V_north = {1, 0, 0}; // NUE
	Vector3f N_m = V_mxz.cross(V_north);
	float theta_err = std::acos(V_mxz.dot(V_north));

	float real = std::cos(theta_err / 2);
	Vector3f imaginary = N_m * std::sin(theta_err / 2);

	Quaternionf Q_me;
	Q_me.w() = real;
	Q_me.x() = imaginary.x();
	Q_me.y() = imaginary.y();
	Q_me.z() = imaginary.z();

	return Q_me * q_est;
}

Matrix3f TwoStepGeometricEstimator::direction_cosine_matrix(const Quaternionf& q)
{
	Matrix3f C_bn;

	float q0 = q.w();
	float q1 = q.x();
	float q2 = q.y();
	float q3 = q.z();

	C_bn(0,0) = q0*q0 + q1*q1 - q2*q2 - q3*q3;
	C_bn(0,1) = 2*q0*q3 + 2*q1*q2;
	C_bn(0,2) = -2*q0*q2 + 2*q1*q3;

	C_bn(1,0) = -2*q0*q3 + 2*q1*q2;
	C_bn(1,1) = q0*q0 - q1*q1 + q2*q2 - q3*q3;
	C_bn(1,2) = 2*q0*q1 + 2*q2*q3;

	C_bn(2,0) = 2*q0*q2 + 2*q1*q3;
	C_bn(2,1) = -2*q0*q1 + 2*q2*q3;
	C_bn(2,2) = q0*q0 - q1*q1 - q2*q2 + q3*q3;

	return C_bn;
}

