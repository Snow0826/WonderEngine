#pragma once
#include <string>

class Registry;
class IndirectCommandManager;
class ModelManager;
class ObjectManager;
struct Vector3;

class Human {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param indirectCommandManager 間接コマンドマネージャー
	/// @param modelManager モデルマネージャー
	/// @param objectManager オブジェクトマネージャー
	Human(Registry *registry, IndirectCommandManager *indirectCommandManager, ModelManager *modelManager, ObjectManager *objectManager)
		: registry_(registry)
		, indirectCommandManager_(indirectCommandManager)
		, modelManager_(modelManager)
		, objectManager_(objectManager) {
	}

	/// @brief 人間の作成
	/// @param fileName モデルファイル名
	/// @param position 位置
	void Create(const std::string &fileName, const Vector3 &position);

private:
	Registry *registry_;								// レジストリ
	IndirectCommandManager *indirectCommandManager_;	// 間接コマンドマネージャー
	ModelManager *modelManager_;						// モデルマネージャー
	ObjectManager *objectManager_;						// オブジェクトマネージャー
};