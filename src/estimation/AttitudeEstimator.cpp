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

#include <AttitudeEstimator.hpp>

using namespace Eigen;

Quaternionf AttitudeEstimator::estimate_quat_1st_step(void)
{
	// Get data from system
	auto accel_data = _accel_sub.get();
	Vector3f accel(accel_data.x, accel_data.y, accel_data.z);

	// Apply calibration
	accel.x() = (accel.x() - ACCEL_OFFSET_X) * ACCEL_SCALE_X;
	accel.y() = (accel.y() - ACCEL_OFFSET_Y) * ACCEL_SCALE_Y;
	accel.z() = (accel.z() - ACCEL_OFFSET_Z) * ACCEL_SCALE_Z;

	// Rotate accel into reference frame (XYZ = NUE) ---> accel::XYZ = -X -Z Y
	auto a = accel;
	accel.x() = -a.x();
	accel.y() = -a.z();
	accel.z() =  a.y();

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
	return Q_ae * _q_estimated;
}

Quaternionf AttitudeEstimator::estimate_quat_2nd_step(const Quaternionf& q_est)
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

float AttitudeEstimator::roll_from_quat(const Quaternionf& q)
{
	float q0 = q.w();
	float q1 = q.x();
	float q2 = q.y();
	float q3 = q.z();

	return std::atan((2*q0*q1 - 2*q2*q3) / (q0*q0 - q1*q1 + q2*q2 - q3*q3));
}

float AttitudeEstimator::pitch_from_quat(const Quaternionf& q)
{
	float q0 = q.w();
	float q1 = q.x();
	float q2 = q.y();
	float q3 = q.z();

	return std::asin(2*q0*q3 + 2*q1*q2);
}

float AttitudeEstimator::yaw_from_quat(const Quaternionf& q)
{
	float q0 = q.w();
	float q1 = q.x();
	float q2 = q.y();
	float q3 = q.z();

	return std::atan((2*q0*q2-2*q1*q3) / (q0*q0 + q1*q1 - q2*q2 - q3*q3));
}




Quaternionf AttitudeEstimator::quat_error(Quaternionf q1, Quaternionf q2)
{
	Vector4f q_error, auxq1;
	Matrix4f K;
	Quaternionf err;

	auxq1 << q1.vec(), q1.w();

	K << q2.w() * MatrixXf::Identity(3, 3) - crossp_mat(q2.vec()), -q2.vec(), q2.vec().transpose(), q2.w();

	q_error = K * auxq1;

	err.w() = q_error(3);
	err.vec()(0) = q_error(0);
	err.vec()(1) = q_error(1);
	err.vec()(2) = q_error(2);

	return err;
}

Matrix3f AttitudeEstimator::crossp_mat(Vector3f v)
{
	Matrix3f aux;

	aux << 	0, -v(2), v(1),
			v(2), 0, -v(0),
			-v(1), v(0), 0;

	return aux;
}



Quaternionf AttitudeEstimator::vect2q(Vector4f v)
{
	Quaternionf q;
	Vector3f v_3;

	q.w() = v(0);
	v_3(0) = v(1);
	v_3(1) = v(2);
	v_3(2) = v(3);

	q.vec() = v_3;

	return q;
}

Vector4f AttitudeEstimator::q2vect(Quaternionf q)
{
	Vector4f quat;
	Vector3f q_v;

	quat(0) = q.w();
	q_v = q.vec();

	quat(1) = q_v(0);
	quat(2) = q_v(1);
	quat(3) = q_v(2);

	return quat;
}

Quaternionf AttitudeEstimator::qexp(Quaternionf q)
{
	Quaternionf exp_q;
	Vector3f vect;
	Vector3f exp_v;

	float sigma, exp_sigma, w, sinw;

	sigma = q.w();
	vect = q.vec();

	w = vect.norm();
	sinw = std::sin(w);
	exp_sigma = std::exp(sigma) * std::cos(w);

	if (w != 0)
	{
		exp_v(0) = std::exp(sigma) * (vect(0) / w) * sinw;
		exp_v(1) = std::exp(sigma) * (vect(1) / w) * sinw;
		exp_v(2) = std::exp(sigma) * (vect(2) / w) * sinw;
	}
	else
	{
		exp_v(0) = 0;
		exp_v(1) = 0;
		exp_v(2) = 0;
	}

	exp_q.w() = exp_sigma;
	exp_q.vec() = exp_v;

	return exp_q;
}
