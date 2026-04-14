#pragma once
#include <array>

/// @brief マップチップとの衝突判定オブジェクト
class MapChipCollider {
public:
	/// @brief 方向
	enum Direction {
		kUp,			// 上
		kDown,			// 下
		kLeft,			// 左
		kRight,			// 右
		kFront,			// 前
		kBack,			// 後
		kNumDirection	// 方向の数
	};

	/// @brief 衝突判定オブジェクトの移動
	virtual void MoveCollider() = 0;

	/// @brief 天井に接触している場合の処理
	virtual void ContactCeiling() = 0;

	/// @brief 壁に接触している場合の処理
	virtual void ContactWall() = 0;

	/// @brief 接地状態の切り替え処理
	virtual void ChangeOnGround() = 0;

	/// @brief 平行移動を取得する
	/// @return 平行移動
	virtual const Vector3 &GetTranslate() const = 0;

	/// @brief リセット
	void Reset() {
		isHit_.fill(false);	// 衝突フラグを全てfalseにする
		moveAmount_ = { 0.0f, 0.0f, 0.0f }; // 移動量をリセット
	}

	/// @brief 当たり判定の長さを取得する
	/// @return 当たり判定の長さ
	Vector3 GetLength() const { return length_; }

	/// @brief 当たり判定の隙間を取得する
	/// @return 当たり判定の隙間
	float GetBlank() const { return blank_; }

	/// @brief 下方向の当たり判定の隙間を取得する
	/// @return 下方向の当たり判定の隙間
	float GetBottomBlank() const { return bottomBlank; }

	/// @brief 指定した方向で衝突しているかどうかを判定する
	/// @param direction 指定する方向
	/// @return 衝突しているかどうか
	bool IsHit(Direction direction) const { return isHit_[direction]; }

	/// @brief 指定した方向の衝突フラグを立てる
	/// @param direction 指定する方向
	void SetHit(Direction direction) { isHit_[direction] = true; }

	/// @brief 移動量を取得する
	/// @return 移動量
	Vector3 GetMoveAmount() const { return moveAmount_; }

	/// @brief 移動量を設定する
	/// @param amount 移動量
	void SetMoveAmount(const Vector3 &moveAmount) { moveAmount_ = moveAmount; }

private:
	Vector3 length_ = { 0.8f, 0.8f, 0.8f };	// 当たり判定の長さ
	float blank_ = 0.2f;					// 当たり判定の隙間
	float bottomBlank = 0.2f;				// 下方向の当たり判定の隙間
	std::array<bool, kNumDirection> isHit_;	// 衝突フラグ
	Vector3 moveAmount_;					// 移動量
};