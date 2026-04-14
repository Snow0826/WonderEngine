#pragma once

class Registry;
class IndirectCommandManager;
class ModelManager;
class ObjectManager;

/// @brief 天球
class Skydome {
public:
	/// @brief 天球の作成
	/// @param registry レジストリ
	/// @param indirectCommandManager 間接コマンドマネージャー
	/// @param modelManager モデルマネージャー
	/// @param objectManager オブジェクトマネージャー
	static void Create(Registry *registry, IndirectCommandManager *indirectCommandManager, ModelManager *modelManager, ObjectManager *objectManager);
};