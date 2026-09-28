#pragma once
#include "Matrix4x4.h"
#include <cstdint>

/// @brief ソフトボディデータ(GPU)
struct SoftBodyDataForGPU final {
	Matrix4x4 worldMatrix = MakeIdentity4x4();	// ワールド行列
	float impactVelocity = 0.0f;	// インパクト速度
	float hitTime = 0.0f;	// ヒット時間
    
	float impactScale = 0.08f;
	float maxImpact = 1.0f;

	float damping = 5.0f;
	float frequency = 18.0f;

	float squashAmount = 0.35f;
	float expandAmount = 0.8f;

	float lowerExpandWeight = 1.0f;
	float upperExpandWeight = 0.2f;

	float padding[2];
};

/// @brief ソフトボディデータ(CPU)
struct SoftBodyDataForCPU final {
	float impactVelocity = 1.0f;
	float hitTime = 1.0f;

	float impactScale = 0.08f;
	float maxImpact = 1.0f;

	float damping = 5.0f;
	float frequency = 18.0f;

	float squashAmount = 0.35f;
	float expandAmount = 0.8f;

	float lowerExpandWeight = 1.0f;
	float upperExpandWeight = 0.2f;
	
	bool wasGrounded = false;
	bool isGrounded = false;
};

class Registry;

/// @brief ソフトボディインスペクター
class SoftBodyInspector {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	SoftBodyInspector(Registry *registry) : registry_(registry) {}

	/// @brief ソフトボディインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;
};