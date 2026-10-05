#pragma once

class Registry;
class InstanceAllocator;
struct Vector3;

/// @brief アニメーションするキューブ
class AnimatedCube {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param instanceAllocator インスタンスアロケータ
	AnimatedCube(Registry *registry, InstanceAllocator *instanceAllocator) : registry_(registry), instanceAllocator_(instanceAllocator) {}

	/// @brief アニメーションするキューブの作成
	/// @param position 位置
	void Create(const Vector3 &position);

private:
	Registry *registry_ = nullptr;						// レジストリ
	InstanceAllocator *instanceAllocator_ = nullptr;	// インスタンスアロケータ
};