#pragma once
#include <vector>

/// @brief インスタンスデータ
struct InstanceData final {
	uint32_t instanceIndex = 0;	// インスタンスインデックス
};

class Registry;

/// @brief インスタンスアロケーター
class InstanceAllocator final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	InstanceAllocator(Registry *registry) : registry_(registry) {}

	/// @brief インスタンスデータを割り当て
	/// @param entity エンティティ
	/// @return 割り当てられたインスタンスデータ
	InstanceData Allocate(uint32_t entity);

	/// @brief インスタンスデータを解放
	/// @param entity エンティティ
	void Free(uint32_t entity);

	/// @brief デバッグ情報を表示
	void Debug() const;

private:
	Registry *registry_ = nullptr;		// レジストリ
	std::vector<uint32_t> entities_;	// インデックスからエンティティへのマッピング
};