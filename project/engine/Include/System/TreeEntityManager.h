#pragma once
#include <Vector3.h>

class Registry;
class InstanceAllocator;
struct Primitive;

/// @brief 木のエンティティマネージャー
class TreeEntityManager {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param instanceAllocator インスタンスアロケーター
	TreeEntityManager(Registry *registry, InstanceAllocator *instanceAllocator) : registry_(registry) , instanceAllocator_(instanceAllocator) {}

	/// @brief 木のエンティティ作成
	/// @param primitive 木のプリミティブ
	/// @param position 木の位置
	/// @return 作成された木のエンティティID
	uint32_t CreateEntity(const Primitive &primitive, const Vector3 &position = { 0.0f, 0.0f, 0.0f });

	/// @brief 木のエンティティ削除
	/// @param entity 削除する木のエンティティID
	void DestroyEntity(uint32_t entity);

private:
	Registry *registry_ = nullptr;						// レジストリ
	InstanceAllocator *instanceAllocator_ = nullptr;	// インスタンスアロケーター
	uint32_t treeEntityCounter_ = 0;					// 木のエンティティカウンター
};