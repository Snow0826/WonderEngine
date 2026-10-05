#pragma once
#include <Vector2.h>

class Registry;
class InstanceAllocator;
class FootprintManager;

/// @brief プレイヤー
class Player {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param instanceAllocator インスタンスアロケータ
	/// @param footprintManager フットプリントマネージャー
	Player(Registry *registry, InstanceAllocator *instanceAllocator, FootprintManager *footprintManager) : registry_(registry), instanceAllocator_(instanceAllocator), footprintManager_(footprintManager) {}

	/// @brief 初期化
	void Initialize();

	/// @brief 更新
	/// @param deltaTime デルタタイム
	void Update(float deltaTime);

	/// @brief 移動
	/// @param x x座標の移動量
	/// @param z z座標の移動量
	void Move(float x, float z) { move_.x = x; move_.y = z; }

	/// @brief 移動量のクリア
	void ClearMove() { move_ = { 0.0f, 0.0f }; }

	/// @brief カメラエンティティの設定
	/// @param cameraEntity カメラエンティティ
	void SetCameraEntity(uint32_t cameraEntity) { cameraEntity_ = cameraEntity; }

	/// @brief エンティティIDの取得
	/// @return エンティティID
	uint32_t GetEntity() const { return entity_; }

private:
	Registry *registry_ = nullptr;						// レジストリ
	InstanceAllocator *instanceAllocator_ = nullptr;	// インスタンスアロケータ	
	FootprintManager *footprintManager_ = nullptr;		// フットプリントマネージャー
	uint32_t entity_ = 0;								// プレイヤーのエンティティID
	uint32_t cameraEntity_ = 0;							// カメラのエンティティID
	Vector2 move_ = { 0.0f, 0.0f };						// 移動方向
};