#pragma once
#include "EntityComponentSystem.h"
#include "Collision.h"

/// @brief 衝突イベント
struct CollisionEvent final {
	uint32_t entityA;	// エンティティA
	uint32_t entityB;	// エンティティB
};

/// @brief 衝突なしタグ
struct NoCollision final {};

class Registry;

/// @brief 衝突システム
class CollisionSystem {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	CollisionSystem(Registry *registry) : registry_(registry) {}

	/// @brief 更新
	void Update();

private:
	Registry *registry_ = nullptr;
	std::vector<CollisionEvent> collisionEvents_;

	/// @brief 同じタイプのコライダー同士の衝突をチェックする
	/// @tparam ColliderType コライダーの型
	template <typename ColliderType>
	void CheckSameType() {
		const std::vector<uint32_t> &entities = registry_->GetComponentEntities<ColliderType>();
		for (size_t i = 0; i < entities.size(); i++) {
			for (size_t j = i + 1; j < entities.size(); j++) {
				// どちらかがNoCollisionを持っている場合は衝突をチェックしない
				if (registry_->HasComponent<NoCollision>(entities[i]) || registry_->HasComponent<NoCollision>(entities[j])) {
					continue;
				}
				auto *colliderA = registry_->GetComponent<ColliderType>(entities[i]);
				auto *colliderB = registry_->GetComponent<ColliderType>(entities[j]);
				if (IsCollision(*colliderA, *colliderB)) {
					collisionEvents_.emplace_back(CollisionEvent{ .entityA = entities[i], .entityB = entities[j] });
				}
			}
		}
	}

	/// @brief 異なるタイプのコライダー同士の衝突をチェックする
	/// @tparam ColliderTypeA コライダーAの型
	/// @tparam ColliderTypeB コライダーBの型
	template <typename ColliderTypeA, typename ColliderTypeB>
	void CheckPair() {
		const std::vector<uint32_t> &entitiesA = registry_->GetComponentEntities<ColliderTypeA>();
		const std::vector<uint32_t> &entitiesB = registry_->GetComponentEntities<ColliderTypeB>();
		for (uint32_t entityA : entitiesA) {
			for (uint32_t entityB : entitiesB) {
				// どちらかがNoCollisionを持っている場合は衝突をチェックしない
				if (registry_->HasComponent<NoCollision>(entityA) || registry_->HasComponent<NoCollision>(entityB)) {
					continue;
				}
				auto *colliderA = registry_->GetComponent<ColliderTypeA>(entityA);
				auto *colliderB = registry_->GetComponent<ColliderTypeB>(entityB);
				if (IsCollision(*colliderA, *colliderB)) {
					collisionEvents_.emplace_back(CollisionEvent{ .entityA = entityA, .entityB = entityB });
				}
			}
		}
	}
};