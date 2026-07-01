#pragma once
#include "BasePlayerBehavior.h"

/// @brief プレイヤーの通常の振る舞い
class PlayerBehaviorRoot : public BasePlayerBehavior {
public:
	/// @brief デストラクタ
	~PlayerBehaviorRoot() override = default;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;
};