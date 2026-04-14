#pragma once
#include <string>

class Registry;
class IndirectCommandManager;
class ModelManager;
class ObjectManager;
struct Vector3;
struct Vector4;

/// @brief タイトル
class Title {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param indirectCommandManager 間接コマンドマネージャー
	/// @param modelManager モデルマネージャー
	/// @param objectManager オブジェクトマネージャー
	Title(Registry *registry, IndirectCommandManager *indirectCommandManager, ModelManager *modelManager, ObjectManager *objectManager)
		: registry_(registry)
		, indirectCommandManager_(indirectCommandManager)
		, modelManager_(modelManager)
		, objectManager_(objectManager) {}

	/// @brief 建物の作成
	/// @param fileName モデルファイル名
	/// @param rotate 回転
	/// @param translate 平行移動
	/// @param color 色
	void Create(const std::string &fileName, const Vector3 &rotate, const Vector3 &translate, const Vector4 &color);

private:
	Registry *registry_;								// レジストリ
	IndirectCommandManager *indirectCommandManager_;	// 間接コマンドマネージャー
	ModelManager *modelManager_;						// モデルマネージャー
	ObjectManager *objectManager_;						// オブジェクトマネージャー
};