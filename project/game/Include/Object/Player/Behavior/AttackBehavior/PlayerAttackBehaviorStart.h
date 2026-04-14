#pragma once
#include "BasePlayerAttackBehavior.h"

/// @brief プレイヤーの攻撃開始の振る舞い
class PlayerAttackBehaviorStart : public BasePlayerAttackBehavior {
public:
	/// @brief デストラクタ
	~PlayerAttackBehaviorStart() override = default;

	/// @brief 更新
	void Update() override;

private:
	uint32_t startAttackTimer_ = 0;	// 攻撃開始タイマー
	static inline constexpr uint32_t kStartAttackDuration = 10;	// 攻撃開始の継続時間
};