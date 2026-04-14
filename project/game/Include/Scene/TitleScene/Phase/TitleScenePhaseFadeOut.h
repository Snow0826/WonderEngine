#pragma once
#include "BaseTitleScenePhase.h"

/// @brief タイトルシーンのフェードアウトフェーズ
class TitleScenePhaseFadeOut : public BaseTitleScenePhase {
public:
	/// @brief デストラクタ
	~TitleScenePhaseFadeOut() override = default;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;
};