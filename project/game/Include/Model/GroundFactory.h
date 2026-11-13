#pragma once

class Registry;
class InstanceAllocator;

/// @brief 地面のエンティティ作成クラス
class GroundFactory {
public:
	/// @brief 地面の作成
	/// @param registry レジストリ
	/// @param instanceAllocator インスタンスアロケータ
	static void Create(Registry &registry, InstanceAllocator &instanceAllocator);
};