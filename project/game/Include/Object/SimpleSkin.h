#pragma once

class Registry;
class IndirectCommandManager;
class ModelManager;
class ObjectManager;

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
	void Create();

private:
	Registry *registry_;								// レジストリ
	IndirectCommandManager *indirectCommandManager_;	// 間接コマンドマネージャー
	ModelManager *modelManager_;						// モデルマネージャー
	ObjectManager *objectManager_;						// オブジェクトマネージャー
};