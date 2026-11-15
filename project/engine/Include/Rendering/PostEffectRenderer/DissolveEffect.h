#pragma once
#include "PostEffectRenderer.h"
#include <d3d12.h>

class TextureManager;

/// @brief ディゾルブエフェクト
class DissolveEffect : public PostEffectRenderer {
public:
	/// @brief コンストラクタ
	/// @param device デバイス
	/// @param world ワールド
	/// @param textureManager テクスチャマネージャー
	/// @param rootSignature ルートシグネチャ
	/// @param pipelineState パイプラインステート
	DissolveEffect(Device *device, World *world, TextureManager *textureManager, ID3D12RootSignature *rootSignature, ID3D12PipelineState *pipelineState)
		: PostEffectRenderer(device, world)
		, textureManager_(textureManager)
		, rootSignature_(rootSignature)
		, pipelineState_(pipelineState) {
	}

	/// @brief バインド
	void Bind() override;

private:
	TextureManager *textureManager_ = nullptr;	// テクスチャマネージャー
	ID3D12RootSignature *rootSignature_ = nullptr;	// ルートシグネチャ
	ID3D12PipelineState *pipelineState_ = nullptr;	// パイプラインステート
};