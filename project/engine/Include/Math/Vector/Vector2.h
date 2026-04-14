#pragma once
#include <cmath>

/// @brief 2次元ベクトル
struct Vector2 final {
	float distanceFrom(const Vector2 &other) const { return (other - *this).length(); }
	float length() const { return std::sqrt(lengthSquare()); }
	constexpr float lengthSquare() const { return dot(*this); }
	constexpr float dot(const Vector2 &other) const { return this->x * other.x + this->y * other.y; }
	constexpr float cross(const Vector2 &other) const { return this->x * other.y - this->y * other.x; }
	Vector2 normalized() const;
	Vector2 normalized(const Vector2 &other) const;
	constexpr bool isZero() const { return *this == Vector2{}; }
	constexpr bool operator==(const Vector2 &other) const { return this->x == other.x && this->y == other.y; }
	constexpr bool operator!=(const Vector2 &other) const { return !(*this == other); }
	constexpr Vector2 operator+() const { return *this; }
	constexpr Vector2 operator-() const { return { -x, -y }; }
	constexpr Vector2 operator+(const Vector2 &other) const { return { this->x + other.x, this->y + other.y }; }
	constexpr Vector2 operator-(const Vector2 &other) const { return { this->x - other.x, this->y - other.y }; }
	constexpr Vector2 operator*(const float &s) const { return { x * s, y * s }; }
	constexpr Vector2 operator/(const float &s) const { return { x / s, y / s }; }
	void operator+=(const Vector2 &other);
	void operator-=(const Vector2 &other);
	void operator*=(const float &s);
	void operator/=(const float &s);

	float x = 0.0f;
	float y = 0.0f;
};

inline constexpr Vector2 operator*(const float &s, const Vector2 &v) { return { s * v.x, s * v.y }; }
inline constexpr Vector2 operator/(const float &s, const Vector2 &v) { return { s / v.x, s / v.y }; }