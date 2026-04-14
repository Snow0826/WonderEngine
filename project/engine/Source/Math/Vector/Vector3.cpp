#include "Vector3.h"
#include "Easing.h"
#include <algorithm>
#include <numbers>

Vector3 Vector3::normalized() const {
	float len = length();
	if (len == 0.0f) {
		return Vector3{};
	}
	return *this / len;
}

Vector3 Vector3::normalized(const Vector3 &other) const {
	float distance = distanceFrom(other);
	if (distance == 0.0f) {
		return Vector3{};
	}
	return (other - *this) / distance;
}

Vector3 Vector3::perpendicular() const {
	if (x != 0.0f || y != 0.0f) {
		return { -y, x, 0.0f };
	}
	return { 0.0f, -z, y };
}

void Vector3::operator+=(const Vector3 &other) {
	this->x += other.x;
	this->y += other.y;
	this->z += other.z;
}

void Vector3::operator-=(const Vector3 &other) {
	this->x -= other.x;
	this->y -= other.y;
	this->z -= other.z;
}

void Vector3::operator*=(const float &s) {
	x *= s;
	y *= s;
	z *= s;
}

void Vector3::operator/=(const float &s) {
	x /= s;
	y /= s;
	z /= s;
}

Vector3 ToCartesian(const Vector3 &spherical) {
	return {
		spherical.x * std::sin(spherical.z) * std::sin(spherical.y),
		spherical.x * std::cos(spherical.z),
		spherical.x * std::sin(spherical.z) * std::cos(spherical.y),
	};
}

Vector3 ToSpherical(const Vector3 &cartesian) {
	Vector3 spherical;
	spherical.x = cartesian.length();
	if (spherical.x == 0.0f) {
		spherical.y = 0.0f;
		spherical.z = 0.0f;
	} else {
		spherical.y = std::atan2(cartesian.x, cartesian.z);
		spherical.z = std::acos(cartesian.y / spherical.x);
	}
	return spherical;
}

Vector3 LerpShortAngle(const Vector3 &v1, const Vector3 &v2, float t) {
	Vector3 result{};

	// 差分を求めて-π~πの範囲に収める
	Vector3 diff{};
	diff.x = std::fmodf(v2.x - v1.x, 2.0f * std::numbers::pi_v<float>);
	diff.y = std::fmodf(v2.y - v1.y, 2.0f * std::numbers::pi_v<float>);
	diff.z = std::fmodf(v2.z - v1.z, 2.0f * std::numbers::pi_v<float>);
	if (diff.x < -std::numbers::pi_v<float>) diff.x += 2.0f * std::numbers::pi_v<float>;
	if (diff.y < -std::numbers::pi_v<float>) diff.y += 2.0f * std::numbers::pi_v<float>;
	if (diff.z < -std::numbers::pi_v<float>) diff.z += 2.0f * std::numbers::pi_v<float>;
	if (diff.x > std::numbers::pi_v<float>) diff.x -= 2.0f * std::numbers::pi_v<float>;
	if (diff.y > std::numbers::pi_v<float>) diff.y -= 2.0f * std::numbers::pi_v<float>;
	if (diff.z > std::numbers::pi_v<float>) diff.z -= 2.0f * std::numbers::pi_v<float>;

	result = v1 + diff * t;
	return result;
}

Vector3 Slerp(const Vector3 &start, const Vector3 &end, float t) {
	Vector3 normalizedStart = start.normalized();
	Vector3 normalizedEnd = end.normalized();
	float dot = normalizedStart.dot(normalizedEnd);
	dot = std::clamp(dot, -1.0f, 1.0f);
	float theta = std::acos(dot);
	float sinTheta = std::sin(theta);
	float sinThetaFrom = std::sin((1.0f - t) * theta);
	float sinThetaTo = std::sin(t * theta);
	Vector3 slerp;
	if (sinTheta < 1.0e-5) {
		slerp = normalizedStart;
	} else {
		slerp = (sinThetaFrom * normalizedStart + sinThetaTo * normalizedEnd) / sinTheta;
	}
	float length1 = normalizedStart.length();
	float length2 = normalizedEnd.length();
	float length = Easing<float>::Lerp(length1, length2, t);
	return slerp * length;
}

void MakeBasis(const Vector3 &dir, Vector3 &right, Vector3 &up) {
	Vector3 n = dir.normalized();
	// dirに最も垂直な軸を選ぶ
	Vector3 temp = (fabs(n.y) < 0.99f) ? Vector3(0, 1, 0) : Vector3(1, 0, 0);
	right = temp.cross(n).normalized();
	up = n.cross(right);
}