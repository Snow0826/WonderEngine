#pragma once
#include <cstdint>

class Registry;
class ParticleManager;
struct Vector3;

/// @brief デスパーティクル
class DeathParticle {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param particleManager パーティクルマネージャー
	DeathParticle(Registry *registry, ParticleManager *particleManager) : registry_(registry), particleManager_(particleManager) {}

	/// @brief 初期化
	/// @param position 位置
	void Initialize(const Vector3 &position);

	/// @brief 更新
	void Update();

	bool IsDead() const;

private:
	Registry *registry_ = nullptr;
	ParticleManager *particleManager_ = nullptr;
	uint32_t entity_ = 0;
};