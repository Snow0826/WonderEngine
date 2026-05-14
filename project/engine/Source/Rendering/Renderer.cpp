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

Renderer::Renderer(Device *device)
	: device_(device)
	, gpuCbvSrvUavDescriptorHeap_(device->GetGpuCbvSrvUavDescriptorHeap())
	, cpuCbvSrvUavDescriptorHeap_(device->GetCpuCbvSrvUavDescriptorHeap())
	, commandList_(device->GetCommandList())
	, object3dRootSignature_(device->GetObject3dRootSignature())
	, instance3dRootSignature_(device->GetInstance3dRootSignature())
	, lineRootSignature_(device->GetLineRootSignature())
	, skyboxRootSignature_(device->GetSkyboxRootSignature())
	, depthStencilCopyRootSignature_(device->GetDepthStencilCopyRootSignature())
	, generateHiZMipMapRootSignature_(device->GetGenerateHiZMipMapRootSignature())
	, occlusionCullingRootSignature_(device->GetOcclusionCullingRootSignature())
	, footprintRootSignature_(device->GetFootprintRootSignature())
	, footprintMapRootSignature_(device->GetFootprintMapRootSignature()) {}

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

	// ブレンドモード名リスト
	std::array<std::string, static_cast<uint32_t>(BlendMode::kCountOfBlendMode)> blendModeNames = {
		"None",
		"Normal",
		"Additive",
		"Subtractive",
		"Multiplicative",
		"Screen"
	};

	// 各ブレンドモードのModel用パイプラインステートの生成
	for (uint32_t i = 0; i < static_cast<uint32_t>(BlendMode::kCountOfBlendMode); i++) {
		modelPipelineState_[i] = PipelineState()
			.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)	// 頂点座標
			.AddInput("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)		// テクスチャ座標
			.AddInput("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)		// 法線ベクトル
			.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)									// RTVのフォーマット
			.SetBlendState(blendDescList[i])														// BlendState
			.SetRasterizer(backCullingRasterizerDesc)												// RasterizerState
			.SetDepthState(writeLessEqualDepthStencilDesc)											// DepthStencilState
			.SetVertexShader(object3dVSBlob->GetBufferPointer(), object3dVSBlob->GetBufferSize())	// 頂点シェーダー
			.SetPixelShader(object3dPSBlob->GetBufferPointer(), object3dPSBlob->GetBufferSize())	// ピクセルシェーダー
			.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)						// プリミティブトポロジー
			.Create(device_->GetDevice(), object3dRootSignature_);
		const std::string logMessage = "Create ModelPipelineState : " + blendModeNames[i] + "\n";
		Logger::Log(logStream, logMessage);
		modelPipelineState_[i]->SetName(StringConverter::ConvertString(blendModeNames[i] + "BlendModelPipelineState").c_str());
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
		spritePipelineState_[i]->SetName(StringConverter::ConvertString(blendModeNames[i] + "BlendSpritePipelineState").c_str());
	}

	// 各ブレンドモードのParticle用パイプラインステートの生成
	for (uint32_t i = 0; i < static_cast<uint32_t>(BlendMode::kCountOfBlendMode); i++) {
		particlePipelineState_[i] = PipelineState()
			.AddInput("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)		// 頂点座標
			.AddInput("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)			// テクスチャ座標
			.AddInput("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)			// 法線ベクトル
			.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)										// RTVのフォーマット
			.SetBlendState(blendDescList[i])															// BlendState
			.SetRasterizer(backCullingRasterizerDesc)													// RasterizerState
			.SetDepthState(noWriteLessEqualDepthStencilDesc)											// DepthStencilState
			.SetVertexShader(instance3dVSBlob->GetBufferPointer(), instance3dVSBlob->GetBufferSize())	// 頂点シェーダー
			.SetPixelShader(instance3dPSBlob->GetBufferPointer(), instance3dPSBlob->GetBufferSize())	// ピクセルシェーダー
			.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)							// プリミティブトポロジー
			.Create(device_->GetDevice(), instance3dRootSignature_);
		const std::string logMessage = "Create ParticlePipelineState : " + blendModeNames[i] + "\n";
		Logger::Log(logStream, logMessage);
		particlePipelineState_[i]->SetName(StringConverter::ConvertString(blendModeNames[i] + "BlendParticlePipelineState").c_str());
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
	hr = device_->GetDevice()->CreateCommandSignature(&commandSignatureDesc, object3dRootSignature_, IID_PPV_ARGS(&commandSignature_));
	assert(SUCCEEDED(hr));
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

	// 各オブジェクトの描画
	PreDrawModel();
	DrawModel();
	PreDrawParticle();
	DrawParticle();
	PreDrawSprite();
	DrawSprite();
	PreDrawLine();
	DrawLine();
	PreDrawSkybox();
	DrawSkybox();
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

bool Renderer::IsDebugCamera() {
	bool isDebugCamera = false;
	registry_->ForEach<Camera, Transform, RenderingCamera>(
		[&](uint32_t entity, Camera *, Transform *, RenderingCamera *) {
			isDebugCamera = true;
		},
		exclude<Disabled, CullingCamera>()
	);
	return isDebugCamera;
}

void Renderer::CopyDepthToHiZ() {
	// 深度ステンシルテクスチャコピー用ルートシグネチャとパイプラインステートの設定
	commandList_->SetComputeRootSignature(depthStencilCopyRootSignature_);
	commandList_->SetPipelineState(depthStencilCopyPipelineState_.Get());

	Resource *depthStencilTexture = device_->GetPreviousDepthStencilTexture();
	Resource *hiZTexture = world_->GetHiZTexture();
	depthStencilTexture->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	hiZTexture->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0);

	// 各種バッファのSRV/UAVを設定
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(0, world_->GetDepthStencilCopySourceHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(1, world_->GetDepthStencilCopyDestHandle());

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
	ConstantBuffer *frustumCB = world_->GetConstantBuffer(ConstantBufferType::kFrustum);
	if (IsDebugCamera()) {
		frustumCB->BindToCompute(0, 1);
	} else {
		frustumCB->BindToCompute(0, 0);
	}

	ConstantBuffer *viewProjCB = world_->GetConstantBuffer(ConstantBufferType::kViewProjection);
	if (IsDebugCamera()) {
		viewProjCB->BindToCompute(1, 2);
	} else {
		viewProjCB->BindToCompute(1, 1);
	}

	ConstantBuffer *cameraCB = world_->GetConstantBuffer(ConstantBufferType::kCamera);
	if (IsDebugCamera()) {
		cameraCB->BindToCompute(2, 1);
	} else {
		cameraCB->BindToCompute(2, 0);
	}

	// メッシュ数の設定
	uint32_t meshCount = indirectCommandManager_->GetMeshCounter();
	commandList_->SetComputeRoot32BitConstant(3, meshCount, 0);

	// 各種バッファのSRV/UAVを設定
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(4, world_->GetCullingObjectHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(5, world_->GetCullingMeshHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(6, world_->GetMeshLODHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToCompute(7, world_->GetHiZTextureHandle());
	for (uint32_t i = 0; i < static_cast<uint32_t>(BlendMode::kCountOfBlendMode); i++) {
		gpuCbvSrvUavDescriptorHeap_->BindToCompute(8 + i, world_->GetBlendProcessedIndirectCommandHandle(static_cast<BlendMode>(i)));
	}

	// コマンドバッファの転送
	Resource *indirectCommandStructuredBuffer = world_->GetStructuredBuffer(StructuredBufferType::kMeshLOD);
	indirectCommandStructuredBuffer->TransitionBarrier(D3D12_RESOURCE_STATE_COPY_DEST);
	indirectCommandStructuredBuffer->CopyFrom(world_->GetCommandBufferUpload()->GetResource(), 0, 0, sizeof(MeshLOD) * world_->GetMaxAABB());
	indirectCommandStructuredBuffer->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	// 処理済みコマンドバッファカウンターのリセット
	for (size_t i = 0; i < static_cast<size_t>(BlendMode::kCountOfBlendMode); i++) {
		Resource *blendProcessedCommandBuffer = world_->GetBlendProcessedCommandBuffer(static_cast<BlendMode>(i));
		blendProcessedCommandBuffer->TransitionBarrier(D3D12_RESOURCE_STATE_COPY_DEST);
		blendProcessedCommandBuffer->CopyFrom(world_->GetProcessedCommandBufferCounterReset()->GetResource(), World::kCommandBufferCounterOffset, 0, sizeof(UINT));
		blendProcessedCommandBuffer->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}

	// オクルージョンカリングの実行
	uint32_t dispatchCount = (meshCount + 63) / 64;
	if (dispatchCount > 0) {
		commandList_->Dispatch(dispatchCount, 1, 1);
		world_->GetHiZTexture()->UAVBarrier();
	}

	// 指定した深度で画面全体をクリアする
	commandList_->ClearDepthStencilView(device_->GetDSVHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
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

void Renderer::PreDrawModel() {
	// Object3d用ルートシグネチャの設定
	commandList_->SetGraphicsRootSignature(object3dRootSignature_);

	// 三角形のトポロジの設定
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// モデルのビュープロジェクションのCBVを設定
	ConstantBuffer *viewProjCB = world_->GetConstantBuffer(ConstantBufferType::kViewProjection);
	if (IsDebugCamera()) {
		viewProjCB->BindToGraphics(1, 2);
	} else {
		viewProjCB->BindToGraphics(1, 1);
	}

	// Object3dの共通のCBVを設定
	world_->GetConstantBuffer(ConstantBufferType::kCamera)->BindToGraphics(3, 0);
	world_->GetConstantBuffer(ConstantBufferType::kDirectionalLight)->BindToGraphics(4, 0);
	LightCount lightCount = {
		.pointLightCount = static_cast<uint32_t>(registry_->GetComponentCount<PointLight>()),
		.spotLightCount = static_cast<uint32_t>(registry_->GetComponentCount<SpotLight>())
	};
	commandList_->SetGraphicsRoot32BitConstants(6, 2, &lightCount, 0);

	// Object3dの共通のSRVを設定
	gpuCbvSrvUavDescriptorHeap_->BindToGraphics(7, world_->GetPointLightHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToGraphics(8, world_->GetSpotLightHandle());
	gpuCbvSrvUavDescriptorHeap_->BindToGraphics(9, 0);
}

void Renderer::DrawModel() {
	for (uint32_t i = 0; i < static_cast<uint32_t>(BlendMode::kCountOfBlendMode); i++) {
		// Model用パイプラインステートの設定
		commandList_->SetPipelineState(modelPipelineState_[i].Get());

		world_->GetBlendProcessedCommandBuffer(static_cast<BlendMode>(i))->TransitionBarrier(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

		commandList_->ExecuteIndirect(
			commandSignature_.Get(),
			world_->GetMaxAABB(),
			world_->GetBlendProcessedCommandBuffer(static_cast<BlendMode>(i))->GetResource(),
			0,
			world_->GetBlendProcessedCommandBuffer(static_cast<BlendMode>(i))->GetResource(),
			World::kCommandBufferCounterOffset);
	}
}

void Renderer::PreDrawParticle() {
	// Instance3d用ルートシグネチャの設定
	commandList_->SetGraphicsRootSignature(instance3dRootSignature_);

	// パーティクルのビュープロジェクションのCBVを設定
	ConstantBuffer *viewProjCB = world_->GetConstantBuffer(ConstantBufferType::kViewProjection);
	if (IsDebugCamera()) {
		viewProjCB->BindToGraphics(0, 2);
	} else {
		viewProjCB->BindToGraphics(0, 1);
	}

	// Instance3dのテクスチャのSRVを設定
	gpuCbvSrvUavDescriptorHeap_->BindToGraphics(3, 0);
}

void Renderer::DrawParticle() {
	// 各ブレンドモードのパーティクルの描画
	for (uint32_t i = 0; i < static_cast<uint32_t>(BlendMode::kCountOfBlendMode); i++) {
		// Particle用パイプラインステートの設定
		commandList_->SetPipelineState(particlePipelineState_[i].Get());

		// パーティクルの描画
		registry_->ForEach<BlendMode, ParticleGroup>([&](uint32_t entity, BlendMode *blendMode, ParticleGroup *particleGroup) {
			if (*blendMode == static_cast<BlendMode>(i)) {
				commandList_->SetGraphicsRoot32BitConstant(1, particleGroup->textureHandle, 0);
				gpuCbvSrvUavDescriptorHeap_->BindToGraphics(2, particleGroup->instanceHandle);
				meshManager_->Draw(particleGroup->meshHandle, particleGroup->numInstance);
			}
			}, exclude<Disabled>());
	}
}

void Renderer::PreDrawSprite() {
	// Object3d用ルートシグネチャの設定
	commandList_->SetGraphicsRootSignature(object3dRootSignature_);

	// スプライトのビュープロジェクションのCBVを設定
	world_->GetConstantBuffer(ConstantBufferType::kViewProjection)->BindToGraphics(1, 0);

	// Object3dのテクスチャのSRVを設定
	gpuCbvSrvUavDescriptorHeap_->BindToGraphics(9, 0);
}

void Renderer::DrawSprite() {
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

void Renderer::PreDrawLine() {
#ifdef DRAW_LINE
	// Line用ルートシグネチャの設定
	commandList_->SetGraphicsRootSignature(lineRootSignature_);

	// ラインのトポロジの設定
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	// Line用パイプラインステートの設定
	commandList_->SetPipelineState(linePipelineState_.Get());

	// ラインのビュープロジェクションのCBVを設定
	ConstantBuffer *viewProjCB = world_->GetConstantBuffer(ConstantBufferType::kViewProjection);
	if (IsDebugCamera()) {
		viewProjCB->BindToGraphics(0, 2);
	} else {
		viewProjCB->BindToGraphics(0, 1);
	}

	// ラインのSRVを設定
	gpuCbvSrvUavDescriptorHeap_->BindToGraphics(1, world_->GetLineHandle());
#endif // DRAW_LINE
}

void Renderer::DrawLine() {
#ifdef DRAW_LINE
	uint32_t instanceCount = debugRenderer_->GetLineCount();
	if (instanceCount) {
		commandList_->DrawInstanced(2, instanceCount, 0, 0);
	}
#endif // DRAW_LINE
}

void Renderer::PreDrawSkybox() {
	// Skybox用ルートシグネチャの設定
	commandList_->SetGraphicsRootSignature(skyboxRootSignature_);

	// 三角形のトポロジの設定
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Skybox用パイプラインステートの設定
	commandList_->SetPipelineState(skyboxPipelineState_.Get());

	// スカイボックスのCBVを設定
	ConstantBuffer *viewProjCB = world_->GetConstantBuffer(ConstantBufferType::kViewProjection);
	if (IsDebugCamera()) {
		viewProjCB->BindToGraphics(1, 2);
	} else {
		viewProjCB->BindToGraphics(1, 1);
	}
}

void Renderer::DrawSkybox() {
	registry_->ForEach<Skybox, Object>([&](uint32_t entity, Skybox *skybox, Object *object) {
		world_->GetConstantBuffer(ConstantBufferType::kTransform)->BindToGraphics(0, object->handle);
		world_->GetConstantBuffer(ConstantBufferType::kMaterial)->BindToGraphics(2, object->handle);
		gpuCbvSrvUavDescriptorHeap_->BindToGraphics(3, skybox->textureHandle);
		meshManager_->Draw(skybox->meshHandle);
		}, exclude<Disabled>());
}
