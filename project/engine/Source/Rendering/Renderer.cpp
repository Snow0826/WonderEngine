#define NOMINMAX
#include "Renderer.h"
#include "DebugRenderer.h"
#include "Resource.h"
#include "PipelineState.h"
#include "Texture.h"
#include "SkinCluster.h"
#include "Model.h"
#include "Sprite.h"
#include "Particle.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "Skybox.h"
#include "FootprintMap.h"
#include "EntityComponentSystem.h"
#include "SelectionContext.h"
#include "Device.h"
#include "World.h"
#include "ConstantBuffer.h"
#include "Footprint.h"
#include "ImGuiManager.h"
#include "Logger.h"
#include "StringConverter.h"
#include <pix3.h>

using namespace StringConverter;

namespace {
	std::array<uint32_t, static_cast<uint32_t>(MeshType::kCountOfMeshType)> pixColors = {
		PIX_COLOR(255, 0, 0),	// MeshType::kModel
		PIX_COLOR(255, 255, 0),	// MeshType::kPlane
		PIX_COLOR(255, 0, 255),	// MeshType::kBox
		PIX_COLOR(0, 255, 0),	// MeshType::kRing
		PIX_COLOR(0, 255, 255)	// MeshType::kCylinder
	};

	// メッシュタイプ名リスト
	std::array<std::string, static_cast<uint32_t>(MeshType::kCountOfMeshType)> meshTypeNames = {
		"Model",
		"Plane",
		"Box",
		"Ring",
		"Cylinder",
	};

	// ブレンドモード名リスト
	std::array<std::string, static_cast<uint32_t>(BlendMode::kCountOfBlendMode)> blendModeNames = {
		"None",
		"Normal",
		"Additive",
		"Subtractive",
		"Multiplicative",
		"Screen"
	};

	constexpr uint32_t kMainCameraIndex = 1;	// メインカメラのインデックス
	constexpr uint32_t kDebugCameraIndex = 2;	// デバッグカメラのインデックス
}

Renderer::Renderer(Device *device)
	: device_(device)
	, gpuCbvSrvUavDescriptorHeap_(device->GetGpuCbvSrvUavDescriptorHeap())
	, cpuCbvSrvUavDescriptorHeap_(device->GetCpuCbvSrvUavDescriptorHeap())
	, commandList_(device->GetCommandList())
	, object3dRootSignature_(device->GetObject3dRootSignature())
	, ringObject3dRootSignature_(device->GetRingObject3dRootSignature())
	, particleRootSignature_(device->GetParticleRootSignature())
	, ringParticleRootSignature_(device->GetRingParticleRootSignature())
	, lineRootSignature_(device->GetLineRootSignature())
	, skyboxRootSignature_(device->GetSkyboxRootSignature())
	, fullscreenRootSignature_(device->GetFullscreenRootSignature())
	, grayscaleRootSignature_(device->GetGrayscaleRootSignature())
	, vignetteRootSignature_(device->GetVignetteRootSignature())
	, boxFilterRootSignature_(device->GetBoxFilterRootSignature())
	, gaussianFilterRootSignature_(device->GetGaussianFilterRootSignature())
	, luminanceBasedOutlineRootSignature_(device->GetLuminanceBasedOutlineRootSignature())
	, depthBasedOutlineRootSignature_(device->GetDepthBasedOutlineRootSignature())
	, radialBlurRootSignature_(device->GetRadialBlurRootSignature())
	, dissolveRootSignature_(device->GetDissolveRootSignature())
	, noiseRootSignature_(device->GetNoiseRootSignature())
	, skinningRootSignature_(device->GetSkinningRootSignature())
	, initializeParticleRootSignature_(device->GetInitializeParticleRootSignature())
	, emitParticleRootSignature_(device->GetEmitParticleRootSignature())
	, updateParticleRootSignature_(device->GetUpdateParticleRootSignature())
	, createCylinderAABBRootSignature_(device->GetCreateCylinderAABBRootSignature())
	, createModelAABBRootSignature_(device->GetCreateModelAABBRootSignature())
	, depthStencilCopyRootSignature_(device->GetDepthStencilCopyRootSignature())
	, generateHiZMipMapRootSignature_(device->GetGenerateHiZMipMapRootSignature())
	, clearMeshCommandStatesRootSignature_(device->GetClearMeshCommandStatesRootSignature())
	, occlusionCullingRootSignature_(device->GetOcclusionCullingRootSignature())
	, prefixSumRootSignature_(device->GetPrefixSumRootSignature())
	, setInstanceCountRootSignature_(device->GetSetInstanceCountRootSignature())
	, setInstanceIndexRootSignature_(device->GetSetInstanceIndexRootSignature())
	, footprintRootSignature_(device->GetFootprintRootSignature())
	, footprintMapRootSignature_(device->GetFootprintMapRootSignature()) {
}

void Renderer::Initialize(std::ofstream &logStream) {
	std::array<D3D12_BLEND_DESC, static_cast<uint32_t>(BlendMode::kCountOfBlendMode)> blendDescList{};

	// kBlendModeNoneのBlendDescの設定
	uint32_t blendModeIndex = static_cast<uint32_t>(BlendMode::kBlendModeNone);
	blendDescList[blendModeIndex].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// 全ての色要素を書き込む

	// kBlendModeNormalのBlendDescの設定
	blendModeIndex = static_cast<uint32_t>(BlendMode::kBlendModeNormal);
	blendDescList[blendModeIndex].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// 全ての色要素を書き込む
	blendDescList[blendModeIndex].RenderTarget[0].BlendEnable = true;									// ブレンドを有効化
	blendDescList[blendModeIndex].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;						// ソースのアルファ値を使用
	blendDescList[blendModeIndex].RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;				// デスティネーションのアルファ値を使用
	blendDescList[blendModeIndex].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;							// ブレンド演算は加算
	blendDescList[blendModeIndex].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;						// ソースのアルファ値を使用
	blendDescList[blendModeIndex].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;					// デスティネーションのアルファ値は使用しない
	blendDescList[blendModeIndex].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;					// アルファブレンド演算は加算

	// kBlendModeAdditiveのBlendDescの設定
	blendModeIndex = static_cast<uint32_t>(BlendMode::kBlendModeAdditive);
	blendDescList[blendModeIndex].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// 全ての色要素を書き込む
	blendDescList[blendModeIndex].RenderTarget[0].BlendEnable = true;									// ブレンドを有効化
	blendDescList[blendModeIndex].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;						// ソースのアルファ値を使用
	blendDescList[blendModeIndex].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;							// デスティネーションのアルファ値を使用
	blendDescList[blendModeIndex].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;							// ブレンド演算は加算
	blendDescList[blendModeIndex].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;						// ソースのアルファ値を使用
	blendDescList[blendModeIndex].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;					// デスティネーションのアルファ値は使用しない
	blendDescList[blendModeIndex].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;					// アルファブレンド演算は加算

	// kBlendModeSubtractiveのBlendDescの設定
	blendModeIndex = static_cast<uint32_t>(BlendMode::kBlendModeSubtractive);
	blendDescList[blendModeIndex].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// 全ての色要素を書き込む
	blendDescList[blendModeIndex].RenderTarget[0].BlendEnable = true;									// ブレンドを有効化
	blendDescList[blendModeIndex].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;						// ソースのアルファ値を使用
	blendDescList[blendModeIndex].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;							// デスティネーションのアルファ値を使用
	blendDescList[blendModeIndex].RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;				// ブレンド演算は逆減算
	blendDescList[blendModeIndex].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;						// ソースのアルファ値を使用
	blendDescList[blendModeIndex].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;					// デスティネーションのアルファ値は使用しない
	blendDescList[blendModeIndex].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;					// アルファブレンド演算は加算

	// kBlendModeMultiplicativeのBlendDescの設定
	blendModeIndex = static_cast<uint32_t>(BlendMode::kBlendModeMultiplicative);
	blendDescList[blendModeIndex].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// 全ての色要素を書き込む
	blendDescList[blendModeIndex].RenderTarget[0].BlendEnable = true;									// ブレンドを有効化
	blendDescList[blendModeIndex].RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;							// ソースのアルファ値は使用しない
	blendDescList[blendModeIndex].RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;					// デスティネーションのアルファ値を使用
	blendDescList[blendModeIndex].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;							// ブレンド演算は加算
	blendDescList[blendModeIndex].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;						// ソースのアルファ値を使用
	blendDescList[blendModeIndex].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;					// デスティネーションのアルファ値は使用しない
	blendDescList[blendModeIndex].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;					// アルファブレンド演算は加算

	// kBlendModeScreenのBlendDescの設定
	blendModeIndex = static_cast<uint32_t>(BlendMode::kBlendModeScreen);
	blendDescList[blendModeIndex].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// 全ての色要素を書き込む
	blendDescList[blendModeIndex].RenderTarget[0].BlendEnable = true;									// ブレンドを有効化
	blendDescList[blendModeIndex].RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;				// ソースのアルファ値を使用
	blendDescList[blendModeIndex].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;							// デスティネーションのアルファ値を使用
	blendDescList[blendModeIndex].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;							// ブレンド演算は加算
	blendDescList[blendModeIndex].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;						// ソースのアルファ値を使用
	blendDescList[blendModeIndex].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;					// デスティネーションのアルファ値は使用しない
	blendDescList[blendModeIndex].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;					// アルファブレンド演算は加算

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC noCullingRasterizerDesc{};
	noCullingRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;	// カリングしない
	noCullingRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;	// 塗りつぶしモード

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC backCullingRasterizerDesc{};
	backCullingRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;	// 裏面をカリング
	backCullingRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;	// 塗りつぶしモード

	// 深度バッファに書き込むDepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC writeLessDepthStencilDesc{};
	writeLessDepthStencilDesc.DepthEnable = true;							// 深度バッファを使用
	writeLessDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	// 深度バッファに書き込む
	writeLessDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;		// 深度比較関数

	// 深度バッファに書き込むDepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC writeLessEqualDepthStencilDesc{};
	writeLessEqualDepthStencilDesc.DepthEnable = true;								// 深度バッファを使用
	writeLessEqualDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;		// 深度バッファに書き込む
	writeLessEqualDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;	// 深度比較関数

	// 深度バッファに書き込まないDepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC noWriteLessEqualDepthStencilDesc{};
	noWriteLessEqualDepthStencilDesc.DepthEnable = true;							// 深度バッファを使用
	noWriteLessEqualDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	// 深度バッファに書き込まない
	noWriteLessEqualDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;	// 深度比較関数

	// dxcCompilerの初期化
	IDxcUtils *dxcUtils = nullptr;
	IDxcCompiler3 *dxcCompiler = nullptr;
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	// includeのためのインターフェースを取得
	IDxcIncludeHandler *includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	// Object3dのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> object3dVSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Object3d.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(object3dVSBlob);
	Microsoft::WRL::ComPtr<IDxcBlob> object3dPSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Object3d.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(object3dPSBlob);

	// Particleのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> particleVSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Particle.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(particleVSBlob);
	Microsoft::WRL::ComPtr<IDxcBlob> particlePSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Particle.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(particlePSBlob);

	// Lineのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> lineVSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Line.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(lineVSBlob);
	Microsoft::WRL::ComPtr<IDxcBlob> linePSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Line.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(linePSBlob);

	// Skyboxのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> skyboxVSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Skybox.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(skyboxVSBlob);
	Microsoft::WRL::ComPtr<IDxcBlob> skyboxPSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Skybox.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(skyboxPSBlob);

	// Fullscreenのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> fullscreenVSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Fullscreen.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(fullscreenVSBlob);
	Microsoft::WRL::ComPtr<IDxcBlob> fullscreenPSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Fullscreen.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(fullscreenPSBlob);

	// Grayscaleのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> grayscalePSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Grayscale.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(grayscalePSBlob);

	// Vignetteのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> vignettePSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Vignette.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vignettePSBlob);

	// BoxFilterのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> boxFilterPSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/BoxFilter.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(boxFilterPSBlob);

	// GaussianFilterのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> gaussianFilterPSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/GaussianFilter.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(gaussianFilterPSBlob);

	// LuminanceBasedOutlineのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> luminanceBasedOutlinePSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/LuminanceBasedOutline.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(luminanceBasedOutlinePSBlob);

	// DepthBasedOutlineのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> depthBasedOutlinePSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/DepthBasedOutline.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(depthBasedOutlinePSBlob);

	// RadialBlurのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> radialBlurPSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/RadialBlur.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(radialBlurPSBlob);

	// Dissolveのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> dissolvePSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Dissolve.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(dissolvePSBlob);

	// Noiseのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> noisePSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Noise.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(noisePSBlob);

	// 枝のシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> branchVSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Branch.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(branchVSBlob);

	// スキニングのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> skinningCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Skinning.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(skinningCSBlob);

	// パーティクル初期化のシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> initializeParticleCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/InitializeParticle.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(initializeParticleCSBlob);

	// パーティクル発生のシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> emitParticleCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/EmitParticle.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(emitParticleCSBlob);

	// パーティクル更新のシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> updateParticleCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/UpdateParticle.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(updateParticleCSBlob);

	// 円柱のAABB生成のシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> createCylinderAABBCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/CreateCylinderAABB.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(createCylinderAABBCSBlob);

	// モデルのAABB生成のシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> createModelAABBCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/CreateModelAABB.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(createModelAABBCSBlob);

	// 深度ステンシルテクスチャコピーのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> depthStencilCopyCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/DepthStencilCopy.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(depthStencilCopyCSBlob);

	// HiZミップマップ生成のシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> generateHiZMipMapCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/GenerateHiZMipMap.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(generateHiZMipMapCSBlob);

	// メッシュコマンドステートのクリア用シェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> clearMeshCommandStatesCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/ClearMeshCommandStates.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(clearMeshCommandStatesCSBlob);

	// オクルージョンカリングのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> occlusionCullingCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/OcclusionCulling.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(occlusionCullingCSBlob);

	// 累積和計算用シェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> prefixSumCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/PrefixSum.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(prefixSumCSBlob);

	// インスタンス数の反映用シェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> setInstanceCountCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/SetInstanceCount.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(setInstanceCountCSBlob);

	// インスタンスインデックスの反映用シェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> setInstanceIndexCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/SetInstanceIndex.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(setInstanceIndexCSBlob);

	// フットプリントのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> footprintCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/FootprintStamp.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(footprintCSBlob);

	// フットプリントマップのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> footprintMapCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/FootprintMap.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(footprintMapCSBlob);

	// Mesh用パイプラインステートの生成
	for (size_t i = 0; i < static_cast<size_t>(MeshType::kCountOfMeshType); i++) {
		for (size_t j = 0; j < static_cast<size_t>(BlendMode::kCountOfBlendMode); j++) {
			if (static_cast<MeshType>(i) == MeshType::kRing) {
				meshPipelineState_[i][j] = PipelineState()
					.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)	// 頂点座標
					.AddInput("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)			// テクスチャ座標
					.AddInput("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)		// 法線ベクトル
					.AddInput("SECTION", 0, DXGI_FORMAT_R32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT)				// セクション番号
					.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
					.SetBlendState(blendDescList[j])															// BlendState
					.SetRasterizer(backCullingRasterizerDesc)													// RasterizerState
					.SetDepthState(writeLessEqualDepthStencilDesc)												// DepthStencilState
					.SetVertexShader(object3dVSBlob->GetBufferPointer(), object3dVSBlob->GetBufferSize())		// 頂点シェーダー
					.SetPixelShader(object3dPSBlob->GetBufferPointer(), object3dPSBlob->GetBufferSize())		// ピクセルシェーダー
					.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
					.Create(device_->GetDevice(), ringObject3dRootSignature_);
			} else if (static_cast<MeshType>(i) == MeshType::kCylinder) {
				meshPipelineState_[i][j] = PipelineState()
					.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)	// 頂点座標
					.AddInput("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)			// テクスチャ座標
					.AddInput("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)		// 法線ベクトル
					.AddInput("SECTION", 0, DXGI_FORMAT_R32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT)				// セクション番号
					.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
					.SetBlendState(blendDescList[j])															// BlendState
					.SetRasterizer(noCullingRasterizerDesc)														// RasterizerState
					.SetDepthState(writeLessEqualDepthStencilDesc)												// DepthStencilState
					.SetVertexShader(branchVSBlob->GetBufferPointer(), branchVSBlob->GetBufferSize())			// 頂点シェーダー
					.SetPixelShader(object3dPSBlob->GetBufferPointer(), object3dPSBlob->GetBufferSize())		// ピクセルシェーダー
					.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
					.Create(device_->GetDevice(), object3dRootSignature_);
			} else {
				meshPipelineState_[i][j] = PipelineState()
					.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)	// 頂点座標
					.AddInput("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)			// テクスチャ座標
					.AddInput("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)		// 法線ベクトル
					.AddInput("SECTION", 0, DXGI_FORMAT_R32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT)				// セクション番号
					.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
					.SetBlendState(blendDescList[j])															// BlendState
					.SetRasterizer(backCullingRasterizerDesc)													// RasterizerState
					.SetDepthState(writeLessEqualDepthStencilDesc)												// DepthStencilState
					.SetVertexShader(object3dVSBlob->GetBufferPointer(), object3dVSBlob->GetBufferSize())		// 頂点シェーダー
					.SetPixelShader(object3dPSBlob->GetBufferPointer(), object3dPSBlob->GetBufferSize())		// ピクセルシェーダー
					.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
					.Create(device_->GetDevice(), object3dRootSignature_);
			}
			const std::string logMessage = "Create MeshPipelineState : " + blendModeNames[j] + " " + meshTypeNames[i] + "\n";
			Logger::Log(logStream, logMessage);
			meshPipelineState_[i][j]->SetName(ConvertString(blendModeNames[j] + "Blend" + meshTypeNames[i] + "PipelineState").c_str());
		}
	}

	// MeshParticle用パイプラインステートの生成
	for (size_t i = 0; i < static_cast<size_t>(MeshType::kCountOfMeshType); i++) {
		for (size_t j = 0; j < static_cast<size_t>(BlendMode::kCountOfBlendMode); j++) {
			if (static_cast<MeshType>(i) == MeshType::kRing) {
				meshParticlePipelineState_[i][j] = PipelineState()
					.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)	// 頂点座標
					.AddInput("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)			// テクスチャ座標
					.AddInput("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)		// 法線ベクトル
					.AddInput("SECTION", 0, DXGI_FORMAT_R32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT)				// セクション番号
					.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
					.SetBlendState(blendDescList[j])															// BlendState
					.SetRasterizer(backCullingRasterizerDesc)													// RasterizerState
					.SetDepthState(noWriteLessEqualDepthStencilDesc)											// DepthStencilState
					.SetVertexShader(particleVSBlob->GetBufferPointer(), particleVSBlob->GetBufferSize())		// 頂点シェーダー
					.SetPixelShader(particlePSBlob->GetBufferPointer(), particlePSBlob->GetBufferSize())		// ピクセルシェーダー
					.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
					.Create(device_->GetDevice(), ringParticleRootSignature_);
			} else if (static_cast<MeshType>(i) == MeshType::kCylinder) {
				meshParticlePipelineState_[i][j] = PipelineState()
					.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)	// 頂点座標
					.AddInput("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)			// テクスチャ座標
					.AddInput("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)		// 法線ベクトル
					.AddInput("SECTION", 0, DXGI_FORMAT_R32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT)				// セクション番号
					.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
					.SetBlendState(blendDescList[j])															// BlendState
					.SetRasterizer(noCullingRasterizerDesc)														// RasterizerState
					.SetDepthState(noWriteLessEqualDepthStencilDesc)											// DepthStencilState
					.SetVertexShader(particleVSBlob->GetBufferPointer(), particleVSBlob->GetBufferSize())		// 頂点シェーダー
					.SetPixelShader(particlePSBlob->GetBufferPointer(), particlePSBlob->GetBufferSize())		// ピクセルシェーダー
					.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
					.Create(device_->GetDevice(), particleRootSignature_);
			} else {
				meshParticlePipelineState_[i][j] = PipelineState()
					.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)	// 頂点座標
					.AddInput("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)			// テクスチャ座標
					.AddInput("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)		// 法線ベクトル
					.AddInput("SECTION", 0, DXGI_FORMAT_R32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT)				// セクション番号
					.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
					.SetBlendState(blendDescList[j])															// BlendState
					.SetRasterizer(backCullingRasterizerDesc)													// RasterizerState
					.SetDepthState(noWriteLessEqualDepthStencilDesc)											// DepthStencilState
					.SetVertexShader(particleVSBlob->GetBufferPointer(), particleVSBlob->GetBufferSize())		// 頂点シェーダー
					.SetPixelShader(particlePSBlob->GetBufferPointer(), particlePSBlob->GetBufferSize())		// ピクセルシェーダー
					.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
					.Create(device_->GetDevice(), particleRootSignature_);
			}
			const std::string logMessage = "Create ParticlePipelineState : " + blendModeNames[j] + " " + meshTypeNames[i] + "\n";
			Logger::Log(logStream, logMessage);
			meshParticlePipelineState_[i][j]->SetName(ConvertString(blendModeNames[j] + "Blend" + meshTypeNames[i] + "ParticlePipelineState").c_str());
		}
	}

	// 各ブレンドモードのSprite用パイプラインステートの生成
	for (uint32_t i = 0; i < static_cast<uint32_t>(BlendMode::kCountOfBlendMode); i++) {
		spritePipelineState_[i] = PipelineState()
			.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)	// 頂点座標
			.AddInput("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)			// テクスチャ座標
			.AddInput("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)		// 法線ベクトル
			.AddInput("SECTION", 0, DXGI_FORMAT_R32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT)				// セクション番号
			.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
			.SetBlendState(blendDescList[i])															// BlendState
			.SetRasterizer(noCullingRasterizerDesc)														// RasterizerState
			.SetDepthState(noWriteLessEqualDepthStencilDesc)											// DepthStencilState
			.SetVertexShader(object3dVSBlob->GetBufferPointer(), object3dVSBlob->GetBufferSize())		// 頂点シェーダー
			.SetPixelShader(object3dPSBlob->GetBufferPointer(), object3dPSBlob->GetBufferSize())		// ピクセルシェーダー
			.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
			.Create(device_->GetDevice(), object3dRootSignature_);
		const std::string logMessage = "Create SpritePipelineState : " + blendModeNames[i] + "\n";
		Logger::Log(logStream, logMessage);
		spritePipelineState_[i]->SetName(ConvertString(blendModeNames[i] + "BlendSpritePipelineState").c_str());
	}

	// Line用パイプラインステートの生成
	linePipelineState_ = PipelineState()
		.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)							// RTVのフォーマット
		.SetBlendState(blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNone)])	// BlendState
		.SetRasterizer(noCullingRasterizerDesc)											// RasterizerState
		.SetDepthState(noWriteLessEqualDepthStencilDesc)								// DepthStencilState
		.SetVertexShader(lineVSBlob->GetBufferPointer(), lineVSBlob->GetBufferSize())	// 頂点シェーダー
		.SetPixelShader(linePSBlob->GetBufferPointer(), linePSBlob->GetBufferSize())	// ピクセルシェーダー
		.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE)					// プリミティブトポロジー
		.Create(device_->GetDevice(), lineRootSignature_);
	Logger::Log(logStream, "Create LinePipelineState\n");
	linePipelineState_->SetName(L"LinePipelineState");

	// Skybox用パイプラインステートの生成
	skyboxPipelineState_ = PipelineState()
		.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT)	// 頂点座標
		.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
		.SetBlendState(blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNone)])				// BlendState
		.SetRasterizer(noCullingRasterizerDesc)														// RasterizerState
		.SetDepthState(noWriteLessEqualDepthStencilDesc)											// DepthStencilState
		.SetVertexShader(skyboxVSBlob->GetBufferPointer(), skyboxVSBlob->GetBufferSize())			// 頂点シェーダー
		.SetPixelShader(skyboxPSBlob->GetBufferPointer(), skyboxPSBlob->GetBufferSize())			// ピクセルシェーダー
		.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
		.Create(device_->GetDevice(), skyboxRootSignature_);
	Logger::Log(logStream, "Create SkyboxPipelineState\n");
	skyboxPipelineState_->SetName(L"SkyboxPipelineState");

	// Fullscreen用パイプラインステートの生成
	fullscreenPipelineState_ = PipelineState()
		.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
		.SetBlendState(blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNone)])				// BlendState
		.SetRasterizer(noCullingRasterizerDesc)														// RasterizerState
		.SetDepthState({ .DepthEnable = false })													// DepthStencilState
		.SetVertexShader(fullscreenVSBlob->GetBufferPointer(), fullscreenVSBlob->GetBufferSize())	// 頂点シェーダー
		.SetPixelShader(fullscreenPSBlob->GetBufferPointer(), fullscreenPSBlob->GetBufferSize())	// ピクセルシェーダー
		.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
		.Create(device_->GetDevice(), fullscreenRootSignature_);
	Logger::Log(logStream, "Create FullscreenPipelineState\n");
	fullscreenPipelineState_->SetName(L"FullscreenPipelineState");

	// Grayscale用パイプラインステートの生成
	grayscalePipelineState_ = PipelineState()
		.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
		.SetBlendState(blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNone)])				// BlendState
		.SetRasterizer(noCullingRasterizerDesc)														// RasterizerState
		.SetDepthState({ .DepthEnable = false })													// DepthStencilState
		.SetVertexShader(fullscreenVSBlob->GetBufferPointer(), fullscreenVSBlob->GetBufferSize())	// 頂点シェーダー
		.SetPixelShader(grayscalePSBlob->GetBufferPointer(), grayscalePSBlob->GetBufferSize())		// ピクセルシェーダー
		.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
		.Create(device_->GetDevice(), grayscaleRootSignature_);
	Logger::Log(logStream, "Create GrayscalePipelineState\n");
	grayscalePipelineState_->SetName(L"GrayscalePipelineState");

	// Vignette用パイプラインステートの生成
	vignettePipelineState_ = PipelineState()
		.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
		.SetBlendState(blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNone)])				// BlendState
		.SetRasterizer(noCullingRasterizerDesc)														// RasterizerState
		.SetDepthState({ .DepthEnable = false })													// DepthStencilState
		.SetVertexShader(fullscreenVSBlob->GetBufferPointer(), fullscreenVSBlob->GetBufferSize())	// 頂点シェーダー
		.SetPixelShader(vignettePSBlob->GetBufferPointer(), vignettePSBlob->GetBufferSize())		// ピクセルシェーダー
		.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
		.Create(device_->GetDevice(), vignetteRootSignature_);
	Logger::Log(logStream, "Create VignettePipelineState\n");
	vignettePipelineState_->SetName(L"VignettePipelineState");

	// BoxFilter用パイプラインステートの生成
	boxFilterPipelineState_ = PipelineState()
		.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
		.SetBlendState(blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNone)])				// BlendState
		.SetRasterizer(noCullingRasterizerDesc)														// RasterizerState
		.SetDepthState({ .DepthEnable = false })													// DepthStencilState
		.SetVertexShader(fullscreenVSBlob->GetBufferPointer(), fullscreenVSBlob->GetBufferSize())	// 頂点シェーダー
		.SetPixelShader(boxFilterPSBlob->GetBufferPointer(), boxFilterPSBlob->GetBufferSize())		// ピクセルシェーダー
		.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
		.Create(device_->GetDevice(), boxFilterRootSignature_);
	Logger::Log(logStream, "Create BoxFilterPipelineState\n");
	boxFilterPipelineState_->SetName(L"BoxFilterPipelineState");

	// GaussianFilter用パイプラインステートの生成
	gaussianFilterPipelineState_ = PipelineState()
		.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)												// RTVのフォーマット
		.SetBlendState(blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNone)])						// BlendState
		.SetRasterizer(noCullingRasterizerDesc)																// RasterizerState
		.SetDepthState({ .DepthEnable = false })															// DepthStencilState
		.SetVertexShader(fullscreenVSBlob->GetBufferPointer(), fullscreenVSBlob->GetBufferSize())			// 頂点シェーダー
		.SetPixelShader(gaussianFilterPSBlob->GetBufferPointer(), gaussianFilterPSBlob->GetBufferSize())	// ピクセルシェーダー
		.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)									// プリミティブトポロジー
		.Create(device_->GetDevice(), gaussianFilterRootSignature_);
	Logger::Log(logStream, "Create GaussianFilterPipelineState\n");
	gaussianFilterPipelineState_->SetName(L"GaussianFilterPipelineState");

	// LuminanceBasedOutline用パイプラインステートの生成
	luminanceBasedOutlinePipelineState_ = PipelineState()
		.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)															// RTVのフォーマット
		.SetBlendState(blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNone)])									// BlendState
		.SetRasterizer(noCullingRasterizerDesc)																			// RasterizerState
		.SetDepthState({ .DepthEnable = false })																		// DepthStencilState
		.SetVertexShader(fullscreenVSBlob->GetBufferPointer(), fullscreenVSBlob->GetBufferSize())						// 頂点シェーダー
		.SetPixelShader(luminanceBasedOutlinePSBlob->GetBufferPointer(), luminanceBasedOutlinePSBlob->GetBufferSize())	// ピクセルシェーダー
		.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)												// プリミティブトポロジー
		.Create(device_->GetDevice(), luminanceBasedOutlineRootSignature_);
	Logger::Log(logStream, "Create LuminanceBasedOutlinePipelineState\n");
	luminanceBasedOutlinePipelineState_->SetName(L"LuminanceBasedOutlinePipelineState");

	// DepthBasedOutline用パイプラインステートの生成
	depthBasedOutlinePipelineState_ = PipelineState()
		.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)													// RTVのフォーマット
		.SetBlendState(blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNone)])							// BlendState
		.SetRasterizer(noCullingRasterizerDesc)																	// RasterizerState
		.SetDepthState({ .DepthEnable = false })																// DepthStencilState
		.SetVertexShader(fullscreenVSBlob->GetBufferPointer(), fullscreenVSBlob->GetBufferSize())				// 頂点シェーダー
		.SetPixelShader(depthBasedOutlinePSBlob->GetBufferPointer(), depthBasedOutlinePSBlob->GetBufferSize())	// ピクセルシェーダー
		.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)										// プリミティブトポロジー
		.Create(device_->GetDevice(), depthBasedOutlineRootSignature_);
	Logger::Log(logStream, "Create DepthBasedOutlinePipelineState\n");
	depthBasedOutlinePipelineState_->SetName(L"DepthBasedOutlinePipelineState");

	// RadialBlur用パイプラインステートの生成
	radialBlurPipelineState_ = PipelineState()
		.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
		.SetBlendState(blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNone)])				// BlendState
		.SetRasterizer(noCullingRasterizerDesc)														// RasterizerState
		.SetDepthState({ .DepthEnable = false })													// DepthStencilState
		.SetVertexShader(fullscreenVSBlob->GetBufferPointer(), fullscreenVSBlob->GetBufferSize())	// 頂点シェーダー
		.SetPixelShader(radialBlurPSBlob->GetBufferPointer(), radialBlurPSBlob->GetBufferSize())	// ピクセルシェーダー
		.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
		.Create(device_->GetDevice(), radialBlurRootSignature_);
	Logger::Log(logStream, "Create RadialBlurPipelineState\n");
	radialBlurPipelineState_->SetName(L"RadialBlurPipelineState");

	// Dissolve用パイプラインステートの生成
	dissolvePipelineState_ = PipelineState()
		.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
		.SetBlendState(blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNone)])				// BlendState
		.SetRasterizer(noCullingRasterizerDesc)														// RasterizerState
		.SetDepthState({ .DepthEnable = false })													// DepthStencilState
		.SetVertexShader(fullscreenVSBlob->GetBufferPointer(), fullscreenVSBlob->GetBufferSize())	// 頂点シェーダー
		.SetPixelShader(dissolvePSBlob->GetBufferPointer(), dissolvePSBlob->GetBufferSize())		// ピクセルシェーダー
		.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
		.Create(device_->GetDevice(), dissolveRootSignature_);
	Logger::Log(logStream, "Create DissolvePipelineState\n");
	dissolvePipelineState_->SetName(L"DissolvePipelineState");

	// Noise用パイプラインステートの生成
	noisePipelineState_ = PipelineState()
		.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
		.SetBlendState(blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNone)])				// BlendState
		.SetRasterizer(noCullingRasterizerDesc)														// RasterizerState
		.SetDepthState({ .DepthEnable = false })													// DepthStencilState
		.SetVertexShader(fullscreenVSBlob->GetBufferPointer(), fullscreenVSBlob->GetBufferSize())	// 頂点シェーダー
		.SetPixelShader(noisePSBlob->GetBufferPointer(), noisePSBlob->GetBufferSize())				// ピクセルシェーダー
		.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
		.Create(device_->GetDevice(), noiseRootSignature_);
	Logger::Log(logStream, "Create NoisePipelineState\n");
	noisePipelineState_->SetName(L"NoisePipelineState");

	// スキニング用パイプラインステートの生成
	skinningPipelineState_ = PipelineState()
		.SetComputeShader(skinningCSBlob->GetBufferPointer(), skinningCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), skinningRootSignature_);
	Logger::Log(logStream, "Create SkinningPipelineState\n");
	skinningPipelineState_->SetName(L"SkinningPipelineState");

	// パーティクル初期化用パイプラインステートの生成
	initializeParticlePipelineState_ = PipelineState()
		.SetComputeShader(initializeParticleCSBlob->GetBufferPointer(), initializeParticleCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), initializeParticleRootSignature_);
	Logger::Log(logStream, "Create InitializeParticlePipelineState\n");
	initializeParticlePipelineState_->SetName(L"InitializeParticlePipelineState");

	// パーティクル発生用パイプラインステートの生成
	emitParticlePipelineState_ = PipelineState()
		.SetComputeShader(emitParticleCSBlob->GetBufferPointer(), emitParticleCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), emitParticleRootSignature_);
	Logger::Log(logStream, "Create EmitParticlePipelineState\n");
	emitParticlePipelineState_->SetName(L"EmitParticlePipelineState");

	// パーティクル更新用パイプラインステートの生成
	updateParticlePipelineState_ = PipelineState()
		.SetComputeShader(updateParticleCSBlob->GetBufferPointer(), updateParticleCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), updateParticleRootSignature_);
	Logger::Log(logStream, "Create UpdateParticlePipelineState\n");
	updateParticlePipelineState_->SetName(L"UpdateParticlePipelineState");

	// 円柱のAABB生成用パイプラインステートの生成
	createCylinderAABBPipelineState_ = PipelineState()
		.SetComputeShader(createCylinderAABBCSBlob->GetBufferPointer(), createCylinderAABBCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), createCylinderAABBRootSignature_);
	Logger::Log(logStream, "Create CreateCylinderAABBPipelineState\n");
	createCylinderAABBPipelineState_->SetName(L"CreateCylinderAABBPipelineState");

	// モデルのAABB生成用パイプラインステートの生成
	createModelAABBPipelineState_ = PipelineState()
		.SetComputeShader(createModelAABBCSBlob->GetBufferPointer(), createModelAABBCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), createModelAABBRootSignature_);
	Logger::Log(logStream, "Create CreateModelAABBPipelineState\n");
	createModelAABBPipelineState_->SetName(L"CreateModelAABBPipelineState");

	// 深度ステンシルテクスチャコピー用パイプラインステートの生成
	depthStencilCopyPipelineState_ = PipelineState()
		.SetComputeShader(depthStencilCopyCSBlob->GetBufferPointer(), depthStencilCopyCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), depthStencilCopyRootSignature_);
	Logger::Log(logStream, "Create DepthStencilCopyPipelineState\n");
	depthStencilCopyPipelineState_->SetName(L"DepthStencilCopyPipelineState");

	// HiZミップマップ生成用パイプラインステートの生成
	generateHiZMipMapPipelineState_ = PipelineState()
		.SetComputeShader(generateHiZMipMapCSBlob->GetBufferPointer(), generateHiZMipMapCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), generateHiZMipMapRootSignature_);
	Logger::Log(logStream, "Create GenerateHiZMipMapPipelineState\n");
	generateHiZMipMapPipelineState_->SetName(L"GenerateHiZMipMapPipelineState");

	// メッシュコマンドステートのクリア用パイプラインステートの生成
	clearMeshCommandStatesPipelineState_ = PipelineState()
		.SetComputeShader(clearMeshCommandStatesCSBlob->GetBufferPointer(), clearMeshCommandStatesCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), clearMeshCommandStatesRootSignature_);
	Logger::Log(logStream, "Create ClearMeshCommandStatesPipelineState\n");
	clearMeshCommandStatesPipelineState_->SetName(L"ClearMeshCommandStatesPipelineState");

	// オクルージョンカリング用パイプラインステートの生成
	occlusionCullingPipelineState_ = PipelineState()
		.SetComputeShader(occlusionCullingCSBlob->GetBufferPointer(), occlusionCullingCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), occlusionCullingRootSignature_);
	Logger::Log(logStream, "Create OcclusionCullingPipelineState\n");
	occlusionCullingPipelineState_->SetName(L"OcclusionCullingPipelineState");

	// 累積和計算用パイプラインステートの生成
	prefixSumPipelineState_ = PipelineState()
		.SetComputeShader(prefixSumCSBlob->GetBufferPointer(), prefixSumCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), prefixSumRootSignature_);
	Logger::Log(logStream, "Create PrefixSumPipelineState\n");
	prefixSumPipelineState_->SetName(L"PrefixSumPipelineState");

	// インスタンス数の反映用パイプラインステートの生成
	setInstanceCountPipelineState_ = PipelineState()
		.SetComputeShader(setInstanceCountCSBlob->GetBufferPointer(), setInstanceCountCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), setInstanceCountRootSignature_);
	Logger::Log(logStream, "Create SetInstanceCountPipelineState\n");
	setInstanceCountPipelineState_->SetName(L"SetInstanceCountPipelineState");

	// インスタンスインデックスの反映用パイプラインステートの生成
	setInstanceIndexPipelineState_ = PipelineState()
		.SetComputeShader(setInstanceIndexCSBlob->GetBufferPointer(), setInstanceIndexCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), setInstanceIndexRootSignature_);
	Logger::Log(logStream, "Create SetInstanceIndexPipelineState\n");
	setInstanceIndexPipelineState_->SetName(L"SetInstanceIndexPipelineState");

	// フットプリント用パイプラインステートの生成
	footprintPipelineState_ = PipelineState()
		.SetComputeShader(footprintCSBlob->GetBufferPointer(), footprintCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), footprintRootSignature_);
	Logger::Log(logStream, "Create FootprintPipelineState\n");
	footprintPipelineState_->SetName(L"FootprintPipelineState");

	// フットプリントマップ用パイプラインステートの生成
	footprintMapPipelineState_ = PipelineState()
		.SetComputeShader(footprintMapCSBlob->GetBufferPointer(), footprintMapCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), footprintMapRootSignature_);
	Logger::Log(logStream, "Create FootprintMapPipelineState\n");
	footprintMapPipelineState_->SetName(L"FootprintMapPipelineState");

	// コマンドシグネチャの引数設定
	D3D12_INDIRECT_ARGUMENT_DESC argumentDescList[4] = {};
	argumentDescList[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
	argumentDescList[0].Constant.RootParameterIndex = 1;
	argumentDescList[0].Constant.DestOffsetIn32BitValues = 0;
	argumentDescList[0].Constant.Num32BitValuesToSet = 1;
	argumentDescList[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
	argumentDescList[1].VertexBuffer.Slot = 0;
	argumentDescList[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
	argumentDescList[3].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	// コマンドシグネチャの設定
	D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
	commandSignatureDesc.pArgumentDescs = argumentDescList;
	commandSignatureDesc.NumArgumentDescs = _countof(argumentDescList);
	commandSignatureDesc.ByteStride = sizeof(IndirectCommand);

	// コマンドシグネチャの生成
	for (size_t i = 0; i < static_cast<size_t>(MeshType::kCountOfMeshType); i++) {
		if (static_cast<MeshType>(i) == MeshType::kRing) {
			hr = device_->GetDevice()->CreateCommandSignature(&commandSignatureDesc, ringObject3dRootSignature_, IID_PPV_ARGS(&meshCommandSignature_[i]));
		} else {
			hr = device_->GetDevice()->CreateCommandSignature(&commandSignatureDesc, object3dRootSignature_, IID_PPV_ARGS(&meshCommandSignature_[i]));
		}
		assert(SUCCEEDED(hr));
	}
}

void Renderer::InitializeParticle() {
	// DescriptorHeapを設定する
	ID3D12DescriptorHeap *descriptorHeaps[] = { gpuCbvSrvUavDescriptorHeap_->GetDescriptorHeap() };
	commandList_->SetDescriptorHeaps(1, descriptorHeaps);

	commandList_->SetComputeRootSignature(initializeParticleRootSignature_);
	commandList_->SetPipelineState(initializeParticlePipelineState_.Get());

	gpuCbvSrvUavDescriptorHeap_->BindToCompute(1, world_->GetFreeListIndexHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(2, world_->GetFreeListHandle());

	registry_->ForEach<ParticleGroup>([&](uint32_t entity, ParticleGroup *particleGroup) {
		gpuCbvSrvUavDescriptorHeap_->BindToCompute(0, particleGroup->uavHandle);
		commandList_->Dispatch(1, 1, 1);
		}, exclude<Disabled>());
}

void Renderer::Render() {
	// パーティクルの発生
	EmitParticle();

	// パーティクルの更新
	UpdateParticle();

	// 円柱のAABBの生成
	CreateCylinderAABB();

	// モデルのAABBの生成
	CreateModelAABB();

	// スキニングの実行
	Skinning();

	// 深度ステンシルテクスチャのHiZテクスチャへのコピー
	CopyDepthToHiZ();

	// HiZミップマップの生成
	GenerateHiZMipMap();

	// メッシュコマンドステートのクリア
	ClearMeshCommandStates();

	// オクルージョンカリングの実行
	OcclusionCulling();

	// 累積和計算の実行
	PrefixSum();

	// インスタンス数の反映
	SetInstanceCount();

	// インスタンスインデックスの反映
	SetInstanceIndex();

	// フットプリントの実行
	Footprint();

	// 結果マップの読み込み
	if (world_->IsResult()) {
		LoadResultMap();
		world_->SetResult(false);
	}

	// フットプリントマップのコピー
	world_->CopyFootprintMapBuffer();

	// シーンビューの描画
	RenderSceneView();

	// ゲームビューの描画	
	RenderGameView();

#ifdef USE_IMGUI
	// スワップチェーンの設定
	device_->SetupSwapChain();
	return;
#endif // USE_IMGUI

	// リリース構成の描画
	RenderRelease();
}

void Renderer::SetRegistry(Registry *registry) {
	assert(registry);
	registry_ = registry;
}

void Renderer::SetWorld(World *world) {
	assert(world);
	world_ = world;
}

void Renderer::SetDebugRenderer(DebugRenderer *debugRenderer) {
	assert(debugRenderer);
	debugRenderer_ = debugRenderer;
}

void Renderer::SetMeshManager(MeshManager *meshManager) {
	assert(meshManager);
	meshManager_ = meshManager;
}

void Renderer::SetTextureManager(TextureManager *textureManager) {
	assert(textureManager);
	textureManager_ = textureManager;
}

void Renderer::SetSkinClusterManager(SkinClusterManager *skinClusterManager) {
	assert(skinClusterManager);
	skinClusterManager_ = skinClusterManager;
}

void Renderer::SetParticleManager(ParticleManager *particleManager) {
	assert(particleManager);
	particleManager_ = particleManager;
}

void Renderer::SetFootprintManager(FootprintManager *footprintManager) {
	assert(footprintManager);
	footprintManager_ = footprintManager;
}

void Renderer::EmitParticle() {
	// DescriptorHeapを設定する
	ID3D12DescriptorHeap *descriptorHeaps[] = { gpuCbvSrvUavDescriptorHeap_->GetDescriptorHeap() };
	commandList_->SetDescriptorHeaps(1, descriptorHeaps);

	commandList_->SetComputeRootSignature(emitParticleRootSignature_);
	commandList_->SetPipelineState(emitParticlePipelineState_.Get());

	world_->GetConstantBuffer(ConstantBufferType::kEmitterSphere)->BindToCompute(0, 0);
	world_->GetConstantBuffer(ConstantBufferType::kPerFrame)->BindToCompute(1, 0);
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(3, world_->GetFreeListIndexHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(4, world_->GetFreeListHandle());

	registry_->ForEach<ParticleGroup>([&](uint32_t entity, ParticleGroup *particleGroup) {
		gpuCbvSrvUavDescriptorHeap_->BindToCompute(2, particleGroup->uavHandle);
		commandList_->Dispatch(1, 1, 1);
		particleManager_->GetParticleResource(particleGroup->resourceHandle)->UAVBarrier();
		}, exclude<Disabled>());
}

void Renderer::UpdateParticle() {
	commandList_->SetComputeRootSignature(updateParticleRootSignature_);
	commandList_->SetPipelineState(updateParticlePipelineState_.Get());

	world_->GetConstantBuffer(ConstantBufferType::kPerFrame)->BindToCompute(0, 0);
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(2, world_->GetFreeListIndexHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(3, world_->GetFreeListHandle());

	registry_->ForEach<ParticleGroup>([&](uint32_t entity, ParticleGroup *particleGroup) {
		gpuCbvSrvUavDescriptorHeap_->BindToCompute(1, particleGroup->uavHandle);
		commandList_->Dispatch(1, 1, 1);
		}, exclude<Disabled>());
}

void Renderer::Skinning() {
	commandList_->SetComputeRootSignature(skinningRootSignature_);
	commandList_->SetPipelineState(skinningPipelineState_.Get());

	registry_->ForEach<Model, SkinMesh>([&](uint32_t entity, Model *model, SkinMesh *skinMesh) {
		size_t vertexCount = model->modelData.meshes.back().lods.back().vertices.size();
		commandList_->SetComputeRoot32BitConstants(0, 1, &vertexCount, 0);
		gpuCbvSrvUavDescriptorHeap_->BindToCompute(1, skinClusterManager_->GetPaletteSRVHandle(model->skinClusterHandle));
		gpuCbvSrvUavDescriptorHeap_->BindToCompute(2, skinClusterManager_->GetVertexSRVHandle(model->skinClusterHandle));
		gpuCbvSrvUavDescriptorHeap_->BindToCompute(3, skinClusterManager_->GetInfluenceSRVHandle(model->skinClusterHandle));
		gpuCbvSrvUavDescriptorHeap_->BindToCompute(4, skinClusterManager_->GetVertexUAVHandle(model->skinClusterHandle));
		commandList_->Dispatch((static_cast<uint32_t>(vertexCount) + 1023) / 1024, 1, 1);
		}, exclude<Disabled>());
}

void Renderer::CreateCylinderAABB() {
	commandList_->SetComputeRootSignature(createCylinderAABBRootSignature_);
	commandList_->SetPipelineState(createCylinderAABBPipelineState_.Get());

	uint32_t cylinderCount = world_->GetCylinderCounter();
	commandList_->SetComputeRoot32BitConstants(0, 1, &cylinderCount, 0);
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(1, world_->GetStructuredBufferHandle(StructuredBufferType::kCylinder));
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(2, world_->GetAABBUAVHandle());
	uint32_t dispatchCount = (cylinderCount + 63) / 64;
	if (dispatchCount > 0) {
		commandList_->Dispatch(dispatchCount, 1, 1);
	}
}

void Renderer::CreateModelAABB() {
	commandList_->SetComputeRootSignature(createModelAABBRootSignature_);
	commandList_->SetPipelineState(createModelAABBPipelineState_.Get());

	gpuCbvSrvUavDescriptorHeap_->BindToCompute(0, world_->GetStructuredBufferHandle(StructuredBufferType::kMeshInfoForAABB));
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(1, world_->GetStructuredBufferHandle(StructuredBufferType::kVertexDataForAABB));
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(2, world_->GetAABBUAVHandle());
	uint32_t meshInfoForAABBCount = world_->GetMeshInfoForAABBCounter();
	if (meshInfoForAABBCount > 0) {
		commandList_->Dispatch(meshInfoForAABBCount, 1, 1);
	}
}

void Renderer::CopyDepthToHiZ() {
	// 深度ステンシルテクスチャコピー用ルートシグネチャとパイプラインステートの設定
	commandList_->SetComputeRootSignature(depthStencilCopyRootSignature_);
	commandList_->SetPipelineState(depthStencilCopyPipelineState_.Get());

	Resource *depthStencilTexture = device_->GetPreviousMainCameraDepthStencilTexture();
	Resource *hiZTexture = world_->GetHiZTexture();
	depthStencilTexture->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	hiZTexture->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0);

	// 各種バッファのSRV/UAVを設定
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(0, world_->GetPreviousMainCameraDepthStencilTextureSRVHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(1, world_->GetHiZTextureUAVHandle());

	// コピーの実行
	uint32_t groupsX = (static_cast<uint32_t>(device_->GetViewport().Width) + 7) / 8;
	uint32_t groupsY = (static_cast<uint32_t>(device_->GetViewport().Height) + 7) / 8;
	commandList_->Dispatch(groupsX, groupsY, 1);
	hiZTexture->UAVBarrier();
	hiZTexture->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 0);
	depthStencilTexture->TransitionBarrier(D3D12_RESOURCE_STATE_COPY_DEST);
}

void Renderer::GenerateHiZMipMap() {
	// HiZミップマップ生成用ルートシグネチャとパイプラインステートの設定
	commandList_->SetComputeRootSignature(generateHiZMipMapRootSignature_);
	commandList_->SetPipelineState(generateHiZMipMapPipelineState_.Get());

	// HiZテクスチャの取得
	Resource *hiZTexture = world_->GetHiZTexture();

	// ミップマップレベルごとに処理を実行
	uint32_t mipLevels = world_->GetMipLevels();
	for (uint32_t level = 0; level < mipLevels - 1; level++) {
		// ミップマップレベルの遷移バリアの設定
		hiZTexture->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, level + 1);

		// 各種バッファのSRV/UAVを設定
		gpuCbvSrvUavDescriptorHeap_->BindToCompute(0, world_->GetHiZMipMapReadHandle(level));
		gpuCbvSrvUavDescriptorHeap_->BindToCompute(1, world_->GetHiZMipMapWriteHandle(level));

		// ミップマップレベルに応じたグループ数を計算
		uint32_t width = std::max(static_cast<uint32_t>(device_->GetViewport().Width) >> level, 1u);
		uint32_t height = std::max(static_cast<uint32_t>(device_->GetViewport().Height) >> level, 1u);
		uint32_t groupsX = (width + 7) / 8;
		uint32_t groupsY = (height + 7) / 8;

		// ミップマップ生成の実行
		commandList_->Dispatch(groupsX, groupsY, 1);

		// UAVバリアの設定
		hiZTexture->UAVBarrier();
		hiZTexture->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, level + 1);
	}
}

void Renderer::ClearMeshCommandStates() {
	commandList_->SetComputeRootSignature(clearMeshCommandStatesRootSignature_);
	commandList_->SetPipelineState(clearMeshCommandStatesPipelineState_.Get());

	uint32_t meshLODCounter = world_->GetMeshLODCounter();
	commandList_->SetComputeRoot32BitConstants(0, 1, &meshLODCounter, 0);

	gpuCbvSrvUavDescriptorHeap_->BindToCompute(1, world_->GetMeshCommandStateUAVHandle());

	world_->GetMeshCommandStateBuffer()->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	uint32_t dispatchCount = (meshLODCounter + 63) / 64;
	if (dispatchCount > 0) {
		commandList_->Dispatch(dispatchCount, 1, 1);
		world_->GetMeshCommandStateBuffer()->UAVBarrier();
	}
}

void Renderer::OcclusionCulling() {
	// オクルージョンカリング用ルートシグネチャとパイプラインステートの設定
	commandList_->SetComputeRootSignature(occlusionCullingRootSignature_);
	commandList_->SetPipelineState(occlusionCullingPipelineState_.Get());

	// CBVの設定
	world_->GetConstantBuffer(ConstantBufferType::kFrustum)->BindToCompute(0, 0);
	world_->GetConstantBuffer(ConstantBufferType::kViewProjection)->BindToCompute(1, 1);
	world_->GetConstantBuffer(ConstantBufferType::kCameraPosition)->BindToCompute(2, 0);

	// メッシュ数の設定
	CullingConstantsData cullingConstantsData = world_->GetCullingConstantsData();
	commandList_->SetComputeRoot32BitConstants(3, sizeof(CullingConstantsData) / sizeof(uint32_t), &cullingConstantsData, 0);

	// 各種バッファのSRV/UAVを設定
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(4, world_->GetStructuredBufferHandle(StructuredBufferType::kCullingObjectData));
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(5, world_->GetStructuredBufferHandle(StructuredBufferType::kCullingMeshData));
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(6, world_->GetStructuredBufferHandle(StructuredBufferType::kMeshLOD));
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(7, world_->GetAABBSRVHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(8, world_->GetHiZTextureSRVHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(9, world_->GetMeshCommandStateUAVHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(10, world_->GetMeshLODStateUAVHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(11, world_->GetProcessedCommandHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(12, world_->GetCommandCounterHandle());

	// コマンドバッファの転送
	Resource *indirectCommandStructuredBuffer = world_->GetStructuredBuffer(StructuredBufferType::kMeshLOD);
	indirectCommandStructuredBuffer->TransitionBarrier(D3D12_RESOURCE_STATE_COPY_DEST);
	indirectCommandStructuredBuffer->CopyFrom(world_->GetCommandUploadBuffer()->GetResource(), 0, 0, sizeof(MeshLOD) * world_->GetMaxAABB());
	indirectCommandStructuredBuffer->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	// オクルージョンカリングの実行前にUAVを遷移する
	world_->GetProcessedCommandBuffer()->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	world_->GetMeshLODStateBuffer()->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	world_->GetCommandCounterBuffer()->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// オクルージョンカリングの実行前にカウンタをクリアする
	UINT clearValues[4] = { 0, 0, 0, 0 };
	commandList_->ClearUnorderedAccessViewUint(
		gpuCbvSrvUavDescriptorHeap_->GetGPUDescriptorHandle(world_->GetCommandCounterHandle()),
		cpuCbvSrvUavDescriptorHeap_->GetCPUDescriptorHandle(world_->GetCommandCounterHandle()),
		world_->GetCommandCounterBuffer()->GetResource(),
		clearValues,
		0,
		nullptr
	);

	world_->GetCommandCounterBuffer()->UAVBarrier();

	// オクルージョンカリングの実行
	uint32_t dispatchCount = (cullingConstantsData.meshCount + 63) / 64;
	if (dispatchCount > 0) {
		commandList_->Dispatch(dispatchCount, 1, 1);
		world_->GetProcessedCommandBuffer()->UAVBarrier();
		world_->GetMeshCommandStateBuffer()->UAVBarrier();
		world_->GetMeshLODStateBuffer()->UAVBarrier();
		world_->GetCommandCounterBuffer()->UAVBarrier();
	}
}

void Renderer::PrefixSum() {
	commandList_->SetComputeRootSignature(prefixSumRootSignature_);
	commandList_->SetPipelineState(prefixSumPipelineState_.Get());

	uint32_t meshLODCounter = world_->GetMeshLODCounter();
	commandList_->SetComputeRoot32BitConstants(0, 1, &meshLODCounter, 0);
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(1, world_->GetMeshCommandStateUAVHandle());

	commandList_->Dispatch(1, 1, 1);
	world_->GetMeshCommandStateBuffer()->UAVBarrier();
}

void Renderer::SetInstanceCount() {
	commandList_->SetComputeRootSignature(setInstanceCountRootSignature_);
	commandList_->SetPipelineState(setInstanceCountPipelineState_.Get());

	uint32_t meshLODCounter = world_->GetMeshLODCounter();
	commandList_->SetComputeRoot32BitConstants(0, 1, &meshLODCounter, 0);

	gpuCbvSrvUavDescriptorHeap_->BindToCompute(1, world_->GetMeshCommandStateSRVHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(2, world_->GetProcessedCommandHandle());

	world_->GetMeshCommandStateBuffer()->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	uint32_t dispatchCount = (meshLODCounter + 63) / 64;
	if (dispatchCount > 0) {
		commandList_->Dispatch(dispatchCount, 1, 1);
	}
}

void Renderer::SetInstanceIndex() {
	commandList_->SetComputeRootSignature(setInstanceIndexRootSignature_);
	commandList_->SetPipelineState(setInstanceIndexPipelineState_.Get());

	uint32_t meshCount = world_->GetCullingConstantsData().meshCount;
	commandList_->SetComputeRoot32BitConstants(0, 1, &meshCount, 0);

	gpuCbvSrvUavDescriptorHeap_->BindToCompute(1, world_->GetMeshLODStateSRVHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(2, world_->GetMeshCommandStateUAVHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(3, world_->GetProcessedInstanceIndexUAVHandle());

	world_->GetMeshLODStateBuffer()->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	world_->GetMeshCommandStateBuffer()->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	world_->GetProcessedInstanceIndexBuffer()->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	uint32_t dispatchCount = (meshCount + 63) / 64;
	if (dispatchCount > 0) {
		commandList_->Dispatch(dispatchCount, 1, 1);
		world_->GetProcessedInstanceIndexBuffer()->UAVBarrier();
	}
}

void Renderer::Footprint() {
	// フットプリント用ルートシグネチャとパイプラインステートの設定
	commandList_->SetComputeRootSignature(footprintRootSignature_);
	commandList_->SetPipelineState(footprintPipelineState_.Get());

	// 各種バッファのSRV/UAVを設定
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(1, world_->GetStructuredBufferHandle(StructuredBufferType::kFootprint));
	uint32_t groupsZ = footprintManager_->GetCurrentIndex();
	footprintManager_->RemoveOnceFootprint();

	uint32_t footprintMapCounter = 0;
	registry_->ForEach<Model, FootprintMap>([&](uint32_t entity, Model *model, FootprintMap *footprintMap) {
		// フットプリントマップテクスチャをUAVに遷移
		textureManager_->GetTextureResource(model->modelData.materials.back().textureFilePath)->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		// ゲーム終了時はUAVをクリア
		if (isGameFinished_) {
			float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			commandList_->ClearUnorderedAccessViewFloat(
				gpuCbvSrvUavDescriptorHeap_->GetGPUDescriptorHandle(textureManager_->GetTextureWriteHandle(model->modelData.materials.back().textureFilePath)),
				cpuCbvSrvUavDescriptorHeap_->GetCPUDescriptorHandle(textureManager_->GetTextureWriteHandle(model->modelData.materials.back().textureFilePath)),
				textureManager_->GetTextureResource(model->modelData.materials.back().textureFilePath)->GetResource(),
				clearColor,
				0,
				nullptr
			);
			isGameFinished_ = false;
		}

		// フットプリント用CBVを設定
		world_->GetConstantBuffer(ConstantBufferType::kFootprintMap)->BindToCompute(0, footprintMapCounter);
		gpuCbvSrvUavDescriptorHeap_->BindToCompute(2, textureManager_->GetTextureWriteHandle(model->modelData.materials.back().textureFilePath));

		// ルートパラメータに基点座標を設定
		uint32_t groupsX = (static_cast<uint32_t>(textureManager_->GetResourceDesc(model->modelData.materials.back().textureFilePath).Width) + 31) / 32;
		uint32_t groupsY = (static_cast<uint32_t>(textureManager_->GetResourceDesc(model->modelData.materials.back().textureFilePath).Height) + 31) / 32;
		if (groupsZ > 0) {
			commandList_->Dispatch(groupsX, groupsY, groupsZ);
		} else {
			commandList_->Dispatch(groupsX, groupsY, 1);
		}

		// フットプリントマップテクスチャをSRVに遷移
		textureManager_->GetTextureResource(model->modelData.materials.back().textureFilePath)->TransitionBarrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		footprintMapCounter++;
		}, exclude<Disabled>());
}

void Renderer::LoadResultMap() {
	// フットプリントマップ用ルートシグネチャとパイプラインステートの設定
	commandList_->SetComputeRootSignature(footprintMapRootSignature_);
	commandList_->SetPipelineState(footprintMapPipelineState_.Get());

	// 各種バッファのSRV/UAVを設定
	registry_->ForEach<Model, FootprintMap>([&](uint32_t entity, Model *model, FootprintMap *footprintMap) {
		// フットプリントマップテクスチャをSRVに遷移
		textureManager_->GetTextureResource(model->modelData.materials.back().textureFilePath)->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		// フットプリント用CBVを設定
		gpuCbvSrvUavDescriptorHeap_->BindToCompute(0, textureManager_->GetTextureReadHandle(model->modelData.materials.back().textureFilePath));
		gpuCbvSrvUavDescriptorHeap_->BindToCompute(1, world_->GetFootprintMapHandle());

		// ルートパラメータに基点座標を設定
		uint32_t groupsX = (static_cast<uint32_t>(textureManager_->GetResourceDesc(model->modelData.materials.back().textureFilePath).Width) + 31) / 32;
		uint32_t groupsY = (static_cast<uint32_t>(textureManager_->GetResourceDesc(model->modelData.materials.back().textureFilePath).Height) + 31) / 32;
		commandList_->Dispatch(groupsX, groupsY, 1);

		// フットプリントマップテクスチャをSRVに遷移
		textureManager_->GetTextureResource(model->modelData.materials.back().textureFilePath)->TransitionBarrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}, exclude<Disabled>());
}

void Renderer::RenderSceneView() {
#ifdef USE_IMGUI
	if (!isSceneViewVisible_) {
		return;
	}

	world_->GetSceneRenderTexture()->TransitionBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET);

	bool isDebugCamera = false;
	registry_->ForEach<Camera, RenderingCamera>([&](uint32_t entity, Camera *camera, RenderingCamera *renderingCamera) {
		isDebugCamera = true;
		}, exclude<Disabled, MainCamera>());

	if (isDebugCamera) {
		// レンダーターゲットの設定
		SetupRenderTarget(world_->GetSceneRenderTextureRTVHandle(), device_->GetDebugCameraDepthStencilTextureDSVHandle());

		// ワールド描画
		RenderWorld(kDebugCameraIndex);

		// デバッグ描画
		DrawLine(kDebugCameraIndex);
	} else {
		// レンダーターゲットの設定
		SetupRenderTarget(world_->GetSceneRenderTextureRTVHandle(), device_->GetMainCameraDepthStencilTextureDSVHandle());

		// ワールド描画
		RenderWorld(kMainCameraIndex);

		// デバッグ描画
		DrawLine(kMainCameraIndex);
	}

	world_->GetSceneRenderTexture()->TransitionBarrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#endif // USE_IMGUI
}

void Renderer::RenderGameView() {
#ifdef USE_IMGUI
	if (!isGameViewVisible_) {
		return;
	}

	world_->GetPostEffectRenderTexture()->TransitionBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET);

	// ゲームビューのレンダーターゲットの設定
	SetupRenderTarget(world_->GetGameRenderTextureRTVHandle(), device_->GetMainCameraDepthStencilTextureDSVHandle());

	// ワールド描画
	RenderWorld(kMainCameraIndex);

	// ポストエフェクトのレンダーターゲットの設定
	SetupRenderTarget(world_->GetPostEffectRenderTextureRTVHandle(), device_->GetMainCameraDepthStencilTextureDSVHandle(), false);

	// ポストエフェクトの描画
	CopyImage();

	world_->GetPostEffectRenderTexture()->TransitionBarrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#endif // USE_IMGUI
}

void Renderer::RenderRelease() {
	// ゲームビューのレンダーターゲットの設定
	SetupRenderTarget(world_->GetGameRenderTextureRTVHandle(), device_->GetMainCameraDepthStencilTextureDSVHandle());

	// ワールド描画
	RenderWorld(kMainCameraIndex);

	// スワップチェーンの設定
	device_->SetupSwapChain();

	// ポストエフェクトの描画
	CopyImage();
}

void Renderer::SetupRenderTarget(uint32_t rtvHandle, uint32_t dsvHandle, bool clearDepth) {
	DescriptorHeap *rtvDescriptorHeap = device_->GetRTVDescriptorHeap();
	DescriptorHeap *dsvDescriptorHeap = device_->GetDSVDescriptorHeap();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle = rtvDescriptorHeap->GetCPUDescriptorHandle(rtvHandle);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUHandle = dsvDescriptorHeap->GetCPUDescriptorHandle(dsvHandle);

	// 描画先のRTVとDSVを設定する
	commandList_->OMSetRenderTargets(1, &rtvCPUHandle, false, &dsvCPUHandle);

	// 指定した色で画面全体をクリアする
	float clearColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	commandList_->ClearRenderTargetView(rtvCPUHandle, clearColor, 0, nullptr);

	// 指定した深度で画面全体をクリアする
	if (clearDepth) {
		commandList_->ClearDepthStencilView(dsvCPUHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	// ビューポートとシザー矩形の設定
	D3D12_VIEWPORT viewport = device_->GetViewport();
	D3D12_RECT scissorRect = device_->GetScissorRect();
	commandList_->RSSetViewports(1, &viewport);			// ビューポートの設定
	commandList_->RSSetScissorRects(1, &scissorRect);	// シザー矩形の設定
}

void Renderer::RenderWorld(uint32_t cameraBufferLocationIndex) {
	DrawSkybox(cameraBufferLocationIndex);
	DrawMesh(cameraBufferLocationIndex);
	DrawParticle(cameraBufferLocationIndex);
	DrawSprite();
}

void Renderer::DrawMesh(uint32_t cameraBufferLocationIndex) {
	// 三角形のトポロジの設定
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// リソースの遷移バリアの設定
	world_->GetProcessedInstanceIndexBuffer()->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	world_->GetProcessedCommandBuffer()->TransitionBarrier(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	world_->GetCommandCounterBuffer()->TransitionBarrier(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	// メッシュタイプごとに描画
	for (uint32_t i = 0; i < static_cast<uint32_t>(MeshType::kCountOfMeshType); i++) {
		std::string label = "Draw" + meshTypeNames[i];
		PIXBeginEvent(commandList_, pixColors[i], ConvertString(label).c_str());
		if (static_cast<MeshType>(i) == MeshType::kRing) {
			commandList_->SetGraphicsRootSignature(ringObject3dRootSignature_);
		} else {
			commandList_->SetGraphicsRootSignature(object3dRootSignature_);
		}

		// メッシュの共通のCBV・SRVを設定
		world_->GetConstantBuffer(ConstantBufferType::kViewProjection)->BindToGraphics(0, cameraBufferLocationIndex);
		world_->GetConstantBuffer(ConstantBufferType::kCameraPosition)->BindToGraphics(2, 0);
		world_->GetConstantBuffer(ConstantBufferType::kDirectionalLight)->BindToGraphics(3, 0);
		LightData lightData = {
			.pointLightCount = static_cast<uint32_t>(registry_->GetComponentCount<PointLight>()),
			.spotLightCount = static_cast<uint32_t>(registry_->GetComponentCount<SpotLight>())
		};
		commandList_->SetGraphicsRoot32BitConstants(4, 2, &lightData, 0);
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(5, world_->GetProcessedInstanceIndexSRVHandle());
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(6, world_->GetStructuredBufferHandle(StructuredBufferType::kInstanceData));
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(7, world_->GetStructuredBufferHandle(StructuredBufferType::kMaterial));
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(8, world_->GetStructuredBufferHandle(StructuredBufferType::kTextureData));
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(9, world_->GetStructuredBufferHandle(StructuredBufferType::kPointLight));
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(10, world_->GetStructuredBufferHandle(StructuredBufferType::kSpotLight));
		registry_->ForEach<Skybox>([&](uint32_t entity, Skybox *skybox) {
			gpuCbvSrvUavDescriptorHeap_->BindToGraphics(11, skybox->textureHandle);
			}, exclude<Disabled>());
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(12, 0);

		// ブレンドモードごとに描画
		for (uint32_t j = 0; j < static_cast<uint32_t>(BlendMode::kCountOfBlendMode); j++) {
			label = blendModeNames[j] + "Blend";
			PIXBeginEvent(commandList_, PIX_COLOR(255, 255, 255), ConvertString(label).c_str());
			commandList_->SetPipelineState(meshPipelineState_[i][j].Get());

			// メッシュの描画
			uint32_t queueIndex = i * static_cast<uint32_t>(BlendMode::kCountOfBlendMode) + j;
			commandList_->ExecuteIndirect(
				meshCommandSignature_[i].Get(),
				world_->GetMaxCommandPerQueue(),
				world_->GetProcessedCommandBuffer()->GetResource(),
				sizeof(IndirectCommand) * queueIndex * world_->GetMaxCommandPerQueue(),
				world_->GetCommandCounterBuffer()->GetResource(),
				sizeof(uint32_t) * queueIndex);
			PIXEndEvent(commandList_);
		}
		PIXEndEvent(commandList_);
	}
}

void Renderer::DrawParticle(uint32_t cameraBufferLocationIndex) {
	// メッシュタイプごとに描画
	for (size_t i = 0; i < static_cast<uint32_t>(MeshType::kCountOfMeshType); i++) {
		std::string label = "Draw" + meshTypeNames[i] + "Particle";
		PIXBeginEvent(commandList_, pixColors[i], ConvertString(label).c_str());

		// Particle用ルートシグネチャの設定
		commandList_->SetGraphicsRootSignature(static_cast<MeshType>(i) == MeshType::kRing ? ringParticleRootSignature_ : particleRootSignature_);

		// パーティクルのビューごとのデータのCBVを設定
		world_->GetConstantBuffer(ConstantBufferType::kParticlePerView)->BindToGraphics(0, cameraBufferLocationIndex - 1);

		// パーティクルのテクスチャのSRVを設定
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(3, 0);

		// ブレンドモードごとに描画
		for (size_t j = 0; j < static_cast<uint32_t>(BlendMode::kCountOfBlendMode); j++) {
			label = blendModeNames[j] + "Blend";
			PIXBeginEvent(commandList_, PIX_COLOR(255, 255, 255), ConvertString(label).c_str());
			// MeshParticle用パイプラインステートの設定
			commandList_->SetPipelineState(meshParticlePipelineState_[i][j].Get());

			// MeshParticleの描画
			registry_->ForEach<MeshType, BlendMode, ParticleGroup>([&](uint32_t entity, MeshType *meshType, BlendMode *blendMode, ParticleGroup *particleGroup) {
				if (*meshType == static_cast<MeshType>(i) && *blendMode == static_cast<BlendMode>(j)) {
					commandList_->SetGraphicsRoot32BitConstant(1, particleGroup->textureHandle, 0);
					gpuCbvSrvUavDescriptorHeap_->BindToGraphics(2, particleGroup->srvHandle);
					meshManager_->Draw(particleGroup->meshName, ParticleManager::kMaxParticle);
				}
				}, exclude<Disabled>());
			PIXEndEvent(commandList_);
		}
		PIXEndEvent(commandList_);
	}
}

void Renderer::DrawSprite() {
	// Object3d用ルートシグネチャの設定
	commandList_->SetGraphicsRootSignature(object3dRootSignature_);

	// 各ブレンドモードのスプライトの描画
	for (uint32_t i = 0; i < static_cast<uint32_t>(BlendMode::kCountOfBlendMode); i++) {
		// Sprite用パイプラインステートの設定
		commandList_->SetPipelineState(spritePipelineState_[i].Get());

		// スプライトの描画
		registry_->ForEach<BlendMode, Sprite>([&](uint32_t entity, BlendMode *blendMode, Sprite *sprite) {
			if (*blendMode == static_cast<BlendMode>(i)) {
				meshManager_->Draw(sprite->meshName, 1);
			}
			}, exclude<Disabled>());
	}
}

void Renderer::DrawSkybox(uint32_t cameraBufferLocationIndex) {
	// Skybox用ルートシグネチャの設定
	commandList_->SetGraphicsRootSignature(skyboxRootSignature_);

	// 三角形のトポロジの設定
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Skybox用パイプラインステートの設定
	commandList_->SetPipelineState(skyboxPipelineState_.Get());

	// スカイボックスのCBVを設定
	world_->GetConstantBuffer(ConstantBufferType::kViewProjection)->BindToGraphics(1, cameraBufferLocationIndex);

	// スカイボックスの描画
	registry_->ForEach<Skybox>([&](uint32_t entity, Skybox *skybox) {
		world_->GetConstantBuffer(ConstantBufferType::kSkybox)->BindToGraphics(0, 0);
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(2, skybox->textureHandle);
		meshManager_->Draw(skybox->meshName, 1);
		}, exclude<Disabled>());
}

void Renderer::CopyImage() {
	world_->GetGameRenderTexture()->TransitionBarrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	device_->GetMainCameraDepthStencilTexture()->TransitionBarrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	switch (world_->GetPostEffect()) {
		case PostEffect::kNone:
			commandList_->SetGraphicsRootSignature(fullscreenRootSignature_);
			commandList_->SetPipelineState(fullscreenPipelineState_.Get());
			gpuCbvSrvUavDescriptorHeap_->BindToGraphics(0, world_->GetGameRenderTextureSRVHandle());
			break;
		case PostEffect::kGrayscale:
			commandList_->SetGraphicsRootSignature(grayscaleRootSignature_);
			commandList_->SetPipelineState(grayscalePipelineState_.Get());
			world_->GetConstantBuffer(ConstantBufferType::kGrayscaleColor)->BindToGraphics(0, 0);
			gpuCbvSrvUavDescriptorHeap_->BindToGraphics(1, world_->GetGameRenderTextureSRVHandle());
			break;
		case PostEffect::kVignette:
			commandList_->SetGraphicsRootSignature(vignetteRootSignature_);
			commandList_->SetPipelineState(vignettePipelineState_.Get());
			world_->GetConstantBuffer(ConstantBufferType::kVignetteParam)->BindToGraphics(0, 0);
			gpuCbvSrvUavDescriptorHeap_->BindToGraphics(1, world_->GetGameRenderTextureSRVHandle());
			break;
		case PostEffect::kBoxFilter:
			commandList_->SetGraphicsRootSignature(boxFilterRootSignature_);
			commandList_->SetPipelineState(boxFilterPipelineState_.Get());
			world_->GetConstantBuffer(ConstantBufferType::kBoxFilterParam)->BindToGraphics(0, 0);
			gpuCbvSrvUavDescriptorHeap_->BindToGraphics(1, world_->GetGameRenderTextureSRVHandle());
			break;
		case PostEffect::kGaussianFilter:
			commandList_->SetGraphicsRootSignature(gaussianFilterRootSignature_);
			commandList_->SetPipelineState(gaussianFilterPipelineState_.Get());
			world_->GetConstantBuffer(ConstantBufferType::kGaussianFilterParam)->BindToGraphics(0, 0);
			gpuCbvSrvUavDescriptorHeap_->BindToGraphics(1, world_->GetGameRenderTextureSRVHandle());
			break;
		case PostEffect::kLuminanceBasedOutline:
			commandList_->SetGraphicsRootSignature(luminanceBasedOutlineRootSignature_);
			commandList_->SetPipelineState(luminanceBasedOutlinePipelineState_.Get());
			world_->GetConstantBuffer(ConstantBufferType::kPrewittFilterParam)->BindToGraphics(0, 0);
			gpuCbvSrvUavDescriptorHeap_->BindToGraphics(1, world_->GetGameRenderTextureSRVHandle());
			break;
		case PostEffect::kDepthBasedOutline:
			commandList_->SetGraphicsRootSignature(depthBasedOutlineRootSignature_);
			commandList_->SetPipelineState(depthBasedOutlinePipelineState_.Get());
			world_->GetConstantBuffer(ConstantBufferType::kPrewittFilterParam)->BindToGraphics(0, 1);
			world_->GetConstantBuffer(ConstantBufferType::kDepthMaterial)->BindToGraphics(1, 0);
			gpuCbvSrvUavDescriptorHeap_->BindToGraphics(2, world_->GetGameRenderTextureSRVHandle());
			gpuCbvSrvUavDescriptorHeap_->BindToGraphics(3, world_->GetMainCameraDepthStencilTextureSRVHandle());
			break;
		case PostEffect::kRadialBlur:
			commandList_->SetGraphicsRootSignature(radialBlurRootSignature_);
			commandList_->SetPipelineState(radialBlurPipelineState_.Get());
			world_->GetConstantBuffer(ConstantBufferType::kRadialBlurParam)->BindToGraphics(0, 0);
			gpuCbvSrvUavDescriptorHeap_->BindToGraphics(1, world_->GetGameRenderTextureSRVHandle());
			break;
		case PostEffect::kDissolve:
			commandList_->SetGraphicsRootSignature(dissolveRootSignature_);
			commandList_->SetPipelineState(dissolvePipelineState_.Get());
			world_->GetConstantBuffer(ConstantBufferType::kDissolveParam)->BindToGraphics(0, 0);
			gpuCbvSrvUavDescriptorHeap_->BindToGraphics(1, world_->GetGameRenderTextureSRVHandle());
			gpuCbvSrvUavDescriptorHeap_->BindToGraphics(2, textureManager_->GetTextureReadHandle("noise0.png"));
			break;
		case PostEffect::kNoise:
			commandList_->SetGraphicsRootSignature(noiseRootSignature_);
			commandList_->SetPipelineState(noisePipelineState_.Get());
			world_->GetConstantBuffer(ConstantBufferType::kPerFrame)->BindToGraphics(0, 0);
			gpuCbvSrvUavDescriptorHeap_->BindToGraphics(1, world_->GetGameRenderTextureSRVHandle());
			break;
		case PostEffect::kCountOfPostEffect:
			break;
		default:
			break;
	}
	commandList_->DrawInstanced(3, 1, 0, 0);
	device_->GetMainCameraDepthStencilTexture()->TransitionBarrier(D3D12_RESOURCE_STATE_DEPTH_WRITE);
	world_->GetGameRenderTexture()->TransitionBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void Renderer::DrawLine(uint32_t cameraBufferLocationIndex) {
#ifdef DRAW_LINE
	// Line用ルートシグネチャの設定
	commandList_->SetGraphicsRootSignature(lineRootSignature_);

	// ラインのトポロジの設定
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	// Line用パイプラインステートの設定
	commandList_->SetPipelineState(linePipelineState_.Get());

	// ラインのビュープロジェクションのCBVを設定
	world_->GetConstantBuffer(ConstantBufferType::kViewProjection)->BindToGraphics(0, cameraBufferLocationIndex);

	// ラインのSRVを設定
	gpuCbvSrvUavDescriptorHeap_->BindToGraphics(1, world_->GetStructuredBufferHandle(StructuredBufferType::kLine));

	// ラインの描画
	uint32_t instanceCount = debugRenderer_->GetLineCount();
	if (instanceCount) {
		commandList_->DrawInstanced(2, instanceCount, 0, 0);
	}
#endif // DRAW_LINE
}