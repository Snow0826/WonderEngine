#include "Vector4.h"

Vector4 Vector4::normalized() const {
	float len = length();
	if (len == 0.0f) {
		return Vector4{};
	}
	return *this / len;
}

Vector4 Vector4::normalized(const Vector4 &other) const {
	float distance = distanceFrom(other);
	if (distance == 0.0f) {
		return Vector4{};
	}
	return (other - *this) / distance;
}

void Vector4::operator+=(const Vector4 &other) {
	this->x += other.x;
	this->y += other.y;
	this->z += other.z;
	this->w += other.w;
}

void Vector4::operator-=(const Vector4 &other) {
	this->x -= other.x;
	this->y -= other.y;
	this->z -= other.z;
	this->w -= other.w;
}

void Vector4::operator*=(const float &s) {
	this->x *= s;
	this->y *= s;
	this->z *= s;
	this->w *= s;
}

void Vector4::operator/=(const float &s) {
	this->x /= s;
	this->y /= s;
	this->z /= s;
	this->w /= s;
}