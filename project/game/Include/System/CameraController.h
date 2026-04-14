#pragma once
#include "Vector3.h"

class Registry;
class Input;

/// @brief カメラコントローラー
class CameraController {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param input インプット
	CameraController(Registry *registry, Input *input) : registry_(registry), input_(input) {}

	/// @brief 初期化
	/// @param cameraEntity カメラエンティティ
	/// @param targetEntity カメラの対象エンティティ
	void Initialize(uint32_t cameraEntity, uint32_t targetEntity);

	/// @brief 更新
	void Update();

private:
	Registry *registry_ = nullptr;	// レジストリ
	Input *input_ = nullptr;		// インプット
	uint32_t cameraEntity_ = 0;		// カメラエンティティ
	uint32_t targetEntity_ = 0;		// カメラの対象エンティティ
	Vector3 sphericalPosition_;		// カメラの球面座標
	Vector3 targetPosition_;        // カメラの対象位置
};