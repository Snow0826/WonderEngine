#pragma once

class GameScene;

/// @brief ゲームシーンフェーズの基底クラス
class BaseGameScenePhase {
public:
	/// @brief デストラクタ
	virtual ~BaseGameScenePhase() = default;

	/// @brief 初期化
	virtual void Initialize() = 0;

	/// @brief 更新
	virtual void Update() = 0;

	/// @brief ゲームシーンの設定
	/// @param gameScene ゲームシーン
	void SetGameScene(GameScene *gameScene) { gameScene_ = gameScene; }

protected:
	/// @brief ゲームシーンの取得
	/// @return ゲームシーン
	GameScene *GetGameScene() const { return gameScene_; }

private:
	GameScene *gameScene_ = nullptr;	// ゲームシーン
};