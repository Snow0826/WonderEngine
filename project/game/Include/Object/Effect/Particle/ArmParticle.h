#pragma once
#include <string>

class Registry;
class ParticleManager;
struct Vector3;

/// @brief 腕パーティクル
class ArmParticle {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param particleManager パーティクルマネージャー
	ArmParticle(Registry *registry, ParticleManager *particleManager) : registry_(registry), particleManager_(particleManager) {}

	/// @brief 初期化
	/// @param entity エンティティ
	/// @param groupName パーティクルグループ名
	void Initialize(uint32_t entity, const std::string &groupName);

	/// @brief 更新
	void Update();

private:
	Registry *registry_ = nullptr;					// レジストリ
	ParticleManager *particleManager_ = nullptr;	// パーティクルマネージャー
	uint32_t particleEntity_ = 0;					// パーティクルエンティティ
	uint32_t armEntity_ = 0;						// 腕エンティティ
};