#pragma once

class Registry;
class InstanceAllocator;
struct Vector3;

/// @brief シンプルスキン
class SimpleSkin {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param instanceAllocator インスタンスアロケータ
	SimpleSkin(Registry *registry, InstanceAllocator *instanceAllocator) : registry_(registry), instanceAllocator_(instanceAllocator) {}

	/// @brief シンプルスキンの作成
	/// @param position 位置
	void Create(const Vector3 &position);

private:
	Registry *registry_ = nullptr;						// レジストリ
	InstanceAllocator *instanceAllocator_ = nullptr;	// インスタンスアロケータ
};