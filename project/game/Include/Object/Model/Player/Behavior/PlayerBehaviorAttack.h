#pragma once
#include "BasePlayerBehavior.h"
#include "Vector3.h"
#include <memory>

class BasePlayerAttackBehavior;
class Player;

/// @brief プレイヤーの攻撃の振る舞い
class PlayerBehaviorAttack : public BasePlayerBehavior {
public:
	/// @brief コンストラクタ
	PlayerBehaviorAttack();

	/// @brief デストラクタ
	~PlayerBehaviorAttack() override;
	
	/// @brief 初期化
	void Initialize() override;
	
	/// @brief 更新
	void Update() override;

	/// @brief 次の攻撃振る舞いの設定
	/// @param nextAttackBehavior 次の攻撃振る舞い
	void SetNextAttackBehavior(std::unique_ptr<BasePlayerAttackBehavior> nextAttackBehavior);

	/// @brief プレイヤーの取得
	/// @return プレイヤー
	Player *GetPlayer() const;

	/// @brief 左腕の回転角度の取得
	/// @return 左腕の回転角度
	Vector3 GetLeftArmRotate() const { return leftArmRotate_; }

	/// @brief 左腕の移動量の取得
	/// @return 左腕の移動量
	Vector3 GetLeftArmTranslate() const { return leftArmTranslate_; }

	/// @brief 右腕の回転角度の取得
	/// @return 右腕の回転角度
	Vector3 GetRightArmRotate() const { return rightArmRotate_; }

	/// @brief 右腕の移動量の取得
	/// @return 右腕の移動量
	Vector3 GetRightArmTranslate() const { return rightArmTranslate_; }

private:
	std::unique_ptr<BasePlayerAttackBehavior> currentAttackBehavior_ = nullptr;	// 現在の攻撃振る舞い
	std::unique_ptr<BasePlayerAttackBehavior> nextAttackBehavior_ = nullptr;	// 次の攻撃振る舞い
	Vector3 leftArmRotate_;		// 左腕の回転角度
	Vector3 leftArmTranslate_;	// 左腕の移動量
	Vector3 rightArmRotate_;	// 右腕の回転角度
	Vector3 rightArmTranslate_;	// 右腕の移動量
};