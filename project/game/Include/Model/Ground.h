#pragma once

class Registry;
class ModelManager;
class InstanceAllocator;

/// @brief 地面
class Ground {
public:
	/// @brief 地面の作成
	/// @param registry レジストリ
	/// @param modelManager モデルマネージャー
	/// @param instanceAllocator インスタンスアロケータ
	static void Create(Registry *registry, ModelManager *modelManager, InstanceAllocator *instanceAllocator);
};