#pragma once
#include "BaseGameScenePhase.h"

/// @brief ゲームシーンのプレイフェードアウトフェーズ
class GameScenePhasePlayFadeOut : public BaseGameScenePhase {
public:
	/// @brief デストラクタ
	~GameScenePhasePlayFadeOut() override = default;
	
	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;
};