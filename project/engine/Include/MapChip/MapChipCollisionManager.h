#pragma once
#include "Vector3.h"
#include "MapChipField.h"
#include "MapChipCollider.h"

/// @brief マップチップとの衝突マネージャー
class MapChipCollisionManager final {
public:
	/// @brief 角の座標ペア
	struct DirectionCorner {
		Vector3 first;	// 角の1つ目の座標
		Vector3 second;	// 角の2つ目の座標
		Vector3 third;  // 角の3つ目の座標
		Vector3 fourth; // 角の4つ目の座標
	};

	/// @brief 角
	enum Corner {
		kLeftTopFront,		// 左上前
		kRightTopFront,		// 右上前
		kLeftBottomFront,	// 左下前
		kRightBottomFront,	// 右下前
		kLeftTopBack,		// 左上後
		kRightTopBack,		// 右上後
		kLeftBottomBack,	// 左下後
		kRightBottomBack,	// 右下後
		kNumCorner			// 角の数
	};

	/// @brief マップチップとの当たり判定
	/// @param mapChipCollider マップチップコライダー
	void CollisionMapChip(MapChipCollider *mapChipCollider);

	/// @brief 下方向のマップチップとの当たり判定
	/// @param mapChipCollider マップチップコライダー
	/// @return 衝突しているかどうか
	bool CollisionMapChipDown(MapChipCollider *mapChipCollider);

	/// @brief 指定した角の座標を取得する
	/// @param center 中心座標
	/// @param length 長さ
	/// @param corner 指定する角
	/// @return 指定した角の座標
	Vector3 CornerPosition(const Vector3 &center, const Vector3 &length, Corner corner) const;

	/// @brief マップチップフィールドを設定する
	/// @param mapChipField マップチップフィールド
	void SetMapChipField(MapChipField *mapChipField) { mapChipField_ = mapChipField; }

private:
	MapChipField *mapChipField_ = nullptr;	// マップチップフィールド

	/// @brief マップチップとの当たり判定
	/// @param direction 方向
	/// @param mapChipCollider マップチップコライダー
	void CollisionMapChip(MapChipCollider::Direction direction, MapChipCollider *mapChipCollider);

	/// @brief ブロックと当たっているかどうかを判定する
	/// @param position 座標
	/// @param direction 方向
	/// @return ブロックと当たっているかどうか
	bool IsHitBlock(const Vector3 &position, MapChipCollider::Direction direction);

	/// @brief 指定した方向の座標を取得する
	/// @param center 中心座標
	/// @param length 長さ
	/// @param direction 指定する方向
	/// @return 指定した方向の座標
	Vector3 DirectionPosition(const Vector3 &center, const Vector3 &length, MapChipCollider::Direction direction) const;
};