#pragma once
#include "BasePlayerAttackBehavior.h"

/// @brief プレイヤーの攻撃中の振る舞い
class PlayerAttackBehaviorSwing : public BasePlayerAttackBehavior {
public:
	/// @brief デストラクタ
	~PlayerAttackBehaviorSwing() override = default;

	/// @brief 更新
	void Update() override;

private:
	uint32_t swingAttackTimer_ = 0;	// 攻撃中タイマー
	static inline constexpr uint32_t kSwingAttackDuration = 30;	// 攻撃中の継続時間
};