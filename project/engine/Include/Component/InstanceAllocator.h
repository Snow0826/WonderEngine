#pragma once
#include <vector>

/// @brief インスタンスハンドル
struct InstanceHandle final {
	uint32_t value = 0;	// 値
};

class Registry;

/// @brief インスタンスアロケーター
class InstanceAllocator final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	InstanceAllocator(Registry *registry) : registry_(registry) {}

	/// @brief インスタンスハンドルを割り当て
	/// @param entity エンティティ
	/// @return 割り当てられたインスタンスハンドル
	InstanceHandle Allocate(uint32_t entity);

	/// @brief インスタンスハンドルを解放
	/// @param entity エンティティ
	void Free(uint32_t entity);

	/// @brief デバッグ情報を表示
	void Debug() const;

private:
	Registry *registry_ = nullptr;		// レジストリ
	std::vector<uint32_t> entities_;	// インデックスからエンティティへのマッピング
};