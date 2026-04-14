#pragma once
#include "BaseGameScenePhase.h"
#include <cstdint>

/// @brief ゲームシーンのクリアフェーズ
class GameScenePhaseClear : public BaseGameScenePhase {
public:
	/// @brief デストラクタ
	~GameScenePhaseClear() override = default;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;

private:
	uint32_t entity_ = 0; // エンティティ
};