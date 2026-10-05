#pragma once
#include <string>

class Registry;
class InstanceAllocator;
struct Vector3;

/// @brief 人間
class Human {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param instanceAllocator インスタンスアロケータ
	Human(Registry *registry, InstanceAllocator *instanceAllocator) : registry_(registry), instanceAllocator_(instanceAllocator) {}

	/// @brief 人間の作成
	/// @param fileName モデルファイル名
	/// @param position 位置
	void Create(const std::string &fileName, const Vector3 &position);

private:
	Registry *registry_ = nullptr;						// レジストリ
	InstanceAllocator *instanceAllocator_ = nullptr;	// インスタンスアロケータ
};