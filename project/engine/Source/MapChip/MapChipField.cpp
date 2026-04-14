#include "MapChipField.h"
#include "Vector3.h"
#include <map>
#include <fstream>
#include <sstream>
#include <cassert>

namespace {

    std::map<std::string, MapChipType> mapChipTable = {
        {"0", MapChipType::kBlank},
        {"1", MapChipType::kBlock},
    };

}

void MapChipField::ResetMapChipData() {
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlock.xIndex);
	for (std::vector<std::vector<MapChipType>> &mapChipDataPlane : mapChipData_.data) {
		mapChipDataPlane.resize(kNumBlock.yIndex);
		for (std::vector<MapChipType> &mapChipDataLine : mapChipDataPlane) {
			mapChipDataLine.resize(kNumBlock.zIndex);
		}
	}
}

void MapChipField::LoadMapChipCSV(const std::string& filePath) {
	// マップチップデータをリセット
	ResetMapChipData();

	/// ファイルを開く
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());
	
	// マップチップCSV
	std::stringstream mapChipCsv;
	// ファイルの内容を文字列ストリームにコピー
	mapChipCsv << file.rdbuf();
	// ファイルを閉じる
	file.close();

	// CSVからマップチップデータを読み込む
	for (uint32_t i = 0; i < kNumBlock.xIndex; ++i) {
		std::string line;
		std::getline(mapChipCsv, line);

		// 1行分の文字列をストリームに変換して解析しやすくする
		std::istringstream line_stream(line);

		for (uint32_t j = 0; j < kNumBlock.zIndex; ++j) {
			std::string word;
			std::getline(line_stream, word, ',');

			if (mapChipTable.contains(word)) {
				mapChipData_.data[i][1][j] = mapChipTable[word];
			}
		}
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex, uint32_t zIndex) const {
	if (xIndex < 0 || kNumBlock.xIndex <= xIndex ||
		yIndex < 0 || kNumBlock.yIndex <= yIndex ||
		zIndex < 0 || kNumBlock.zIndex <= zIndex) {
		return MapChipType::kBlank;
	}
	
	return mapChipData_.data[xIndex][yIndex][zIndex];
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex, uint32_t zIndex) const {
	return Vector3(kBlockLength.x * xIndex, kBlockLength.y * yIndex, kBlockLength.z * zIndex);
}

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) const {
	IndexSet indexSet;
	indexSet.xIndex = static_cast<uint32_t>((position.x + kBlockLength.x / 2.0f) / kBlockLength.x);
	indexSet.yIndex = static_cast<uint32_t>((position.y + kBlockLength.y / 2.0f) / kBlockLength.y);
	indexSet.zIndex = static_cast<uint32_t>((position.z + kBlockLength.z / 2.0f) / kBlockLength.z);
	return indexSet;
}

MapChipField::Cube MapChipField::GetCubeByIndex(uint32_t xIndex, uint32_t yIndex, uint32_t zIndex) const {
	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex, zIndex);
	Cube cube;
	cube.top = center.y + kBlockLength.y / 2.0f;
	cube.bottom = center.y - kBlockLength.y / 2.0f;
	cube.left = center.x - kBlockLength.x / 2.0f;
	cube.right = center.x + kBlockLength.x / 2.0f;
	cube.front = center.z - kBlockLength.z / 2.0f;
	cube.back = center.z + kBlockLength.z / 2.0f;
	return cube;
}

const Vector3 MapChipField::GetCenterPosition() const {
	return { kBlockLength.x * kNumBlock.xIndex / 2.0f, kBlockLength.y * kNumBlock.yIndex / 2.0f, kBlockLength.z * kNumBlock.zIndex / 2.0f };
}

void MapChipField::SetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex, uint32_t zIndex, MapChipType mapChipType) {
	if (xIndex < 0 || kNumBlock.xIndex <= xIndex ||
		yIndex < 0 || kNumBlock.yIndex <= yIndex ||
		zIndex < 0 || kNumBlock.zIndex <= zIndex) {
		return;
	}

	mapChipData_.data[xIndex][yIndex][zIndex] = mapChipType;
}