#include "CollisionSystem.h"

void CollisionSystem::Update() {
	collisionEvents_.clear();
	CheckSameType<Collision::Sphere>();
	CheckSameType<Collision::AABB>();
	CheckSameType<Collision::Capsule>();
	CheckPair<Collision::Sphere, Collision::AABB>();
	CheckPair<Collision::Sphere, Collision::OBB>();
	CheckPair<Collision::Sphere, Collision::Plane>();
	CheckPair<Collision::Capsule, Collision::AABB>();
	CheckPair<Collision::Capsule, Collision::OBB>();
	CheckPair<Collision::Capsule, Collision::Plane>();
}