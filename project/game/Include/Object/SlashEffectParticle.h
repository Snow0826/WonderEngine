#pragma once
#include <cstdint>

class Registry;
class ParticleManager;

/// @brief スラッシュエフェクトのパーティクル
class SlashEffectParticle {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param particleManager パーティクルマネージャー
	SlashEffectParticle(Registry *registry, ParticleManager *particleManager)
		: registry_(registry), particleManager_(particleManager) {
	}

	/// @brief 初期化
	void Initialize();

	/// @brief 更新
	void Update();

private:
	Registry *registry_;				// レジストリ
	ParticleManager *particleManager_;	// パーティクルマネージャー
	uint32_t entity_ = 0;				// エンティティ
};