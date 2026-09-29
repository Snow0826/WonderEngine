#pragma once
#include <cstdint>

class Registry;
class ModelManager;
class InstanceAllocator;
class FootprintManager;

/// @brief プレイヤー
class Player {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param modelManager モデルマネージャー
	/// @param instanceAllocator インスタンスアロケータ
	/// @param footprintManager フットプリントマネージャー
	Player(Registry *registry, ModelManager *modelManager, InstanceAllocator *instanceAllocator, FootprintManager *footprintManager) : registry_(registry), modelManager_(modelManager), instanceAllocator_(instanceAllocator), footprintManager_(footprintManager) {}

	/// @brief 初期化
	void Initialize();

	/// @brief 更新
	void Update();

	/// @brief 移動
	/// @param x x座標の移動量
	/// @param z z座標の移動量
	void Move(float x, float z);

private:
	Registry *registry_ = nullptr;						// レジストリ
	ModelManager *modelManager_ = nullptr;				// モデルマネージャー
	InstanceAllocator *instanceAllocator_ = nullptr;	// インスタンスアロケータ
	FootprintManager *footprintManager_ = nullptr;		// フットプリントマネージャー
	uint32_t entity_ = 0;								// プレイヤーのエンティティID
};