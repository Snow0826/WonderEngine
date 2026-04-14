#pragma once
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
#include <string>

/// @brief 親エンティティを持つフラグ
struct HasParent final {};

/// @brief 変換データが変更されたフラグ
struct DirtyTransform final {};

/// @brief 変換データ
struct Transform final {
	Vector3 scale = { 1.0f, 1.0f, 1.0f };				// スケール
	Vector3 rotate = { 0.0f, 0.0f, 0.0f };				// 回転
	Vector3 translate = { 0.0f, 0.0f, 0.0f };			// 平行移動
	Vector3 pivot = { 0.0f, 0.0f, 0.0f };				// ピボット（回転中心）
	Quaternion quaternion = { 0.0f, 0.0f, 0.0f, 1.0f };	// 回転クォータニオン
	Matrix4x4 rotateMatrix = MakeIdentity4x4();			// 回転行列
	Matrix4x4 worldMatrix = MakeIdentity4x4();			// ワールド行列
	uint32_t parentEntity = 0;							// 親エンティティ
};

/// @brief 変換行列データ
struct TransformationMatrix final {
	Matrix4x4 worldMatrix = MakeIdentity4x4();					// ワールド行列
	Matrix4x4 worldInverseTransposeMatrix = MakeIdentity4x4();	// ワールド逆転置行列
};

class Registry;

/// @brief 変換システム
class TransformSystem final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	TransformSystem(Registry *registry) : registry_(registry) {}

	/// @brief 変換フラグの設定
	/// @param entity エンティティ
	void MarkDirty(uint32_t entity);

	/// @brief ワールド行列の更新
	void UpdateWorldMatrix();

	/// @brief 当たり判定データの変換
	void TransformCollisionData();

	/// @brief ローカル座標系の右方向ベクトルの取得
	/// @param entity エンティティ
	/// @return 右方向ベクトル
	Vector3 GetRight(uint32_t entity);

	/// @brief ローカル座標系の上方向ベクトルの取得
	/// @param entity エンティティ
	/// @return 上方向ベクトル
	Vector3 GetUp(uint32_t entity);

	/// @brief ローカル座標系の前方向ベクトルの取得
	/// @param entity エンティティ
	/// @return 前方向ベクトル
	Vector3 GetForward(uint32_t entity);

	/// @brief ワールド座標の取得
	/// @param entity エンティティ
	/// @return ワールド座標
	Vector3 GetWorldPosition(uint32_t entity);

private:
	Registry *registry_ = nullptr;
};

/// @brief 変換データインスペクター
class TransformInspector {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	TransformInspector(Registry *registry) : registry_(registry) {}

	/// @brief 変換データインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;
};