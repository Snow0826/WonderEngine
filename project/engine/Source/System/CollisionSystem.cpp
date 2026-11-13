#include "CollisionSystem.h"

void CollisionSystem::Update() {
	// 衝突イベントをクリアする
	collisionEvents_.clear();

	// 同じタイプのコライダー同士の衝突をチェックする
	CheckSameType<Collision::Sphere>();
	CheckSameType<Collision::AABB>();
	CheckSameType<Collision::Capsule>();

	// 異なるタイプのコライダー同士の衝突をチェックする
	CheckPair<Collision::Sphere, Collision::AABB>();
	CheckPair<Collision::Sphere, Collision::OBB>();
	CheckPair<Collision::Sphere, Collision::Plane>();
	CheckPair<Collision::Capsule, Collision::AABB>();
	CheckPair<Collision::Capsule, Collision::OBB>();
	CheckPair<Collision::Capsule, Collision::Plane>();
}