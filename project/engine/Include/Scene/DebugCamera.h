#pragma once
#include "Vector3.h"
#include <string>

class Registry;
class Input;

/// @brief デバッグカメラ
class DebugCamera final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param input 入力
	DebugCamera(Registry *registry, Input *input) : registry_(registry), input_(input) {}

	/// @brief 初期化
	/// @param cameraEntity カメラエンティティ
	void Initialize(uint32_t cameraEntity);

	/// @brief 更新
	void Update();

	/// @brief リセット
	void Reset();

	/// @brief 編集
	/// @param label ラベル
	void Edit(const std::string &label);

	/// @brief リセット位置の設定
	/// @param position 位置
	void SetResetPosition(const Vector3 &position) { resetPosition_ = position; }

private:
	Registry *registry_ = nullptr;	// レジストリ
	Input *input_ = nullptr;		// 入力
	uint32_t cameraEntity_ = 0;		// カメラエンティティ
	Vector3 targetPosition_;		// カメラの対象座標
	Vector3 sphericalPosition_;		// カメラの球面座標
	Vector3 resetPosition_;			// リセット位置
};