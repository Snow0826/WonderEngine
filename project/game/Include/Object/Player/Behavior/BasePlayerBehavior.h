#pragma once

class Player;

/// @brief プレイヤーの振る舞いの基底クラス
class BasePlayerBehavior {
public:
	/// @brief デストラクタ
	virtual ~BasePlayerBehavior() = default;

	/// @brief 初期化
	virtual void Initialize() = 0;

	/// @brief 更新
	virtual void Update() = 0;

	/// @brief プレイヤーの設定
	/// @param player プレイヤー
	void SetPlayer(Player *player) { player_ = player; }

protected:
	/// @brief プレイヤーの取得
	/// @return プレイヤー
	Player *GetPlayer() const { return player_; }

private:
	Player *player_ = nullptr;	// プレイヤー
};