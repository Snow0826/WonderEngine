#pragma once
#include <cmath>

/// @brief 4次元ベクトル
struct Vector4 final {
	constexpr Vector4() = default;
	constexpr Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	constexpr Vector4(float matrix[4]) : x(matrix[0]), y(matrix[1]), z(matrix[2]), w(matrix[3]) {}

	constexpr uint32_t ToRGBA() const {
		return static_cast<uint32_t>(x) << 24
			| static_cast<uint32_t>(y) << 16
			| static_cast<uint32_t>(z) << 8
			| static_cast<uint32_t>(w);
	}

	float distanceFrom(const Vector4 &other) const { return (other - *this).length(); }
	float length() const { return std::sqrt(lengthSquare()); }
	constexpr float lengthSquare() const { return dot(*this); }
	constexpr float dot(const Vector4 &other) const { return this->x * other.x + this->y * other.y + this->z * other.z + this->w * other.w; }
	Vector4 normalized() const;
	Vector4 normalized(const Vector4 &other) const;
	constexpr bool isZero() const { return *this == Vector4{}; }
	constexpr bool operator==(const Vector4 &other) const { return this->x == other.x && this->y == other.y && this->z == other.z && this->w == other.w; }
	constexpr bool operator!=(const Vector4 &other) const { return !(*this == other); }
	constexpr Vector4 operator+() const { return *this; }
	constexpr Vector4 operator-() const { return { -x, -y, -z, -w }; }
	constexpr Vector4 operator+(const Vector4 &other) const { return { this->x + other.x, this->y + other.y, this->z + other.z, this->w + other.w }; }
	constexpr Vector4 operator-(const Vector4 &other) const { return { this->x - other.x, this->y - other.y, this->z - other.z, this->w - other.w }; }
	constexpr Vector4 operator*(const float &s) const { return { x * s, y * s, z * s, w * s }; }
	constexpr Vector4 operator/(const float &s) const { return { x / s, y / s, z / s, w / s }; }
	void operator+=(const Vector4 &other);
	void operator-=(const Vector4 &other);
	void operator*=(const float &s);
	void operator/=(const float &s);

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;
};

inline constexpr Vector4 ToVector4(const uint32_t &color) {
	return {
		static_cast<float>(color >> 24 & 0xFF),
		static_cast<float>(color >> 16 & 0xFF),
		static_cast<float>(color >> 8 & 0xFF),
		static_cast<float>(color & 0xFF)
	};
}

inline constexpr Vector4 operator*(const float &s, const Vector4 &v) { return { s * v.x, s * v.y, s * v.z, s * v.w }; }
inline constexpr Vector4 operator/(const float &s, const Vector4 &v) { return { s / v.x, s / v.y, s / v.z, s / v.w }; }