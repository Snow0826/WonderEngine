#pragma once
#include "PostEffectRenderer.h"
#include <d3d12.h>

/// @brief グレースケールエフェクト
class GrayscaleEffect : public PostEffectRenderer {
public:
	/// @brief コンストラクタ
	/// @param device デバイス
	/// @param world ワールド
	/// @param rootSignature ルートシグネチャ
	/// @param pipelineState パイプラインステート
	GrayscaleEffect(Device *device, World *world, ID3D12RootSignature *rootSignature, ID3D12PipelineState *pipelineState)
		: PostEffectRenderer(device, world)
		, rootSignature_(rootSignature)
		, pipelineState_(pipelineState) {
	}

	/// @brief バインド
	void Bind() override;

private:
	ID3D12RootSignature *rootSignature_ = nullptr;	// ルートシグネチャ
	ID3D12PipelineState *pipelineState_ = nullptr;	// パイプラインステート
};