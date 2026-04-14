#pragma once
#include "Vector3.h"

class Registry;
class ParticleManager;
class FootprintManager;

/// @brief 攻撃パーティクル
class AttackParticle {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param particleManager パーティクルマネージャー
	/// @param footprintManager フットプリントマネージャー
	AttackParticle(Registry *registry, ParticleManager *particleManager, FootprintManager *footprintManager)
		: registry_(registry), particleManager_(particleManager), footprintManager_(footprintManager) {}

	/// @brief 初期化
	/// @param entity エンティティ
	void Initialize(uint32_t entity);

	/// @brief 更新
	void Update();

	/// @brief 開始
	void Start();

	/// @brief 停止
	void Stop();

	/// @brief パーティクルエンティティの取得
	/// @return パーティクルエンティティ
	uint32_t GetParticleEntity() const { return particleEntity_; }

private:
	Registry *registry_ = nullptr;						// レジストリ
	ParticleManager *particleManager_ = nullptr;		// パーティクルマネージャー
	FootprintManager *footprintManager_ = nullptr;		// フットプリントマネージャー
	uint32_t particleEntity_ = 0;						// パーティクルエンティティ
	uint32_t playerEntity_ = 0;							// プレイヤーエンティティ
	static inline constexpr Vector3 kFieldAreaSize = { 10.0f, 10.0f, 10.0f };	// フィールドエリアのサイズ
};