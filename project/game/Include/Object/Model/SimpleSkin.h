#pragma once

class Registry;
class IndirectCommandManager;
class ModelManager;
class ObjectManager;
struct Vector3;

class SimpleSkin {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param indirectCommandManager 間接コマンドマネージャー
	/// @param modelManager モデルマネージャー
	/// @param objectManager オブジェクトマネージャー
	SimpleSkin(Registry *registry, IndirectCommandManager *indirectCommandManager, ModelManager *modelManager, ObjectManager *objectManager)
		: registry_(registry)
		, indirectCommandManager_(indirectCommandManager)
		, modelManager_(modelManager)
		, objectManager_(objectManager) {
	}

	/// @brief シンプルスキンの作成
	/// @param position 位置
	void Create(const Vector3 &position);

private:
	Registry *registry_ = nullptr;								// レジストリ
	IndirectCommandManager *indirectCommandManager_ = nullptr;	// 間接コマンドマネージャー
	ModelManager *modelManager_ = nullptr;						// モデルマネージャー
	ObjectManager *objectManager_ = nullptr;					// オブジェクトマネージャー
};