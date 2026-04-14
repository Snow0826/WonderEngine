#pragma once
#include "BaseGameScenePhase.h"

/// @brief ゲームシーンのクリアフェードインフェーズ
class GameScenePhaseClearFadeIn : public BaseGameScenePhase {
public:
	/// @brief デストラクタ
	~GameScenePhaseClearFadeIn() override = default;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;
};