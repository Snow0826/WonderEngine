#pragma once
#include <string>

class Registry;
class ModelManager;
class InstanceAllocator;
struct Vector3;

class Human {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param modelManager モデルマネージャー
	/// @param instanceAllocator インスタンスアロケータ
	Human(Registry *registry, ModelManager *modelManager, InstanceAllocator *instanceAllocator) : registry_(registry), modelManager_(modelManager), instanceAllocator_(instanceAllocator) {}

	/// @brief 人間の作成
	/// @param fileName モデルファイル名
	/// @param position 位置
	void Create(const std::string &fileName, const Vector3 &position);

private:
	Registry *registry_ = nullptr;						// レジストリ
	ModelManager *modelManager_ = nullptr;				// モデルマネージャー
	InstanceAllocator *instanceAllocator_ = nullptr;	// インスタンスアロケータ
};