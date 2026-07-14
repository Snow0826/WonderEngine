#pragma once
#include <string>

class Registry;
class ModelManager;
struct Vector3;

class Human {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param modelManager モデルマネージャー
	Human(Registry *registry, ModelManager *modelManager) : registry_(registry) , modelManager_(modelManager) {}

	/// @brief 人間の作成
	/// @param fileName モデルファイル名
	/// @param position 位置
	void Create(const std::string &fileName, const Vector3 &position);

private:
	Registry *registry_;			// レジストリ
	ModelManager *modelManager_;	// モデルマネージャー
};