#pragma once

class Registry;
class ModelManager;
class InstanceAllocator;
struct Vector3;

/// @brief プレイヤー
class Player {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param modelManager モデルマネージャー
	/// @param instanceAllocator インスタンスアロケータ
	Player(Registry *registry, ModelManager *modelManager, InstanceAllocator *instanceAllocator) : registry_(registry), modelManager_(modelManager), instanceAllocator_(instanceAllocator) {}

	/// @brief プレイヤーの作成
	/// @param position 位置
	void Create(const Vector3 &position);

private:
	Registry *registry_ = nullptr;						// レジストリ
	ModelManager *modelManager_ = nullptr;				// モデルマネージャー
	InstanceAllocator *instanceAllocator_ = nullptr;	// インスタンスアロケータ
};