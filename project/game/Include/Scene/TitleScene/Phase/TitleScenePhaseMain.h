#pragma once
#include "BaseTitleScenePhase.h"

/// @brief タイトルシーンのメインフェーズ
class TitleScenePhaseMain : public BaseTitleScenePhase {
public:
	/// @brief デストラクタ
	~TitleScenePhaseMain() override = default;

	/// @brief 初期化
	void Initialize() override {}

	/// @brief 更新
	void Update() override;
};