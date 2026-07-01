#pragma once

class Registry;
class IndirectCommandManager;
class ModelManager;
class ObjectManager;
class FootprintManager;

/// @brief 地面
class Ground {
public:
	/// @brief 地面の作成
	/// @param registry レジストリ
	/// @param indirectCommandManager 間接コマンドマネージャー
	/// @param modelManager モデルマネージャー
	/// @param objectManager オブジェクトマネージャー
	/// @param footprintManager フットプリントマネージャー
	static void Create(Registry *registry, IndirectCommandManager *indirectCommandManager, ModelManager *modelManager, ObjectManager *objectManager, FootprintManager *footprintManager);
};