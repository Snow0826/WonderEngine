#pragma once
#include "BaseScene.h"

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
};