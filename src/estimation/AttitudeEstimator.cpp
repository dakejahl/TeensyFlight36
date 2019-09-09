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

Quaternionf AttitudeEstimator::calculate_quaternions(void)
{
	static Vector3f b1(0,0,1); // global frame expected accel

	static Vector3f b2(0.4, 0, 0.916); // global frame expected mag

	static float a1 = 1; // weight for accel
	static float a2 = 1; // weight for mag
	static float k = 4; // non-linear observer gain

	Vector3f gyro_data = {}; // get data
	Vector3f accel_data = {}; // get data
	Vector3f mag_data = {}; // get data

	auto now = time::HighPrecisionTimer::Instance()->get_absolute_time_us();
	auto Ts = now - _last_time; // units of microseconds
	_last_time = now;

	auto r1 = accel_data;
	auto r2 = mag_data;
	auto w_gyr = gyro_data;

	auto q_meas = Dav_qmethod(r1, r2, b1, b2, a1, a2);

	// non-linear observer
	_q_obs = _q_obs * qexp(vect2q(Ts / 2 * q2vect(_unknown_variable)));
	_bias_obs = _bias_obs - Ts / 2 * _epsilon_err * sgn(_eta_err);

	// angular rate from gyro minus the bias from the oberserver
	auto w = w_gyr - _bias_obs;

	// Calculate error between measured and observer
	auto q_err = quat_error(q_meas, _q_obs);
	_eta_err = q_err.w();

	// save the error vector
	_epsilon_err = q_err.vec();

	Quaternionf w_quat;
	w_quat.w() = 0;
	w_quat.vec() = w + k * _epsilon_err * sgn(_eta_err);
	_unknown_variable = q_err *  w_quat * q_err.inverse();

	Quaternionf q_att;

	if (_q_obs.w() < 0)
	{
		q_att.w() = -_q_obs.w();
		q_att.vec() = -_q_obs.vec();
	}
	else
	{
		q_att = _q_obs;
	}

	return q_att;

	// q_att = quaternion attitude

}

Quaternionf AttitudeEstimator::Dav_qmethod(Vector3f r1, Vector3f r2, Vector3f b1, Vector3f b2, float a1, float a2)
{
	Vector3f Z;

	Vector4f eigenvalues;
	Vector4f eig_vector;

	Matrix3f B;
	Matrix3f S;

	Matrix4f K;
	Matrix4f eigenvectors;

	float eig_max;

	// Begin the math
	B = a1 * b1 * r1.transpose() + a2 * b2 * r2.transpose();
	S = B + B.transpose();

	// Push elements into vector3
	Z << B(1, 2) - B(2, 1), B(2, 0) - B(0, 2), B(0, 1) - B(1, 0);

	// Push rows into matrix4
	K << (S - (B.trace() * MatrixXf::Identity(3, 3))), Z, Z.transpose(), B.trace();

	// calculate eigen values and vectors
	SelfAdjointEigenSolver<Matrix4f> eigensolver(K);

	if (eigensolver.info() != Success)
	{
		SYS_INFO("eigen value error...?");
	}

	eigenvalues = eigensolver.eigenvalues();
	eigenvectors = eigensolver.eigenvectors();

	eig_max = eigenvalues.maxCoeff();

	for (unsigned j = 0; j < 4; j++)
	{
		if (eigenvalues(j) == eig_max)
		{
			eig_vector(0) = eigenvectors(3, j);
			eig_vector(1) = -eigenvectors(0, j);
			eig_vector(2) = -eigenvectors(1, j);
			eig_vector(3) = -eigenvectors(2, j);
		}
	}

	if (eig_vector(0) < 0)
	{
		eig_vector(0) = -eig_vector(0);
		eig_vector(1) = -eig_vector(1);
		eig_vector(2) = -eig_vector(2);
		eig_vector(3) = -eig_vector(3);

	}

	return vect2q(eig_vector);
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
