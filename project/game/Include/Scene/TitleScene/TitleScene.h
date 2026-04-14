#pragma once
#include "BaseScene.h"

class BaseTitleScenePhase;
class Button;

/// @brief タイトルシーン
class TitleScene : public BaseScene {
public:
	/// @brief コンストラクタ
	TitleScene();

	/// @brief デストラクタ
	~TitleScene() override;

	/// @brief 初期化
	void OnInitialize() override;

	/// @brief 更新
	void OnUpdate() override;

	/// @brief フェーズの切り替え
	/// @param newBaseTitleScenePhase 新しいフェーズ
	void ChangePhase(BaseTitleScenePhase *newBaseTitleScenePhase);

	/// @brief フェードの取得
	/// @return フェード
	Fade *GetFade() const { return fade_.get(); }

	/// @brief シーンマネージャーの取得
	/// @return シーンマネージャー
	SceneManager *GetSceneManager() const { return sceneManager_; }

	/// @brief スタートボタンの取得
	/// @return スタートボタン
	Button *GetStartButton() const { return startButton_.get(); }

private:
	BaseTitleScenePhase *phase_ = nullptr;			// 現在のフェーズ
	std::unique_ptr<Button> startButton_ = nullptr;	// スタートボタン
};