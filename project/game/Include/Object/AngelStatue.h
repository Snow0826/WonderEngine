#pragma once

class Registry;
class IndirectCommandManager;
class ModelManager;
class ObjectManager;
struct Vector3;

class AngelStatue {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param indirectCommandManager 間接コマンドマネージャー
	/// @param modelManager モデルマネージャー
	/// @param objectManager オブジェクトマネージャー
	AngelStatue(Registry *registry, IndirectCommandManager *indirectCommandManager, ModelManager *modelManager, ObjectManager *objectManager)
		: registry_(registry)
		, indirectCommandManager_(indirectCommandManager)
		, modelManager_(modelManager)
		, objectManager_(objectManager) {}

	/// @brief 建物の作成
	/// @param rotate 回転
	/// @param translate 平行移動
	void Create(const Vector3 &rotate, const Vector3 &translate);

private:
	Registry *registry_;								// レジストリ
	IndirectCommandManager *indirectCommandManager_;	// 間接コマンドマネージャー
	ModelManager *modelManager_;						// モデルマネージャー
	ObjectManager *objectManager_;						// オブジェクトマネージャー
};