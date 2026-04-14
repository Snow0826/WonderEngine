#pragma once
#include <vector>

/// @brief オブジェクト
struct Object final {
	uint32_t handle = 0;	// ハンドル
};

class Registry;

/// @brief オブジェクトマネージャー
class ObjectManager final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	ObjectManager(Registry *registry) : registry_(registry) {}

	/// @brief オブジェクトの作成
	/// @param entity エンティティ
	Object CreateObject(uint32_t entity);

	/// @brief オブジェクトの削除
	/// @param entity エンティティ
	void RemoveObject(uint32_t entity);

	/// @brief オブジェクト数のデバッグ表示
	void Debug() const;

private:
	Registry *registry_ = nullptr;		// レジストリ
	std::vector<uint32_t> entities_;	// インデックスからエンティティへのマッピング
};