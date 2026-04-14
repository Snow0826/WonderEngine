#pragma once
#include "BaseGameScenePhase.h"

/// @brief ゲームシーンのプレイフェードインフェーズ
class GameScenePhasePlayFadeIn : public BaseGameScenePhase {
public:
	/// @brief デストラクタ
	~GameScenePhasePlayFadeIn() override = default;
	
	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;
};