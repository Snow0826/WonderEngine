#pragma once

class Registry;
class IndirectCommandManager;
class ModelManager;
class ObjectManager;
struct Vector3;

/// @brief アニメーションするキューブ
class AnimatedCube {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param indirectCommandManager 間接コマンドマネージャー
	/// @param modelManager モデルマネージャー
	/// @param objectManager オブジェクトマネージャー
	AnimatedCube(Registry *registry, IndirectCommandManager *indirectCommandManager, ModelManager *modelManager, ObjectManager *objectManager)
		: registry_(registry)
		, indirectCommandManager_(indirectCommandManager)
		, modelManager_(modelManager)
		, objectManager_(objectManager) {
	}

	/// @brief アニメーションするキューブの作成
	/// @param position 位置
	void Create(const Vector3 &position);

private:
	Registry *registry_;								// レジストリ
	IndirectCommandManager *indirectCommandManager_;	// 間接コマンドマネージャー
	ModelManager *modelManager_;						// モデルマネージャー
	ObjectManager *objectManager_;						// オブジェクトマネージャー
};