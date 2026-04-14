#pragma once

class TitleScene;

/// @brief タイトルシーンフェーズの基底クラス
class BaseTitleScenePhase {
public:
	/// @brief デストラクタ
	virtual ~BaseTitleScenePhase() = default;

	/// @brief 初期化
	virtual void Initialize() = 0;

	/// @brief 更新
	virtual void Update() = 0;

	/// @brief タイトルシーンの設定
	/// @param titleScene タイトルシーン
	void SetTitleScene(TitleScene *titleScene) { titleScene_ = titleScene; }

protected:
	/// @brief タイトルシーンの取得
	/// @return タイトルシーン
	TitleScene *GetTitleScene() const { return titleScene_; }

private:
	TitleScene *titleScene_ = nullptr;	// ゲームシーン
};