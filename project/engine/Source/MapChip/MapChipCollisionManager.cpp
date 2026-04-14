#include "MapChipCollisionManager.h"

void MapChipCollisionManager::CollisionMapChip(MapChipCollider *mapChipCollider) {
	for (uint32_t i = 0; i < MapChipCollider::kNumDirection; i++) {
		CollisionMapChip(static_cast<MapChipCollider::Direction>(i), mapChipCollider);
	}
}

void MapChipCollisionManager::CollisionMapChip(MapChipCollider::Direction direction, MapChipCollider *mapChipCollider) {
	const Vector3 &translate = mapChipCollider->GetTranslate();
	const Vector3 length = mapChipCollider->GetLength();
	const float kBlank = mapChipCollider->GetBlank();
	const Vector3 &moveAmount = mapChipCollider->GetMoveAmount();

	// 方向ベクトル
	std::array<Vector3, MapChipCollider::kNumDirection> directionVec = {
		Vector3{ 0.0f, 1.0f, 0.0f },	// 上
		Vector3{ 0.0f, -1.0f, 0.0f },	// 下
		Vector3{ -1.0f, 0.0f, 0.0f },	// 左
		Vector3{ 1.0f, 0.0f, 0.0f },	// 右
		Vector3{ 0.0f, 0.0f, -1.0f },	// 前
		Vector3{ 0.0f, 0.0f, 1.0f }		// 後
	};

	// 逆方向なら何もしない
	float dot = directionVec[direction].dot(moveAmount);
	if (dot <= 0.0f) {
		return;
	}

	// 移動後の頂点座標を計算する
	std::array<Vector3, kNumCorner> cornerPositionNew;
	for (uint32_t i = 0; i < kNumCorner; i++) {
		cornerPositionNew[i] = CornerPosition(translate + moveAmount, length, static_cast<Corner>(i));
	}

	// 移動後の各方向の頂点座標
	std::array<DirectionCorner, MapChipCollider::kNumDirection> directionCornerPositionNew = {
		DirectionCorner{ cornerPositionNew[kLeftTopFront], cornerPositionNew[kRightTopFront], cornerPositionNew[kLeftTopBack], cornerPositionNew[kRightTopBack] },				// 上
		DirectionCorner{ cornerPositionNew[kLeftBottomFront], cornerPositionNew[kRightBottomFront], cornerPositionNew[kLeftBottomBack], cornerPositionNew[kRightBottomBack] },	// 下
		DirectionCorner{ cornerPositionNew[kLeftBottomFront], cornerPositionNew[kLeftTopFront], cornerPositionNew[kLeftBottomBack], cornerPositionNew[kLeftTopBack] },			// 左
		DirectionCorner{ cornerPositionNew[kRightBottomFront], cornerPositionNew[kRightTopFront], cornerPositionNew[kRightBottomBack], cornerPositionNew[kRightTopBack] },		// 右
		DirectionCorner{ cornerPositionNew[kLeftTopFront], cornerPositionNew[kRightTopFront], cornerPositionNew[kLeftBottomFront], cornerPositionNew[kRightBottomFront] },		// 前
		DirectionCorner{ cornerPositionNew[kLeftTopBack], cornerPositionNew[kRightTopBack], cornerPositionNew[kLeftBottomBack], cornerPositionNew[kRightBottomBack] },			// 後
	};

	// 各方向の頂点座標がブロックと当たっているかどうかを判定する
	bool hit = false;
	hit = IsHitBlock(directionCornerPositionNew[direction].first, direction);
	hit |= IsHitBlock(directionCornerPositionNew[direction].second, direction);
	hit |= IsHitBlock(directionCornerPositionNew[direction].third, direction);
	hit |= IsHitBlock(directionCornerPositionNew[direction].fourth, direction);
	if (hit) {
		Vector3 directionPositionNow = DirectionPosition(translate, length, direction);
		Vector3 directionPositionNew = DirectionPosition(translate + moveAmount, length, direction);

		MapChipField::IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(directionPositionNow);
		MapChipField::IndexSet indexSetNew = mapChipField_->GetMapChipIndexSetByPosition(directionPositionNew);

		if (indexSetNow.xIndex != indexSetNew.xIndex || indexSetNow.yIndex != indexSetNew.yIndex || indexSetNow.zIndex != indexSetNew.zIndex) {
			MapChipField::Cube cube = mapChipField_->GetCubeByIndex(indexSetNew.xIndex, indexSetNew.yIndex, indexSetNew.zIndex);

			std::array<Vector3, MapChipCollider::kNumDirection> moveAmountTable = {
				Vector3{ moveAmount.x, std::max(0.0f, cube.bottom - translate.y - (length.y / 2.0f + kBlank)), moveAmount.z },	// 上
				Vector3{ moveAmount.x, std::min(0.0f, cube.top - translate.y + length.y / 2.0f + kBlank), moveAmount.z },		// 下
				Vector3{ std::max(0.0f, cube.right - translate.x - (length.x / 2.0f + kBlank)), moveAmount.y, moveAmount.z },	// 左
				Vector3{ std::min(0.0f, cube.left - translate.x + length.x / 2.0f + kBlank), moveAmount.y, moveAmount.z },		// 右
				Vector3{ moveAmount.x, moveAmount.y, std::max(0.0f, cube.back - translate.z - (length.z / 2.0f + kBlank)) },	// 前
				Vector3{ moveAmount.x, moveAmount.y, std::min(0.0f, cube.front - translate.z + length.z / 2.0f + kBlank) }		// 後
			};

			mapChipCollider->SetMoveAmount(moveAmountTable[direction]);
			mapChipCollider->SetHit(direction);
		}
	}
}

bool MapChipCollisionManager::CollisionMapChipDown(MapChipCollider *mapChipCollider) {
	const Vector3 &translate = mapChipCollider->GetTranslate();
	const Vector3 length = mapChipCollider->GetLength();
	const float kBottomBlank = mapChipCollider->GetBottomBlank();
	const Vector3 &moveAmount = mapChipCollider->GetMoveAmount();

	// 移動後の頂点座標を計算する
	std::array<Vector3, kNumCorner> positionNew;
	for (uint32_t i = 0; i < kNumCorner; i++) {
		positionNew[i] = CornerPosition(translate + moveAmount, length, static_cast<Corner>(i));
	}

	// 下方向の頂点座標を少し上にずらす
	positionNew[kLeftBottomFront].y -= kBottomBlank;
	positionNew[kRightBottomFront].y -= kBottomBlank;
	positionNew[kLeftBottomBack].y -= kBottomBlank;
	positionNew[kRightBottomBack].y -= kBottomBlank;

	// 下方向の頂点座標がブロックと当たっているかどうかを判定する
	bool hit = false;
	hit = IsHitBlock(positionNew[kLeftBottomFront], MapChipCollider::kDown);
	hit |= IsHitBlock(positionNew[kRightBottomFront], MapChipCollider::kDown);
	hit |= IsHitBlock(positionNew[kLeftBottomBack], MapChipCollider::kDown);
	hit |= IsHitBlock(positionNew[kRightBottomBack], MapChipCollider::kDown);
	return hit;
}

bool MapChipCollisionManager::IsHitBlock(const Vector3 &position, MapChipCollider::Direction direction) {
	// マップチップのインデックスを取得
	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(position);

	// マップチップの種類を取得
	MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, indexSet.zIndex);

	// 各方向のインデックスセットのテーブル
	std::array<MapChipField::IndexSet, MapChipCollider::kNumDirection> indexSetTable = {
		MapChipField::IndexSet{indexSet.xIndex, indexSet.yIndex + 1, indexSet.zIndex},
		MapChipField::IndexSet{indexSet.xIndex, indexSet.yIndex - 1, indexSet.zIndex},
		MapChipField::IndexSet{indexSet.xIndex + 1, indexSet.yIndex, indexSet.zIndex},
		MapChipField::IndexSet{indexSet.xIndex - 1, indexSet.yIndex, indexSet.zIndex},
		MapChipField::IndexSet{indexSet.xIndex, indexSet.yIndex, indexSet.zIndex + 1},
		MapChipField::IndexSet{indexSet.xIndex, indexSet.yIndex, indexSet.zIndex - 1}
	};

	// 次のマップチップの種類を取得
	MapChipType mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSetTable[direction].xIndex, indexSetTable[direction].yIndex, indexSetTable[direction].zIndex);

	// ブロックに当たっているかどうかを返す
	return (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock);
}

Vector3 MapChipCollisionManager::CornerPosition(const Vector3 &center, const Vector3 &length, Corner corner) const {
	std::array<Vector3, kNumCorner> offsetTable = {
		Vector3{-length.x / 2.0f, length.y / 2.0f, -length.z / 2.0f},	// kLeftTopFront
		Vector3{length.x / 2.0f,  length.y / 2.0f, -length.z / 2.0f},	// kRightTopFront
		Vector3{-length.x / 2.0f, -length.y / 2.0f, -length.z / 2.0f},	// kLeftBottomFront
		Vector3{length.x / 2.0f,  -length.y / 2.0f, -length.z / 2.0f},	// kRightBottomFront
		Vector3{-length.x / 2.0f, length.y / 2.0f, length.z / 2.0f},	// kLeftTopBack
		Vector3{length.x / 2.0f,  length.y / 2.0f, length.z / 2.0f},	// kRightTopBack
		Vector3{-length.x / 2.0f, -length.y / 2.0f, length.z / 2.0f},	// kLeftBottomBack
		Vector3{length.x / 2.0f,  -length.y / 2.0f, length.z / 2.0f}	// kRightBottomBack
	};
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

Vector3 MapChipCollisionManager::DirectionPosition(const Vector3 &center, const Vector3 &length, MapChipCollider::Direction direction) const {
	std::array<Vector3, MapChipCollider::kNumDirection> offsetTable = {
		Vector3{0.0f, length.y / 2.0f, 0.0f},		// 上
		Vector3{0.0f, -length.y / 2.0f, 0.0f},		// 下
		Vector3{-length.x / 2.0f, 0.0f, 0.0f},		// 左
		Vector3{length.x / 2.0f, 0.0f, 0.0f},		// 右
		Vector3{0.0f, 0.0f, -length.z / 2.0f},		// 前
		Vector3{0.0f, 0.0f, length.z / 2.0f}		// 後
	};
	return center + offsetTable[static_cast<uint32_t>(direction)];
}