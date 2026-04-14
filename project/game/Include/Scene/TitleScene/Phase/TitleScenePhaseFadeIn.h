#pragma once
#include "BaseTitleScenePhase.h"

/// @brief タイトルシーンのフェードインフェーズ
class TitleScenePhaseFadeIn : public BaseTitleScenePhase {
public:
	/// @brief デストラクタ
	~TitleScenePhaseFadeIn() override = default;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;
};