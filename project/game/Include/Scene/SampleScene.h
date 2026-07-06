#pragma once
#include "BaseScene.h"
#include <vector>

class AnimatedCube;

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
	std::unique_ptr<AnimatedCube> animatedCube_;	// アニメーションするキューブ
	std::unique_ptr<DebugCamera> mainCamera_;	// メインカメラ
	std::vector<uint32_t> treeEntities_;		// 木のエンティティIDのリスト
};