#pragma once

struct Vector3;
struct Matrix4x4;

/// @brief クォータニオン
struct Quaternion final {
	/// @brief 乗算演算子
	/// @param other 乗算するクォータニオン
	/// @return 乗算結果のクォータニオン
	Quaternion operator*(const Quaternion &other) const;

	/// @brief 等価比較演算子
	/// @param other 比較対象のクォータニオン
	/// @return 等しい場合はtrue、それ以外はfalse
	bool operator==(const Quaternion &other) const;

	/// @brief 非等価比較演算子
	/// @param other 比較対象のクォータニオン
	/// @return 等しくない場合はtrue、それ以外はfalse
	bool operator!=(const Quaternion &other) const;

	/// @brief 単位クォータニオンを取得
	/// @return 単位クォータニオン
	static Quaternion IdentityQuaternion();

	/// @brief 共役クォータニオンを取得
	/// @return 共役クォータニオン
	Quaternion Conjugate() const;

	/// @brief ノルムを取得
	/// @return ノルム
	float Norm() const;

	/// @brief 正規化クォータニオンを取得
	/// @return 正規化クォータニオン
	Quaternion Normalize() const;

	/// @brief 逆クォータニオンを取得
	/// @return 逆クォータニオン
	Quaternion Inverse() const;

	/// @brief ベクトルをクォータニオンで回転させる
	/// @param v 回転させるベクトル
	/// @return 回転後のベクトル
	Vector3 RotateVector(const Vector3 &v);

	/// @brief 回転行列を作成
	/// @return 回転行列
	Matrix4x4 MakeRotateMatrix() const;

	/// @brief 軸と角度から回転クォータニオンを作成
	/// @param axis 回転軸
	/// @param angle 回転角度（ラジアン）
	/// @return 回転クォータニオン
	static Quaternion MakeRotateAxisAngleQuaternion(const Vector3 &axis, float angle);

	/// @brief 方向ベクトルから方向ベクトルへの回転クォータニオンを作成
	/// @param from 元の方向ベクトル
	/// @param to 目標の方向ベクトル
	/// @return 回転クォータニオン
	static Quaternion DirectionToDirection(const Vector3 &from, const Vector3 &to);


	/// @brief 指定した前方ベクトルと上方向ベクトルからクォータニオンを作成
	/// @param forward 前方ベクトル
	/// @param up 上方向ベクトル
	/// @return 作成されたクォータニオン
	static Quaternion LookRotation(const Vector3 &forward, const Vector3 &up);

	/// @brief 球面線形補間(Slerp)
	/// @param q0 開始クォータニオン
	/// @param q1 終了クォータニオン
	/// @param t 補間パラメータ（0.0〜1.0）
	/// @return 補間結果のクォータニオン
	static Quaternion Slerp(const Quaternion &q0, const Quaternion &q1, float t);

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;
};