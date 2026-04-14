#pragma once

struct Vector3;
struct Vector4;
struct Quaternion;

/// @brief 4x4行列
struct Matrix4x4 final {
	Matrix4x4 operator+(const Matrix4x4 &other) const;
	Matrix4x4 operator-(const Matrix4x4 &other) const;
	Matrix4x4 operator*(const Matrix4x4 &other) const;
	Matrix4x4 operator*(float other) const;
	Matrix4x4 operator/(float other) const;
	void operator+=(const Matrix4x4 &other);
	void operator-=(const Matrix4x4 &other);
	void operator*=(const Matrix4x4 &other);
	void operator*=(float other);
	void operator/=(float other);
	Vector3 operator*(const Vector3 &other) const;
	Vector4 operator*(const Vector4 &other) const;
	Matrix4x4 operator-() const;
	Matrix4x4 transpose() const;
	Matrix4x4 inverse() const;

	float m[4][4] = { 0.0f };
};

/// @brief 3次元ベクトルと4x4行列の乗算
/// @param vector 3次元ベクトル
/// @param matrix 4x4行列
/// @return 変換後の3次元ベクトル
Vector3 operator*(const Vector3 &vector, const Matrix4x4 &matrix);

/// @brief 4次元ベクトルと4x4行列の乗算
/// @param vector 4次元ベクトル
/// @param matrix 4x4行列
/// @return 変換後の4次元ベクトル
Vector4 operator*(const Vector4 &vector, const Matrix4x4 &matrix);

/// @brief 3次元ベクトルの回転
/// @param vector 3次元ベクトル
/// @param matrix 4x4行列
/// @return 変換後の3次元ベクトル
Vector3 TransformNormal(const Vector3 &vector, const Matrix4x4 &matrix);

/// @brief 3次元ベクトルの絶対値回転
/// @param vector 3次元ベクトル
/// @param matrix 4x4行列
/// @return 変換後の3次元ベクトル
Vector3 TransformNormalAbs(const Vector3 &vector, const Matrix4x4 &matrix);

/// @brief 単位行列の作成
/// @return 単位行列
Matrix4x4 MakeIdentity4x4();

/// @brief スケール行列の作成
/// @param scale スケールベクトル
/// @return スケール行列
Matrix4x4 MakeScaleMatrix(const Vector3 &scale);

/// @brief X軸回転行列の作成
/// @param radian 回転角（ラジアン）
/// @return X軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian);

/// @brief Y軸回転行列の作成
/// @param radian 回転角（ラジアン）
/// @return Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian);

/// @brief Z軸回転行列の作成
/// @param radian 回転角（ラジアン）
/// @return Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian);

/// @brief 回転行列の作成
/// @param rotate 回転ベクトル（ラジアン）
/// @return 回転行列
Matrix4x4 MakeRotateMatrix(const Vector3 &rotate);

/// @brief 任意軸回転行列の作成
/// @param axis 回転軸ベクトル
/// @param angle 回転角（ラジアン）
/// @return 任意軸回転行列
Matrix4x4 MakeRotateMatrixAroundAxis(const Vector3 &axis, float angle);

/// @brief 方向ベクトルから方向ベクトルへの回転行列の作成
/// @param from 元の方向ベクトル
/// @param to 目標の方向ベクトル
/// @return 回転行列
Matrix4x4 DirectionToDirection(const Vector3 &from, const Vector3 &to);

/// @brief ルックアット行列の作成
/// @param eye 視点位置
/// @param target 注視点位置
/// @param up 上方向ベクトル
/// @return ルックアット行列
Matrix4x4 LookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up);

/// @brief 平行移動行列の作成
/// @param translate 平行移動ベクトル
/// @return 平行移動行列
Matrix4x4 MakeTranslateMatrix(const Vector3 &translate);

/// @brief アフィン変換行列の作成
/// @param scale スケールベクトル
/// @param rotate 回転ベクトル（オイラー角）
/// @param translate 平行移動ベクトル
/// @return アフィン変換行列
Matrix4x4 MakeAffineMatrix(const Vector3 &scale, const Vector3 &rotate, const Vector3 &translate);

/// @brief アフィン変換行列の作成
/// @param scale スケールベクトル
/// @param rotateMatrix 回転行列
/// @param translate 平行移動ベクトル
/// @return アフィン変換行列
Matrix4x4 MakeAffineMatrix(const Vector3 &scale, const Matrix4x4 &rotateMatrix, const Vector3 &translate);

/// @brief アフィン変換行列の作成
/// @param scale スケールベクトル
/// @param rotate 回転（クォータニオン）
/// @param translate 平行移動ベクトル
/// @return アフィン変換行列
Matrix4x4 MakeAffineMatrix(const Vector3 &scale, const Quaternion &rotate, const Vector3 &translate);

/// @brief 透視投影行列の作成
/// @param fovY 垂直視野角（ラジアン）
/// @param aspectRatio アスペクト比
/// @param nearClip 近接クリッピング平面
/// @param farClip 遠方クリッピング平面
/// @return 透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

/// @brief 正射影行列の作成
/// @param left 左
/// @param top 上
/// @param right 右
/// @param bottom 下
/// @param nearClip 近接クリッピング平面
/// @param farClip 遠方クリッピング平面
/// @return 正射影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

/// @brief ビューポート行列の作成
/// @param left 左
/// @param top 上
/// @param width 幅
/// @param height 高さ
/// @param minDepth 最小深度
/// @param maxDepth 最大深度
/// @return ビューポート行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);