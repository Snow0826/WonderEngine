#pragma once
#include "BasePlayerBehavior.h"
#include <cstdint>

/// @brief プレイヤーのダッシュの振る舞い
class PlayerBehaviorDash : public BasePlayerBehavior {
public:
	/// @brief デストラクタ
	~PlayerBehaviorDash() override = default;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;

private:
	uint32_t dashParameter_ = 0;	// ダッシュパラメーター
	static inline constexpr uint32_t kDashDuration = 10;	// ダッシュの継続時間
};