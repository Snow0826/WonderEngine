#pragma once
#include "BaseGameScenePhase.h"

/// @brief ゲームシーンのプレイフェーズ
class GameScenePhasePlay : public BaseGameScenePhase {
public:
	/// @brief デストラクタ
	~GameScenePhasePlay() override = default;
	
	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;
};