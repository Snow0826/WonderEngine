#pragma once

struct Vector2;
struct Vector3;

/// @brief 3x3行列
struct Matrix3x3 final {
	Matrix3x3 operator+(const Matrix3x3 &other) const;
	Matrix3x3 operator-(const Matrix3x3 &other) const;
	Matrix3x3 operator*(const Matrix3x3 &other) const;
	Matrix3x3 operator*(float other) const;
	Matrix3x3 operator/(float other) const;
	void operator+=(const Matrix3x3 &other);
	void operator-=(const Matrix3x3 &other);
	void operator*=(const Matrix3x3 &other);
	void operator*=(float other);
	void operator/=(float other);
	Vector2 operator*(const Vector2 &other) const;
	Vector3 operator*(const Vector3 &other) const;
	Matrix3x3 operator-() const;
	Matrix3x3 transpose() const;
	bool inverse(Matrix3x3 &result) const;

	float m[3][3] = { 0.0f };
};

/// @brief 2次元ベクトルと3x3行列の乗算
/// @param vector 2次元ベクトル
/// @param matrix 3x3行列
/// @return 変換後の2次元ベクトル
Vector2 operator*(const Vector2 &vector, const Matrix3x3 &matrix);

/// @brief 3次元ベクトルと3x3行列の乗算
/// @param vector 3次元ベクトル
/// @param matrix 3x3行列
/// @return 変換後の3次元ベクトル
Vector3 operator*(const Vector3 &vector, const Matrix3x3 &matrix);

/// @brief 単位行列の作成
/// @return 単位行列
Matrix3x3 MakeIdentity3x3();

/// @brief スケール行列の作成
/// @param scale スケールベクトル
/// @return スケール行列
Matrix3x3 MakeScaleMatrix(const Vector2 &scale);

/// @brief 回転行列の作成
/// @param radian 回転角（ラジアン）
/// @return 回転行列
Matrix3x3 MakeRotateMatrix(float radian);

/// @brief 平行移動行列の作成
/// @param translation 平行移動ベクトル
/// @return 平行移動行列
Matrix3x3 MakeTranslateMatrix(const Vector2 &translation);

/// @brief アフィン変換行列の作成
/// @param scale スケールベクトル
/// @param radian 回転角（ラジアン）
/// @param translation 平行移動ベクトル
/// @return アフィン変換行列
Matrix3x3 MakeAffineMatrix(const Vector2 &scale, float radian, const Vector2 &translation);