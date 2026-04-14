#pragma once
#include "BaseGameScenePhase.h"
#include <memory>

class DeathParticle;

/// @brief ゲームシーンのデス演出フェーズ
class GameScenePhaseDeath : public BaseGameScenePhase {
public:
	/// @brief コンストラクタ
	GameScenePhaseDeath();

	/// @brief デストラクタ
	~GameScenePhaseDeath() override;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;

private:
	std::unique_ptr<DeathParticle> deathParticle_ = nullptr;	// デスパーティクル
};