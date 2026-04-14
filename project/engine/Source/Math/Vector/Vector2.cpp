#include "Vector2.h"

Vector2 Vector2::normalized() const {
	float len = length();
	if (len == 0.0f) {
		return Vector2{};
	}
	return *this / len;
}

Vector2 Vector2::normalized(const Vector2 &other) const {
	float distance = distanceFrom(other);
	if (distance == 0.0f) {
		return Vector2{};
	}
	return (other - *this) / distance;
}

void Vector2::operator+=(const Vector2 &other) {
	this->x += other.x;
	this->y += other.y;
}

void Vector2::operator-=(const Vector2 &other) {
	this->x -= other.x;
	this->y -= other.y;
}

void Vector2::operator*=(const float &s) {
	x *= s;
	y *= s;
}

void Vector2::operator/=(const float &s) {
	x /= s;
	y /= s;
}