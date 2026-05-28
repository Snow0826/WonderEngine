#pragma once
#include "BaseScene.h"

class HitEffect;
class SlashEffect;
class HitRingEffect;
class SlashRingEffect;

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
	std::unique_ptr<HitEffect> hitEffect_ = nullptr;				// ヒットエフェクト
	std::unique_ptr<SlashEffect> slashEffect_ = nullptr;			// スラッシュエフェクト
	std::unique_ptr<HitRingEffect> hitRingEffect_ = nullptr;		// ヒットリングエフェクト
	std::unique_ptr<SlashRingEffect> slashRingEffect_ = nullptr;	// スラッシュリングエフェクト
};