#pragma once
#include <cstdint>

class Registry;
class IndirectCommandManager;
class ModelManager;
class ObjectManager;
class FootprintManager;
struct Vector3;
namespace Collision {
	struct Plane;
	struct AABB;
}

/// @brief 敵キャラクター
class Enemy {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param indirectCommandManager 間接コマンドマネージャー
	/// @param modelManager モデルマネージャー
	/// @param objectManager オブジェクトマネージャー
	/// @param footprintManager フットプリントマネージャー
	Enemy(Registry *registry, IndirectCommandManager *indirectCommandManager, ModelManager *modelManager, ObjectManager *objectManager, FootprintManager *footprintManager)
		: registry_(registry)
		, indirectCommandManager_(indirectCommandManager)
		, modelManager_(modelManager)
		, objectManager_(objectManager)
		, footprintManager_(footprintManager) {}

	/// @brief 初期化
	/// @param position 初期位置
	void Initialize(const Vector3 &position);

	/// @brief 更新
	void Update();

	/// @brief 衝突応答
	/// @param plane 衝突した平面
	void OnCollision(const Collision::Plane &plane);

	/// @brief 衝突応答
	/// @param aabb 衝突したAABB
	void OnCollision(const Collision::AABB &aabb);

	/// @brief 衝突応答
	void OnCollision();

	/// @brief 死亡処理
	void Dead();

	/// @brief プレイヤーエンティティの設定
	/// @param playerEntity プレイヤーエンティティ
	void SetPlayerEntity(uint32_t playerEntity) { playerEntity_ = playerEntity; }

	/// @brief 敵エンティティの取得
	/// @return 敵エンティティ
	uint32_t GetEnemyEntity() const { return enemyEntity_; }

	/// @brief 死亡判定
	/// @return 死亡しているかどうか
	bool IsDead() const;

private:
	Registry *registry_ = nullptr;								// レジストリ
	IndirectCommandManager *indirectCommandManager_ = nullptr;	// 間接コマンドマネージャー
	ModelManager *modelManager_ = nullptr;						// モデルマネージャー
	ObjectManager *objectManager_ = nullptr;					// オブジェクトマネージャー
	FootprintManager *footprintManager_ = nullptr;				// フットプリントマネージャー
	uint32_t enemyEntity_ = 0;									// 敵エンティティ
	uint32_t playerEntity_ = 0;									// プレイヤーエンティティ
};