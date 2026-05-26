#pragma once
#include "BaseScene.h"

class HitEffectParticle;
class SlashEffectParticle;

/// @brief サンプルシーン
class SampleScene : public BaseScene {
public:
	/// @brief コンストラクタ
	SampleScene();

	/// @brief デストラクタ
	~SampleScene() override;

	/// @brief 初期化
	void OnInitialize() override;

	/// @brief 更新
	void OnUpdate() override;

private:
	std::unique_ptr<HitEffectParticle> hitEffectParticle_ = nullptr;		// ヒットエフェクトのパーティクル
	std::unique_ptr<SlashEffectParticle> slashEffectParticle_ = nullptr;	// スラッシュエフェクトのパーティクル
};