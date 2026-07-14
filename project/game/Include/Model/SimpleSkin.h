#pragma once

class Registry;
class ModelManager;
struct Vector3;

class SimpleSkin {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param modelManager モデルマネージャー
	SimpleSkin(Registry *registry, ModelManager *modelManager) : registry_(registry) , modelManager_(modelManager) {}

	/// @brief シンプルスキンの作成
	/// @param position 位置
	void Create(const Vector3 &position);

private:
	Registry *registry_ = nullptr;			// レジストリ
	ModelManager *modelManager_ = nullptr;	// モデルマネージャー
};