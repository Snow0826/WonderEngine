#pragma once
#include "BaseScene.h"
#include "Vector3.h"
#include <vector>

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
	/// @brief 枝の構造体
	struct Branch {
		Vector3 start;	// 開始位置
		Vector3 end;	// 終了位置
	};

	std::vector<Branch> branches_;	// 枝のリスト
	std::unique_ptr<HitEffectParticle> hitEffectParticle_ = nullptr;		// ヒットエフェクトのパーティクル
	std::unique_ptr<SlashEffectParticle> slashEffectParticle_ = nullptr;	// スラッシュエフェクトのパーティクル

	/// @brief 枝の生成
	/// @param start 開始位置
	/// @param direction 方向
	/// @param length 長さ
	/// @param depth 深さ
	void GenerateBranch(const Vector3 &start, const Vector3 &direction, float length, int32_t depth);
};