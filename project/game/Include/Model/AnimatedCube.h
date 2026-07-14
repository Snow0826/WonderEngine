#pragma once

class Registry;
class ModelManager;
struct Vector3;

/// @brief アニメーションするキューブ
class AnimatedCube {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param modelManager モデルマネージャー
	AnimatedCube(Registry *registry, ModelManager *modelManager) : registry_(registry) , modelManager_(modelManager) {}

	/// @brief アニメーションするキューブの作成
	/// @param position 位置
	void Create(const Vector3 &position);

private:
	Registry *registry_;			// レジストリ
	ModelManager *modelManager_;	// モデルマネージャー
};