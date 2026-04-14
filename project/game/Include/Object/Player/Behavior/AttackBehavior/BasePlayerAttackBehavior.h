#pragma once
#include "Vector3.h"
#include <numbers>

class PlayerBehaviorAttack;

/// @brief プレイヤーの攻撃振る舞いの基底クラス
class BasePlayerAttackBehavior {
public:
	/// @brief デストラクタ
	virtual ~BasePlayerAttackBehavior() = default;

	/// @brief 更新
	virtual void Update() = 0;

	/// @brief プレイヤーの攻撃の振る舞いの設定
	/// @param playerBehaviorAttack プレイヤーの攻撃の振る舞い
	void SetPlayerBehaviorAttack(PlayerBehaviorAttack *playerBehaviorAttack) { playerBehaviorAttack_ = playerBehaviorAttack; }

protected:
	static inline constexpr Vector3 kLeftArmAttackRotate = { 0.0f, -std::numbers::pi_v<float> *3.0f / 4.0f, 0.0f };	// 攻撃時の左腕の回転角度
	static inline constexpr Vector3 kLeftArmAttackTranslate = { -0.5f, 0.0f, -1.0f };	// 攻撃時の左腕の移動量
	static inline constexpr Vector3 kRightArmAttackRotate = { 0.0f, std::numbers::pi_v<float> *3.0f / 4.0f, 0.0f };	// 攻撃時の右腕の回転角度
	static inline constexpr Vector3 kRightArmAttackTranslate = { 0.5f, 0.0f, -1.0f };	// 攻撃時の右腕の移動量

	/// @brief プレイヤーの攻撃の振る舞いの取得
	/// @return プレイヤーの攻撃の振る舞い
	PlayerBehaviorAttack *GetPlayerBehaviorAttack() const { return playerBehaviorAttack_; }

private:
	PlayerBehaviorAttack *playerBehaviorAttack_ = nullptr;	// プレイヤーの攻撃の振る舞い
};