#pragma once
#include <cmath>

/// @brief 3次元ベクトル
struct Vector3 final {
	float distanceFrom(const Vector3 &other) const { return (other - *this).length(); }
	float length() const { return std::sqrt(lengthSquare()); }
	constexpr float lengthSquare() const { return dot(*this); }
	constexpr float dot(const Vector3 &other) const { return this->x * other.x + this->y * other.y + this->z * other.z; }
	constexpr Vector3 cross(const Vector3 &other) const {
		return {
			this->y * other.z - this->z * other.y,
			this->z * other.x - this->x * other.z,
			this->x * other.y - this->y * other.x
		};
	}
	Vector3 normalized() const;
	Vector3 normalized(const Vector3 &other) const;

	/// @brief 垂直なベクトルを求める関数
	/// @return 垂直なベクトル
	Vector3 perpendicular() const;

	constexpr bool isZero() const { return *this == Vector3{}; }
	constexpr bool operator==(const Vector3 &other) const { return this->x == other.x && this->y == other.y && this->z == other.z; }
	constexpr bool operator!=(const Vector3 &other) const { return !(*this == other); }
	constexpr Vector3 operator+() const { return *this; }
	constexpr Vector3 operator-() const { return { -x, -y, -z }; }
	constexpr Vector3 operator+(const Vector3 &other) const { return { this->x + other.x, this->y + other.y, this->z + other.z }; }
	constexpr Vector3 operator-(const Vector3 &other) const { return { this->x - other.x, this->y - other.y, this->z - other.z }; }
	constexpr Vector3 operator*(const float &s) const { return { x * s, y * s, z * s }; }
	constexpr Vector3 operator/(const float &s) const { return { x / s, y / s, z / s }; }
	void operator+=(const Vector3 &other);
	void operator-=(const Vector3 &other);
	void operator*=(const float &s);
	void operator/=(const float &s);

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

inline constexpr Vector3 operator*(const float &s, const Vector3 &v) { return { s * v.x, s * v.y, s * v.z }; }
inline constexpr Vector3 operator/(const float &s, const Vector3 &v) { return { s / v.x, s / v.y, s / v.z }; }

/// @brief 座標変換(球面座標→デカルト座標)
/// @param spherical 球面座標
/// @return デカルト座標
Vector3 ToCartesian(const Vector3 &spherical);

/// @brief 座標変換(デカルト座標→球面座標)
/// @param cartesian デカルト座標
/// @return 球面座標
Vector3 ToSpherical(const Vector3 &cartesian);

/// @brief 角度の線形補間
/// @param start 始点ベクトル
/// @param end 終点ベクトル
/// @param t 補間割合
/// @return 補間ベクトル
Vector3 LerpShortAngle(const Vector3 &start, const Vector3 &end, float t);

/// @brief 球面線形補間
/// @param start 始点ベクトル
/// @param end 終点ベクトル
/// @param t 補間割合
/// @return 補間ベクトル 
Vector3 Slerp(const Vector3 &start, const Vector3 &end, float t);

/// @brief 基底ベクトルを作成する関数
/// @param dir 基底ベクトルの向き
/// @param right 基底ベクトルの右方向
/// @param up 基底ベクトルの上方向
void MakeBasis(const Vector3 &dir, Vector3 &right, Vector3 &up);