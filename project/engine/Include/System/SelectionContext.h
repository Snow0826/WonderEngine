#pragma once
#include <cstdint>

/// @brief 選択コンテキスト
struct SelectionContext final {
	uint32_t selectedEntity = UINT32_MAX;	// 選択されたエンティティ
};