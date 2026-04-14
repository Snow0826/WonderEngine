#pragma once
#include "BaseGameScenePhase.h"

/// @brief ゲームシーンのクリアフェードアウトフェーズ
class GameScenePhaseClearFadeOut : public BaseGameScenePhase {
public:
	/// @brief デストラクタ
	~GameScenePhaseClearFadeOut() override = default;
	
	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;
};