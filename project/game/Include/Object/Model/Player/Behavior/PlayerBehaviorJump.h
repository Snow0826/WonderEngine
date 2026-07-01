#pragma once
#include "BasePlayerBehavior.h"

/// @brief プレイヤーのジャンプの振る舞い
class PlayerBehaviorJump : public BasePlayerBehavior {
public:
	/// @brief デストラクタ
	~PlayerBehaviorJump() override = default;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;
};