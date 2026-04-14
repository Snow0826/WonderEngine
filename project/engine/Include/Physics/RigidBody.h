#pragma once
#include "Vector3.h"
#include "Matrix3x3.h"

/// @brief 剛体
struct RigidBody final {
	Vector3 force;				// 力
	Vector3 velocity;			// 速度
	Vector3 angularMomentum;	// 角運動量
	Vector3 angularVelocity;	// 角速度
	float mass = 1.0f;			// 質量
	float radius = 0.0f;		// 半径
};

class Registry;

/// @brief 物理システム
class PhysicalSystem {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	PhysicalSystem(Registry *registry) : registry_(registry) {}

	/// @brief 更新
	/// @param deltaTime デルタタイム
	void Update(float deltaTime);

	/// @brief 反射
	/// @param deltaTime デルタタイム
	/// @param entityA エンティティA
	/// @param entityB エンティティB
	void Reflect(float deltaTime, uint32_t entityA, uint32_t entityB);

	/// @brief 力の反射
	/// @param deltaTime デルタタイム
	/// @param entityA エンティティA
	/// @param entityB エンティティB
	void ReflectImpulse(float deltaTime, uint32_t entityA, uint32_t entityB);

private:
	Registry *registry_ = nullptr;	// レジストリ

	/// @brief 球の慣性テンソルの作成
	/// @param mass 質量
	/// @param radius 半径
	/// @return 球の慣性テンソル
	Matrix3x3 MakeSphereInertiaTensor(float mass, float radius);
};

/// @brief 剛体インスペクター
class RigidBodyInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	RigidBodyInspector(Registry *registry) : registry_(registry) {}
	
	/// @brief 剛体インスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;
};