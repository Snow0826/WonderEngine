#pragma once

class Registry;
class ParticleManager;

/// @brief パーティクルオブジェクト
class ParticleObject {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param particleManager パーティクルマネージャー
	ParticleObject(Registry *registry, ParticleManager *particleManager) : registry_(registry) , particleManager_(particleManager) {}

	/// @brief パーティクルオブジェクトの作成
	void Create();

private:
	Registry *registry_;	// レジストリ
	ParticleManager *particleManager_;	// パーティクルマネージャー
};