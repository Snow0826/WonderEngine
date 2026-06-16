#define NOMINMAX
#include "Renderer.h"
#include "DebugRenderer.h"
#include "Resource.h"
#include "PipelineState.h"
#include "Texture.h"
#include "Object.h"
#include "Model.h"
#include "Sprite.h"
#include "Particle.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "Skybox.h"
#include "FootprintMap.h"
#include "EntityComponentSystem.h"
#include "Device.h"
#include "World.h"
#include "ConstantBuffer.h"
#include "IndirectCommand.h"
#include "Footprint.h"
#include "ImGuiManager.h"
#include "Logger.h"
#include "StringConverter.h"
#include <pix3.h>

using namespace StringConverter;

namespace {
	// メッシュタイプ名リスト
	std::array<std::string, static_cast<uint32_t>(MeshType::kCountOfMeshType)> meshTypeNames = {
		"Model",
		"Plane",
		"Box",
		"Ring",
		"Cylinder"
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
	, instance3dRootSignature_(device->GetInstance3dRootSignature())
	, ringInstance3dRootSignature_(device->GetRingInstance3dRootSignature())
	, lineRootSignature_(device->GetLineRootSignature())
	, skyboxRootSignature_(device->GetSkyboxRootSignature())
	, fullscreenRootSignature_(device->GetFullscreenRootSignature())
	, grayscaleRootSignature_(device->GetGrayscaleRootSignature())
	, vignetteRootSignature_(device->GetVignetteRootSignature())
	, boxFilterRootSignature_(device->GetBoxFilterRootSignature())
	, gaussianFilterRootSignature_(device->GetGaussianFilterRootSignature())
	, luminanceBasedOutlineRootSignature_(device->GetLuminanceBasedOutlineRootSignature())
	, depthBasedOutlineRootSignature_(device->GetDepthBasedOutlineRootSignature())
	, depthStencilCopyRootSignature_(device->GetDepthStencilCopyRootSignature())
	, generateHiZMipMapRootSignature_(device->GetGenerateHiZMipMapRootSignature())
	, occlusionCullingRootSignature_(device->GetOcclusionCullingRootSignature())
	, footprintRootSignature_(device->GetFootprintRootSignature())
	, footprintMapRootSignature_(device->GetFootprintMapRootSignature()) {
}

void Renderer::Initialize(std::ofstream &logStream) {
	std::array<D3D12_BLEND_DESC, static_cast<uint32_t>(BlendMode::kCountOfBlendMode)> blendDescList{};

	// kBlendModeNoneのBlendDescの設定
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNone)].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// 全ての色要素を書き込む

	// kBlendModeNormalのBlendDescの設定
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNormal)].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// 全ての色要素を書き込む
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNormal)].RenderTarget[0].BlendEnable = true;									// ブレンドを有効化
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNormal)].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;						// ソースのアルファ値を使用
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNormal)].RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;				// デスティネーションのアルファ値を使用
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNormal)].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;							// ブレンド演算は加算
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNormal)].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;						// ソースのアルファ値を使用
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNormal)].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;					// デスティネーションのアルファ値は使用しない
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNormal)].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;					// アルファブレンド演算は加算

	// kBlendModeAdditiveのBlendDescの設定
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeAdditive)].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// 全ての色要素を書き込む
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeAdditive)].RenderTarget[0].BlendEnable = true;										// ブレンドを有効化
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeAdditive)].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;						// ソースのアルファ値を使用
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeAdditive)].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;							// デスティネーションのアルファ値を使用
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeAdditive)].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;							// ブレンド演算は加算
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeAdditive)].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;						// ソースのアルファ値を使用
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeAdditive)].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;						// デスティネーションのアルファ値は使用しない
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeAdditive)].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;						// アルファブレンド演算は加算

	// kBlendModeSubtractiveのBlendDescの設定
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeSubtractive)].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// 全ての色要素を書き込む
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeSubtractive)].RenderTarget[0].BlendEnable = true;										// ブレンドを有効化
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeSubtractive)].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;						// ソースのアルファ値は使用
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeSubtractive)].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;								// デスティネーションのアルファ値を使用
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeSubtractive)].RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;					// ブレンド演算は逆減算
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeSubtractive)].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;							// ソースのアルファ値を使用
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeSubtractive)].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;						// デスティネーションのアルファ値は使用しない
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeSubtractive)].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;						// アルファブレンド演算は加算

	// kBlendModeMultiplicativeのBlendDescの設定
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeMultiplicative)].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// 全ての色要素を書き込む
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeMultiplicative)].RenderTarget[0].BlendEnable = true;									// ブレンドを有効化
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeMultiplicative)].RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;							// ソースのアルファ値は使用しない
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeMultiplicative)].RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;					// デスティネーションのアルファ値を使用
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeMultiplicative)].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;							// ブレンド演算は加算
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeMultiplicative)].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;						// ソースのアルファ値を使用
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeMultiplicative)].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;					// デスティネーションのアルファ値は使用しない
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeMultiplicative)].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;					// アルファブレンド演算は加算

	// kBlendModeScreenのBlendDescの設定
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeScreen)].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// 全ての色要素を書き込む
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeScreen)].RenderTarget[0].BlendEnable = true;									// ブレンドを有効化
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeScreen)].RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;				// ソースのアルファ値を使用
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeScreen)].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;							// デスティネーションのアルファ値を使用
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeScreen)].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;							// ブレンド演算は加算
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeScreen)].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;						// ソースのアルファ値を使用
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeScreen)].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;					// デスティネーションのアルファ値は使用しない
	blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeScreen)].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;					// アルファブレンド演算は加算

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
	Microsoft::WRL::ComPtr<IDxcBlob> instance3dVSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Instance3d.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(instance3dVSBlob);
	Microsoft::WRL::ComPtr<IDxcBlob> instance3dPSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/Instance3d.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(instance3dPSBlob);

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

	// 深度ステンシルテクスチャコピーのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> depthStencilCopyCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/DepthStencilCopy.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(depthStencilCopyCSBlob);

	// HiZミップマップ生成のシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> generateHiZMipMapCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/GenerateHiZMipMap.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(generateHiZMipMapCSBlob);

	// オクルージョンカリングのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> occlusionCullingCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/OcclusionCulling.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(occlusionCullingCSBlob);

	// フットプリントのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> footprintCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/FootprintStamp.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(footprintCSBlob);

	// フットプリントマップのシェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> footprintMapCSBlob = PipelineState::CompileShader(logStream, L"resources/shaders/FootprintMap.CS.hlsl", L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(footprintMapCSBlob);

	// Mesh用パイプラインステートの生成
	for (size_t i = 0; i < static_cast<size_t>(MeshType::kCountOfMeshType); i++) {
		for (size_t j = 0; j < static_cast<size_t>(BlendMode::kCountOfBlendMode); j++) {
			meshPipelineState_[i][j] = PipelineState()
				.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)	// 頂点座標
				.AddInput("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)		// テクスチャ座標
				.AddInput("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)		// 法線ベクトル
				.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)									// RTVのフォーマット
				.SetBlendState(blendDescList[j])														// BlendState
				.SetRasterizer(backCullingRasterizerDesc)												// RasterizerState
				.SetDepthState(writeLessEqualDepthStencilDesc)											// DepthStencilState
				.SetVertexShader(object3dVSBlob->GetBufferPointer(), object3dVSBlob->GetBufferSize())	// 頂点シェーダー
				.SetPixelShader(object3dPSBlob->GetBufferPointer(), object3dPSBlob->GetBufferSize())	// ピクセルシェーダー
				.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)						// プリミティブトポロジー
				.Create(device_->GetDevice(), static_cast<MeshType>(i) == MeshType::kRing ? ringObject3dRootSignature_ : object3dRootSignature_);
			const std::string logMessage = "Create MeshPipelineState : " + blendModeNames[j] + " " + meshTypeNames[i] + "\n";
			Logger::Log(logStream, logMessage);
			meshPipelineState_[i][j]->SetName(ConvertString(blendModeNames[j] + "Blend" + meshTypeNames[i] + "PipelineState").c_str());
		}
	}

	// MeshParticle用パイプラインステートの生成
	for (size_t i = 0; i < static_cast<size_t>(MeshType::kCountOfMeshType); i++) {
		for (size_t j = 0; j < static_cast<size_t>(BlendMode::kCountOfBlendMode); j++) {
			meshParticlePipelineState_[i][j] = PipelineState()
				.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)		// 頂点座標
				.AddInput("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)			// テクスチャ座標
				.AddInput("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)			// 法線ベクトル
				.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
				.SetBlendState(blendDescList[j])															// BlendState
				.SetRasterizer(backCullingRasterizerDesc)													// RasterizerState
				.SetDepthState(noWriteLessEqualDepthStencilDesc)											// DepthStencilState
				.SetVertexShader(instance3dVSBlob->GetBufferPointer(), instance3dVSBlob->GetBufferSize())	// 頂点シェーダー
				.SetPixelShader(instance3dPSBlob->GetBufferPointer(), instance3dPSBlob->GetBufferSize())	// ピクセルシェーダー
				.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
				.Create(device_->GetDevice(), static_cast<MeshType>(i) == MeshType::kRing ? ringInstance3dRootSignature_ : instance3dRootSignature_);
			const std::string logMessage = "Create ParticlePipelineState : " + blendModeNames[j] + " " + meshTypeNames[i] + "\n";
			Logger::Log(logStream, logMessage);
			meshParticlePipelineState_[i][j]->SetName(ConvertString(blendModeNames[j] + "Blend" + meshTypeNames[i] + "ParticlePipelineState").c_str());
		}
	}

	// 各ブレンドモードのSprite用パイプラインステートの生成
	for (uint32_t i = 0; i < static_cast<uint32_t>(BlendMode::kCountOfBlendMode); i++) {
		spritePipelineState_[i] = PipelineState()
			.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)	// 頂点座標
			.AddInput("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)		// テクスチャ座標
			.AddInput("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)		// 法線ベクトル
			.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)									// RTVのフォーマット
			.SetBlendState(blendDescList[i])														// BlendState
			.SetRasterizer(noCullingRasterizerDesc)													// RasterizerState
			.SetDepthState(noWriteLessEqualDepthStencilDesc)										// DepthStencilState
			.SetVertexShader(object3dVSBlob->GetBufferPointer(), object3dVSBlob->GetBufferSize())	// 頂点シェーダー
			.SetPixelShader(object3dPSBlob->GetBufferPointer(), object3dPSBlob->GetBufferSize())	// ピクセルシェーダー
			.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)						// プリミティブトポロジー
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
		.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)	// 頂点座標
		.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)									// RTVのフォーマット
		.SetBlendState(blendDescList[static_cast<uint32_t>(BlendMode::kBlendModeNone)])			// BlendState
		.SetRasterizer(noCullingRasterizerDesc)													// RasterizerState
		.SetDepthState(noWriteLessEqualDepthStencilDesc)										// DepthStencilState
		.SetVertexShader(skyboxVSBlob->GetBufferPointer(), skyboxVSBlob->GetBufferSize())		// 頂点シェーダー
		.SetPixelShader(skyboxPSBlob->GetBufferPointer(), skyboxPSBlob->GetBufferSize())		// ピクセルシェーダー
		.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)						// プリミティブトポロジー
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

	// オクルージョンカリング用パイプラインステートの生成
	occlusionCullingPipelineState_ = PipelineState()
		.SetComputeShader(occlusionCullingCSBlob->GetBufferPointer(), occlusionCullingCSBlob->GetBufferSize())	// コンピュートシェーダー
		.Create(device_->GetDevice(), occlusionCullingRootSignature_);
	Logger::Log(logStream, "Create OcclusionCullingPipelineState\n");
	occlusionCullingPipelineState_->SetName(L"OcclusionCullingPipelineState");

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
	D3D12_INDIRECT_ARGUMENT_DESC argumentDescList[6] = {};
	argumentDescList[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
	argumentDescList[0].ConstantBufferView.RootParameterIndex = 0;
	argumentDescList[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
	argumentDescList[1].ConstantBufferView.RootParameterIndex = 2;
	argumentDescList[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
	argumentDescList[2].Constant.RootParameterIndex = 5;
	argumentDescList[2].Constant.DestOffsetIn32BitValues = 0;
	argumentDescList[2].Constant.Num32BitValuesToSet = 2;
	argumentDescList[3].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
	argumentDescList[3].VertexBuffer.Slot = 0;
	argumentDescList[4].Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
	argumentDescList[5].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	// コマンドシグネチャの設定
	D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
	commandSignatureDesc.pArgumentDescs = argumentDescList;
	commandSignatureDesc.NumArgumentDescs = _countof(argumentDescList);
	commandSignatureDesc.ByteStride = sizeof(IndirectCommand);

	// コマンドシグネチャの生成
	for (size_t i = 0; i < static_cast<size_t>(MeshType::kCountOfMeshType); i++) {
		hr = device_->GetDevice()->CreateCommandSignature(&commandSignatureDesc, static_cast<MeshType>(i) == MeshType::kRing ? ringObject3dRootSignature_ : object3dRootSignature_, IID_PPV_ARGS(&meshCommandSignature_[i]));
		assert(SUCCEEDED(hr));
	}
}

void Renderer::Render() {
	// オクルージョンカリングの実行
	CopyDepthToHiZ();
	GenerateHiZMipMap();
	OcclusionCulling();

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

void Renderer::SetIndirectCommandManager(IndirectCommandManager *indirectCommandManager) {
	assert(indirectCommandManager);
	indirectCommandManager_ = indirectCommandManager;
}

void Renderer::SetMeshManager(MeshManager *meshManager) {
	assert(meshManager);
	meshManager_ = meshManager;
}

void Renderer::SetTextureManager(TextureManager *textureManager) {
	assert(textureManager);
	textureManager_ = textureManager;
}

void Renderer::SetFootprintManager(FootprintManager *footprintManager) {
	assert(footprintManager);
	footprintManager_ = footprintManager;
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

void Renderer::OcclusionCulling() {
	// オクルージョンカリング用ルートシグネチャとパイプラインステートの設定
	commandList_->SetComputeRootSignature(occlusionCullingRootSignature_);
	commandList_->SetPipelineState(occlusionCullingPipelineState_.Get());

	// CBVの設定
	world_->GetConstantBuffer(ConstantBufferType::kFrustum)->BindToCompute(0, 0);
	world_->GetConstantBuffer(ConstantBufferType::kViewProjection)->BindToCompute(1, 1);
	world_->GetConstantBuffer(ConstantBufferType::kCameraPosition)->BindToCompute(2, 0);

	// メッシュ数の設定
	CullingConstantsData cullingConstantsData = {
		.meshCount = indirectCommandManager_->GetMeshCounter(),
		.queueOffsets = world_->GetQueueOffsets()
	};
	commandList_->SetComputeRoot32BitConstants(3, sizeof(CullingConstantsData) / sizeof(uint32_t), &cullingConstantsData, 0);

	// 各種バッファのSRV/UAVを設定
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(4, world_->GetCullingObjectHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(5, world_->GetCullingMeshHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(6, world_->GetMeshLODHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(7, world_->GetHiZTextureSRVHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(8, world_->GetProcessedCommandHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(9, world_->GetCommandCounterHandle());

	// コマンドバッファの転送
	Resource *indirectCommandStructuredBuffer = world_->GetStructuredBuffer(StructuredBufferType::kMeshLOD);
	indirectCommandStructuredBuffer->TransitionBarrier(D3D12_RESOURCE_STATE_COPY_DEST);
	indirectCommandStructuredBuffer->CopyFrom(world_->GetCommandBufferUpload()->GetResource(), 0, 0, sizeof(MeshLOD) * world_->GetMaxAABB());
	indirectCommandStructuredBuffer->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	// オクルージョンカリングの実行前にUAVを遷移する
	world_->GetProcessedCommandBuffer()->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	world_->GetCommandCounterBuffer()->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// オクルージョンカリングの実行前にUAVをクリアする
	UINT clearValues[4] = { 0, 0, 0, 0 };
	commandList_->ClearUnorderedAccessViewUint(
		gpuCbvSrvUavDescriptorHeap_->GetGPUDescriptorHandle(world_->GetCommandCounterHandle()),
		cpuCbvSrvUavDescriptorHeap_->GetCPUDescriptorHandle(world_->GetCommandCounterHandle()),
		world_->GetCommandCounterBuffer()->GetResource(),
		clearValues,
		0,
		nullptr
	);

	// オクルージョンカリングの実行
	uint32_t dispatchCount = (cullingConstantsData.meshCount + 63) / 64;
	if (dispatchCount > 0) {
		commandList_->Dispatch(dispatchCount, 1, 1);
		world_->GetHiZTexture()->UAVBarrier();
		world_->GetProcessedCommandBuffer()->UAVBarrier();
		world_->GetCommandCounterBuffer()->UAVBarrier();
	}
}

void Renderer::Footprint() {
	// フットプリント用ルートシグネチャとパイプラインステートの設定
	commandList_->SetComputeRootSignature(footprintRootSignature_);
	commandList_->SetPipelineState(footprintPipelineState_.Get());

	// 各種バッファのSRV/UAVを設定
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(1, world_->GetFootprintHandle());
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

	// コマンドバッファとコマンドカウンターバッファをIndirectArgumentに遷移
	world_->GetProcessedCommandBuffer()->TransitionBarrier(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	world_->GetCommandCounterBuffer()->TransitionBarrier(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	// PIXイベントの色の設定
	UINT32 pixColor[static_cast<uint32_t>(MeshType::kCountOfMeshType)] = {
		PIX_COLOR(255, 0, 0),	// MeshType::kModel
		PIX_COLOR(255, 255, 0),	// MeshType::kPlane
		PIX_COLOR(255, 0, 255),	// MeshType::kBox
		PIX_COLOR(0, 255, 0),	// MeshType::kRing
		PIX_COLOR(0, 255, 255)	// MeshType::kCylinder
	};

	// メッシュタイプごとに描画
	for (uint32_t i = 0; i < static_cast<uint32_t>(MeshType::kCountOfMeshType); i++) {
		std::string label = "Draw" + meshTypeNames[i];
		PIXBeginEvent(commandList_, pixColor[i], ConvertString(label).c_str());
		commandList_->SetGraphicsRootSignature(static_cast<MeshType>(i) == MeshType::kRing ? ringObject3dRootSignature_ : object3dRootSignature_);

		// メッシュの共通のCBVを設定
		world_->GetConstantBuffer(ConstantBufferType::kViewProjection)->BindToGraphics(1, cameraBufferLocationIndex);
		world_->GetConstantBuffer(ConstantBufferType::kCameraPosition)->BindToGraphics(3, 0);
		world_->GetConstantBuffer(ConstantBufferType::kDirectionalLight)->BindToGraphics(4, 0);
		LightCount lightCount = {
			.pointLightCount = static_cast<uint32_t>(registry_->GetComponentCount<PointLight>()),
			.spotLightCount = static_cast<uint32_t>(registry_->GetComponentCount<SpotLight>())
		};
		commandList_->SetGraphicsRoot32BitConstants(6, 2, &lightCount, 0);

		// メッシュの共通のSRVを設定
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(7, world_->GetPointLightHandle());
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(8, world_->GetSpotLightHandle());
		registry_->ForEach<Skybox>([&](uint32_t entity, Skybox *skybox) {
			gpuCbvSrvUavDescriptorHeap_->BindToGraphics(9, skybox->textureHandle);
			}, exclude<Disabled>());
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(10, 0);

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
	// PIXイベントの色の設定
	UINT32 pixColor[static_cast<uint32_t>(MeshType::kCountOfMeshType)] = {
		PIX_COLOR(255, 0, 0),	// MeshType::kModel
		PIX_COLOR(255, 255, 0),	// MeshType::kPlane
		PIX_COLOR(255, 0, 255),	// MeshType::kBox
		PIX_COLOR(0, 255, 0),	// MeshType::kRing
		PIX_COLOR(0, 255, 255)	// MeshType::kCylinder
	};

	// メッシュタイプごとに描画
	for (size_t i = 0; i < static_cast<uint32_t>(MeshType::kCountOfMeshType); i++) {
		std::string label = "Draw" + meshTypeNames[i] + "Particle";
		PIXBeginEvent(commandList_, pixColor[i], ConvertString(label).c_str());

		// Instance3d用ルートシグネチャの設定
		commandList_->SetGraphicsRootSignature(static_cast<MeshType>(i) == MeshType::kRing ? ringInstance3dRootSignature_ : instance3dRootSignature_);

		// パーティクルのビュープロジェクションのCBVを設定
		world_->GetConstantBuffer(ConstantBufferType::kViewProjection)->BindToGraphics(0, cameraBufferLocationIndex);

		// パーティクルのテクスチャのSRVを設定
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(3, 0);

		// ブレンドモードごとに描画
		for (size_t j = 0; j < static_cast<uint32_t>(BlendMode::kCountOfBlendMode); j++) {
			label = blendModeNames[j] + "Blend";
			PIXBeginEvent(commandList_, PIX_COLOR(255, 255, 255), ConvertString(label).c_str());
			// MeshParticle用パイプラインステートの設定
			commandList_->SetPipelineState(meshParticlePipelineState_[i][j].Get());

			// MeshParticleの描画
			registry_->ForEach<BlendMode, ParticleGroup>([&](uint32_t entity, BlendMode *blendMode, ParticleGroup *particleGroup) {
				if (particleGroup->meshType == static_cast<MeshType>(i)) {
					if (*blendMode == static_cast<BlendMode>(j)) {
						commandList_->SetGraphicsRoot32BitConstant(1, particleGroup->textureHandle, 0);
						gpuCbvSrvUavDescriptorHeap_->BindToGraphics(2, particleGroup->instanceHandle);
						meshManager_->Draw(particleGroup->meshHandle, particleGroup->numInstance);
					}
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

	// スプライトのビュープロジェクションのCBVを設定
	world_->GetConstantBuffer(ConstantBufferType::kViewProjection)->BindToGraphics(1, 0);

	// Object3dのテクスチャのSRVを設定
	registry_->ForEach<Skybox>([&](uint32_t entity, Skybox *skybox) {
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(9, skybox->textureHandle);
		}, exclude<Disabled>());
	gpuCbvSrvUavDescriptorHeap_->BindToGraphics(10, 0);

	// 各ブレンドモードのスプライトの描画
	for (uint32_t i = 0; i < static_cast<uint32_t>(BlendMode::kCountOfBlendMode); i++) {
		// Sprite用パイプラインステートの設定
		commandList_->SetPipelineState(spritePipelineState_[i].Get());

		// スプライトの描画
		registry_->ForEach<BlendMode, Sprite, Object>([&](uint32_t entity, BlendMode *blendMode, Sprite *sprite, Object *object) {
			if (*blendMode == static_cast<BlendMode>(i)) {
				world_->GetConstantBuffer(ConstantBufferType::kTransform)->BindToGraphics(0, object->handle);
				world_->GetConstantBuffer(ConstantBufferType::kMaterial)->BindToGraphics(2, object->handle);
				TextureData textureData = {
					.textureHandle = sprite->textureHandle,
					.enableMipMaps = sprite->enableMipMaps
				};
				commandList_->SetGraphicsRoot32BitConstants(5, 2, &textureData, 0);
				meshManager_->Draw(sprite->meshHandle);
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
	registry_->ForEach<Skybox, Object>([&](uint32_t entity, Skybox *skybox, Object *object) {
		world_->GetConstantBuffer(ConstantBufferType::kTransform)->BindToGraphics(0, object->handle);
		world_->GetConstantBuffer(ConstantBufferType::kMaterial)->BindToGraphics(2, object->handle);
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(3, skybox->textureHandle);
		meshManager_->Draw(skybox->meshHandle);
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
	gpuCbvSrvUavDescriptorHeap_->BindToGraphics(1, world_->GetLineHandle());

	// ラインの描画
	uint32_t instanceCount = debugRenderer_->GetLineCount();
	if (instanceCount) {
		commandList_->DrawInstanced(2, instanceCount, 0, 0);
	}
#endif // DRAW_LINE
}