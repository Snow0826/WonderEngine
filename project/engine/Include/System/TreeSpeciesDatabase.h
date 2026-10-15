#pragma once
#include "TreeGenerator.h"

/// @brief 樹種
enum class TreeSpecies {
	Zelkova,	// ケヤキ
	Kunugi,		// クヌギ
	Konara,		// コナラ
	Beech,		// ブナ
	Cedar,		// スギ
	Pine,		// マツ
	Willow		// ヤナギ
};

/// @brief 指定された樹種の設定を生成する
/// @param species 樹種
/// @param rootPosition 根の位置
/// @return 生成された木の設定
TreeConfig MakeTreeConfig(TreeSpecies species, const Vector3 &rootPosition);