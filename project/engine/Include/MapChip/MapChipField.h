#pragma once
#include "Vector3.h"
#include <cstdint>
#include <vector>
#include <string>

/// @brief マップチップの種類
enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
};

/// @brief マップチップデータ
struct MapChipData final {
	std::vector<std::vector<std::vector<MapChipType>>> data;
};

/// @brief マップチップフィールド
class MapChipField final {
public:
	// ブロックの長さ
	static inline const Vector3 kBlockLength = { 1.0f, 1.0f, 1.0f };

	/// @brief XYZインデックスのセット
	struct IndexSet final {
		uint32_t xIndex = 0;	// X軸のインデックス
		uint32_t yIndex = 0;	// Y軸のインデックス
		uint32_t zIndex = 0;	// Z軸のインデックス
	};

	// ブロックの個数
	static inline const IndexSet kNumBlock = { 16, 16, 16 };

	/// @brief 立方体の境界
	struct Cube final {
		float top = 0.0f;		// 上
		float bottom = 0.0f;	// 下
		float left = 0.0f;		// 左
		float right = 0.0f;		// 右
		float front = 0.0f;		// 前
		float back = 0.0f;		// 後
	};

	/// @brief マップチップデータのリセット
	void ResetMapChipData();

	/// @brief CSVの読み込み
	/// @param filePath CSVファイルのパス
	void LoadMapChipCSV(const std::string& filePath);

	/// @brief マップチップの種類を取得
	/// @param xIndex X軸のインデックス
	/// @param yIndex Y軸のインデックス
	/// @param zIndex Z軸のインデックス
	/// @return マップチップの種類
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex, uint32_t zIndex) const;

	/// @brief マップチップの座標を取得
	/// @param xIndex X軸のインデックス
	/// @param yIndex Y軸のインデックス
	/// @param zIndex Z軸のインデックス
	/// @return マップチップの座標
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex, uint32_t zIndex) const;

	/// @brief マップチップのインデックスを取得
	/// @param position マップチップの座標
	/// @return マップチップのインデックス
	IndexSet GetMapChipIndexSetByPosition(const Vector3 &position) const;

	/// @brief 境界の座標を取得
	/// @param xIndex X軸のインデックス 
	/// @param yIndex Y軸のインデックス
	/// @param zIndex Z軸のインデックス
	/// @return 境界の座標
	Cube GetCubeByIndex(uint32_t xIndex, uint32_t yIndex, uint32_t zIndex) const;

	constexpr Vector3 GetBlockLength() const { return kBlockLength; }

	/// @brief マップチップの要素数を取得
	/// @return マップチップの要素数
	constexpr IndexSet GetNumBlock() const { return kNumBlock; }

	/// @brief マップチップの中心座標を取得
	/// @return マップチップの中心座標
	const Vector3 GetCenterPosition() const;

	/// @brief マップチップのブロックを設定
	/// @param xIndex X軸のインデックス
	/// @param yIndex Y軸のインデックス
	/// @param zIndex Z軸のインデックス
	/// @param mapChipType マップチップの種類（デフォルトは空白）
	void SetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex, uint32_t zIndex, MapChipType mapChipType = MapChipType::kBlank);

private:
	MapChipData mapChipData_; // マップチップデータ
};