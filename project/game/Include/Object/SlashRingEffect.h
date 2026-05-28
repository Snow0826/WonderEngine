#pragma once
#include <cstdint>

class Registry;
class ParticleManager;

class SlashRingEffect {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param particleManager パーティクルマネージャー
	SlashRingEffect(Registry *registry, ParticleManager *particleManager)
		: registry_(registry), particleManager_(particleManager) {
	}

	/// @brief 初期化
	void Initialize();

	/// @brief 更新
	void Update();

private:
	Registry *registry_ = nullptr;					// レジストリ
	ParticleManager *particleManager_ = nullptr;	// パーティクルマネージャー
	uint32_t entity_ = 0;							// エンティティ
};