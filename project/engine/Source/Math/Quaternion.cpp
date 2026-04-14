#include "Quaternion.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include <cmath>
#include <numbers>
#include <algorithm>

Quaternion Quaternion::operator*(const Quaternion &other) const {
	return Quaternion(
		w * other.x + x * other.w + y * other.z - z * other.y,
		w * other.y - x * other.z + y * other.w + z * other.x,
		w * other.z + x * other.y - y * other.x + z * other.w,
		w * other.w - x * other.x - y * other.y - z * other.z
	);
}

bool Quaternion::operator==(const Quaternion &other) const {
	return x == other.x && y == other.y && z == other.z && w == other.w;
}

bool Quaternion::operator!=(const Quaternion &other) const {
	return !(*this == other);
}

Quaternion Quaternion::IdentityQuaternion() {
	return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
}

Quaternion Quaternion::Conjugate() const {
	return Quaternion(-x, -y, -z, w);
}

float Quaternion::Norm() const {
	return std::sqrt(x * x + y * y + z * z + w * w);
}

Quaternion Quaternion::Normalize() const {
	float norm = Norm();
	if (norm == 0.0f) {
		return IdentityQuaternion();
	}
	return Quaternion{ x / norm, y / norm, z / norm, w / norm };
}

Quaternion Quaternion::Inverse() const {
	float normSq = x * x + y * y + z * z + w * w;
	if (normSq == 0.0f) {
		return IdentityQuaternion();
	}
	Quaternion conj = Conjugate();
	return Quaternion{ conj.x / normSq, conj.y / normSq, conj.z / normSq, conj.w / normSq };
}

Vector3 Quaternion::RotateVector(const Vector3 &v) {
	Quaternion vecQuat(v.x, v.y, v.z, 0.0f);
	Quaternion resQuat = (*this) * vecQuat * this->Inverse();
	return Vector3(resQuat.x, resQuat.y, resQuat.z);
}

Matrix4x4 Quaternion::MakeRotateMatrix() const {
	Matrix4x4 matrix = MakeIdentity4x4();
	float xx = x * x;
	float yy = y * y;
	float zz = z * z;
	float ww = w * w;
	float xy = x * y;
	float xz = x * z;
	float yz = y * z;
	float wx = w * x;
	float wy = w * y;
	float wz = w * z;
	matrix.m[0][0] = ww + xx - yy - zz;
	matrix.m[0][1] = 2.0f * (xy + wz);
	matrix.m[0][2] = 2.0f * (xz - wy);
	matrix.m[1][0] = 2.0f * (xy - wz);
	matrix.m[1][1] = ww - xx + yy - zz;
	matrix.m[1][2] = 2.0f * (yz + wx);
	matrix.m[2][0] = 2.0f * (xz + wy);
	matrix.m[2][1] = 2.0f * (yz - wx);
	matrix.m[2][2] = ww - xx - yy + zz;
	return matrix;
}

Quaternion Quaternion::MakeRotateAxisAngleQuaternion(const Vector3 &axis, float angle) {
	float halfAngle = angle * 0.5f;
	float sinHalfAngle = std::sin(halfAngle);
	return Quaternion(
		axis.x * sinHalfAngle,
		axis.y * sinHalfAngle,
		axis.z * sinHalfAngle,
		std::cos(halfAngle)
	);
}

Quaternion Quaternion::DirectionToDirection(const Vector3 &from, const Vector3 &to) {
	if (from.lengthSquare() < 1e-6f || to.lengthSquare() < 1e-6f) {
		return IdentityQuaternion();
	}

	Vector3 f = from.normalized();
	Vector3 t = to.normalized();
	float dot = f.dot(t);
	dot = std::clamp(dot, -1.0f, 1.0f);

	// 同方向
	if (dot > 0.999999f) {
		return IdentityQuaternion();
	}

	// 逆方向
	if (dot < -0.999999f) {
		Vector3 axis = Vector3{ 0.0f, 0.0f, 1.0f }.cross(f);
		if (std::abs(f.x) < std::abs(f.y)) {
			if (std::abs(f.x) < std::abs(f.z)) {
				axis = Vector3{ 1.0f, 0.0f, 0.0f }.cross(f);
			}
		} else if (std::abs(f.y) < std::abs(f.z)) {
			axis = Vector3{ 0.0f, 1.0f, 0.0f }.cross(f);
		}
		axis = axis.normalized();
		return MakeRotateAxisAngleQuaternion(axis, std::numbers::pi_v<float>);
	}

	Vector3 axis = f.cross(t).normalized();
	float angle = std::acos(dot);
	return MakeRotateAxisAngleQuaternion(axis, angle);
}

Quaternion Quaternion::LookRotation(const Vector3 &forward, const Vector3 &up) {
	constexpr Vector3 baseForward = { 0.0f, 0.0f, 1.0f };
	Vector3 zAxis = forward.normalized();
	Vector3 xAxis = up.cross(zAxis).normalized();
	Vector3 yAxis = zAxis.cross(xAxis).normalized();
	Quaternion rotToForward = DirectionToDirection(baseForward, zAxis);
	Quaternion rotToUp = DirectionToDirection(rotToForward.RotateVector(up), yAxis);
	return (rotToUp * rotToForward).Normalize();
}

Quaternion Quaternion::Slerp(const Quaternion &q0, const Quaternion &q1, float t) {
	float dot = q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;
	Quaternion q2 = q1;
	if (dot < 0.0f) {
		dot = -dot;
		q2 = Quaternion(-q1.x, -q1.y, -q1.z, -q1.w);
	}

	// 誤差対策
	dot = std::clamp(dot, -1.0f, 1.0f);

	// ほぼ同じ回転 → Lerp
	constexpr float DOT_THRESHOLD = 0.9995f;
	if (dot > DOT_THRESHOLD) {
		return Quaternion(
			q0.x + (q2.x - q0.x) * t,
			q0.y + (q2.y - q0.y) * t,
			q0.z + (q2.z - q0.z) * t,
			q0.w + (q2.w - q0.w) * t
		).Normalize();
	}

	float theta = std::acos(dot);
	float sin = std::sin(theta);
	float s0 = std::sin((1.0f - t) * theta) / sin;
	float s1 = std::sin(t * theta) / sin;
	return Quaternion(
		(s0 * q0.x) + (s1 * q2.x),
		(s0 * q0.y) + (s1 * q2.y),
		(s0 * q0.z) + (s1 * q2.z),
		(s0 * q0.w) + (s1 * q2.w)
	);
}