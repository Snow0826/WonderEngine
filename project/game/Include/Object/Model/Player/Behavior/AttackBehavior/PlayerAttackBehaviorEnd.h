#pragma once
#include "BasePlayerAttackBehavior.h"

/// @brief プレイヤーの攻撃終了の振る舞い
class PlayerAttackBehaviorEnd : public BasePlayerAttackBehavior {
public:
	/// @brief デストラクタ
	~PlayerAttackBehaviorEnd() override = default;
	
	/// @brief 更新
	void Update() override;

private:
	uint32_t endAttackTimer_ = 0;	// 攻撃終了タイマー
	static inline constexpr uint32_t kEndAttackDuration = 10;	// 攻撃終了の継続時間
};