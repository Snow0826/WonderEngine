#pragma once
#include "BaseScene.h"
#include <vector>

class HitEffect;
class SlashEffect;
class HitRingEffect;
class SlashRingEffect;
class MagicCircleEffect;

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
	std::unique_ptr<HitEffect> hitEffect_ = nullptr;					// ヒットエフェクト
	std::unique_ptr<SlashEffect> slashEffect_ = nullptr;				// スラッシュエフェクト
	std::unique_ptr<HitRingEffect> hitRingEffect_ = nullptr;			// ヒットリングエフェクト
	std::unique_ptr<SlashRingEffect> slashRingEffect_ = nullptr;		// スラッシュリングエフェクト
	std::unique_ptr<MagicCircleEffect> magicCircleEffect_ = nullptr;	// 魔法陣エフェクト
	std::unique_ptr<DebugCamera> mainCamera_ = nullptr;					// メインカメラ
	std::vector<uint32_t> treeEntities_;								// 木のエンティティIDのリスト
};