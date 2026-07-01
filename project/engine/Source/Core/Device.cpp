#include "Device.h"
#include "Window.h"
#include "ImGuiManager.h"
#include "Logger.h"
#include "StringConverter.h"
#include "Resource.h"
#include "RootSignature.h"
#include <format>
#include <thread>

Device::Device() = default;
Device::~Device() = default;

void Device::Initialize(std::ofstream &logStream, const Window &window) {
	// FPS固定の初期化
	InitializeFixFPS();

	HWND hwnd = window.GetHandle();				// ウィンドウハンドル
	uint32_t width = window.GetClientWidth();	// ウィンドウの幅
	uint32_t height = window.GetClientHeight();	// ウィンドウの高さ

#ifdef _DEBUG
    EnableDebugLayer();
#endif	// _DEBUG

	// DXGIファクトリの作成
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	assert(SUCCEEDED(hr));

	// アダプタの列挙
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter_)) != DXGI_ERROR_NOT_FOUND; ++i) {
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = adapter_->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			Logger::Log(logStream, StringConverter::ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		adapter_ = nullptr;
	}
	assert(adapter_ != nullptr);

	// 機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
	};

	// 機能レベルの文字列
	const char *featureLevelStrings[] = {
		"12.2",
		"12.1",
		"12.0",
	};

	// デバイスの作成
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		hr = D3D12CreateDevice(adapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
		if (SUCCEEDED(hr)) {
			Logger::Log(logStream, std::format("FeatureLevel:{}\n", featureLevelStrings[i]));
			break;
		}
	}

	assert(device_ != nullptr);
	Logger::Log(logStream, "Complete create D3D12Device!!!\n");

#ifdef _DEBUG
	// デバッグコントローラーの設定
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue_)))) {

		infoQueue_->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue_->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		infoQueue_->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
			D3D12_MESSAGE_ID_GPU_BASED_VALIDATION_RESOURCE_STATE_IMPRECISE
		};

		D3D12_MESSAGE_SEVERITY severities[] = {
			D3D12_MESSAGE_SEVERITY_INFO
		};

		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		infoQueue_->PushStorageFilter(&filter);

	}
#endif	// _DEBUG

	// コマンドキューの作成
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
	assert(SUCCEEDED(hr));

	// コマンドアロケータの作成
	hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	assert(SUCCEEDED(hr));

	// コマンドリストの作成
	hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
	assert(SUCCEEDED(hr));

	// フェンスの作成
	hr = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));

	// フェンスを待つイベントの作成
	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent_ != nullptr);

	// スワップチェーンの作成
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1 **>(swapChain_.GetAddressOf()));
	assert(SUCCEEDED(hr));

	// ディスクリプタヒープの作成
	rtvDescriptorHeap_ = DescriptorHeap::Create(this, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 5, false);	// RTV用のディスクリプタヒープの作成
	gpuCbvSrvUavDescriptorHeap_ = DescriptorHeap::Create(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, DescriptorHeap::kMaxSRVCount, true);		// GPU用のCBV,SRV,UAV用のディスクリプタヒープの作成
	cpuCbvSrvUavDescriptorHeap_ = DescriptorHeap::Create(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, DescriptorHeap::kMaxSRVCount, false);	// CPU用のCBV,SRV,UAV用のディスクリプタヒープの作成
	dsvDescriptorHeap_ = DescriptorHeap::Create(this, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 2, false);	// DSV用のディスクリプタヒープの作成

	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// RTVの作成
	for (UINT i = 0; i < 2; ++i) {
		Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResource;
		hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainResource));
		assert(SUCCEEDED(hr));
		uint32_t rtvHandle = rtvDescriptorHeap_.AllocateDescriptor();
		rtvDescriptorHeap_.CreateRenderTargetView(swapChainResource, rtvDesc, rtvHandle);
		Logger::Log(logStream, "SwapChain " + std::to_string(i) + " RTVDescriptorIndex: " + std::to_string(rtvHandle) + "\n");
		swapChainResources_.emplace_back(swapChainResource);
		swapChainResourceRTVHandles_.emplace_back(rtvHandle);
	}

	// Object3d用ルートシグネチャの作成
	object3dRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_VERTEX, 0)																			// 0:Transform
		.AddCBuffer(D3D12_SHADER_VISIBILITY_VERTEX, 1)																			// 1:ViewProjection
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 0)																			// 2:Material
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 1)																			// 3:Camera
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 2)																			// 4:DirectionalLight
		.Add32BitConstant(D3D12_SHADER_VISIBILITY_PIXEL, 3, 2)																	// 5:TextureData
		.Add32BitConstant(D3D12_SHADER_VISIBILITY_PIXEL, 4, 2)																	// 6:LightData
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 0)								// 7:PointLight
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 1)								// 8:SpotLight
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 2)								// 9:EnvironmentTexture
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, DescriptorHeap::kMaxSRVCount, D3D12_SHADER_VISIBILITY_PIXEL, 3)	// 10:Texture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// Samplerを追加
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL)	// MipMap0のSamplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create Object3dRootSignature\n");
	object3dRootSignature_->SetName(L"Object3dRootSignature");

	// RingObject3d用ルートシグネチャの作成
	ringObject3dRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_VERTEX, 0)																			// 0:Transform
		.AddCBuffer(D3D12_SHADER_VISIBILITY_VERTEX, 1)																			// 1:ViewProjection
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 0)																			// 2:Material
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 1)																			// 3:Camera
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 2)																			// 4:DirectionalLight
		.Add32BitConstant(D3D12_SHADER_VISIBILITY_PIXEL, 3, 2)																	// 5:TextureData
		.Add32BitConstant(D3D12_SHADER_VISIBILITY_PIXEL, 4, 2)																	// 6:LightData
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 0)								// 7:PointLight
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 1)								// 8:SpotLight
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 2)								// 9:EnvironmentTexture
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, DescriptorHeap::kMaxSRVCount, D3D12_SHADER_VISIBILITY_PIXEL, 3)	// 10:Texture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// Samplerを追加
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL)	// MipMap0のSamplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create RingObject3dRootSignature\n");
	ringObject3dRootSignature_->SetName(L"RingObject3dRootSignature");

	// SkinningObject3d用ルートシグネチャの作成
	skinningObject3dRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_VERTEX, 0)																				// 0:Transform
		.AddCBuffer(D3D12_SHADER_VISIBILITY_VERTEX, 1)																				// 1:ViewProjection
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 0)																				// 2:Material
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 1)																				// 3:Camera
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 2)																				// 4:DirectionalLight
		.Add32BitConstant(D3D12_SHADER_VISIBILITY_VERTEX, 2, 1)																		// 5:MatrixPaletteData
		.Add32BitConstant(D3D12_SHADER_VISIBILITY_PIXEL, 3, 2)																		// 6:TextureData
		.Add32BitConstant(D3D12_SHADER_VISIBILITY_PIXEL, 4, 2)																		// 7:LightData
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, DescriptorHeap::kMaxSRVCount, D3D12_SHADER_VISIBILITY_VERTEX, 0)		// 8:DescriptorTable
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 0)									// 9:PointLight
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 1)									// 10:SpotLight
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 2)									// 11:EnvironmentTexture
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, DescriptorHeap::kMaxSRVCount, D3D12_SHADER_VISIBILITY_PIXEL, 3)		// 12:Texture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// Samplerを追加
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL)	// MipMap0のSamplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create SkinningObject3dRootSignature\n");
	skinningObject3dRootSignature_->SetName(L"SkinningObject3dRootSignature");

	// Instance3d用ルートシグネチャの作成
	instance3dRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_VERTEX, 0)																			// 0:ViewProjection
		.Add32BitConstant(D3D12_SHADER_VISIBILITY_PIXEL, 0, 1)																	// 1:TextureHandle
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_VERTEX, 0)								// 2:Instancing
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, DescriptorHeap::kMaxSRVCount, D3D12_SHADER_VISIBILITY_PIXEL, 0)	// 3:Texture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// Samplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create Instance3dRootSignature\n");
	instance3dRootSignature_->SetName(L"Instance3dRootSignature");

	// RingInstance3d用ルートシグネチャの作成
	ringInstance3dRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_VERTEX, 0)																			// 0:ViewProjection
		.Add32BitConstant(D3D12_SHADER_VISIBILITY_PIXEL, 0, 1)																	// 1:TextureHandle
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_VERTEX, 0)								// 2:Instancing
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, DescriptorHeap::kMaxSRVCount, D3D12_SHADER_VISIBILITY_PIXEL, 0)	// 3:Texture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// Samplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create RingInstance3dRootSignature\n");
	ringInstance3dRootSignature_->SetName(L"RingInstance3dRootSignature");

	// Line用ルートシグネチャの作成
	lineRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_VERTEX, 0)												// 0:ViewProjection
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_VERTEX, 0)	// 1:Line
		.Create(logStream, device_);
	Logger::Log(logStream, "Create LineRootSignature\n");
	lineRootSignature_->SetName(L"LineRootSignature");

	// Skybox用ルートシグネチャの作成
	skyboxRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_VERTEX, 0)												// 0:WorldTransform
		.AddCBuffer(D3D12_SHADER_VISIBILITY_VERTEX, 1)												// 1:ViewProjection
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 0)												// 2:Material
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 0)	// 3:Texture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// Samplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create SkyboxRootSignature\n");
	skyboxRootSignature_->SetName(L"SkyboxRootSignature");

	// Fullscreen用ルートシグネチャの作成
	fullscreenRootSignature_ = RootSignature()
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 0)	// 0:Texture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// Samplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create FullscreenRootSignature\n");
	fullscreenRootSignature_->SetName(L"FullscreenRootSignature");

	// Grayscale用ルートシグネチャの作成
	grayscaleRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 0)												// 0:Color
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 0)	// 1:Texture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// Samplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create GrayscaleRootSignature\n");
	grayscaleRootSignature_->SetName(L"GrayscaleRootSignature");

	// Vignette用ルートシグネチャの作成
	vignetteRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 0)												// 0:Vignette
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 0)	// 1:Texture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// Samplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create VignetteRootSignature\n");
	vignetteRootSignature_->SetName(L"VignetteRootSignature");

	// BoxFilter用ルートシグネチャの作成
	boxFilterRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 0)												// 0:FilterData
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 0)	// 1:Texture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// Samplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create BoxFilterRootSignature\n");
	boxFilterRootSignature_->SetName(L"BoxFilterRootSignature");

	// GaussianFilter用ルートシグネチャの作成
	gaussianFilterRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 0)												// 0:FilterData
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 0)	// 1:Texture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// Samplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create GaussianFilterRootSignature\n");
	gaussianFilterRootSignature_->SetName(L"GaussianFilterRootSignature");

	// LuminanceBasedOutline用ルートシグネチャの作成
	luminanceBasedOutlineRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 0)												// 0:FilterData
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 0)	// 1:Texture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// Samplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create LuminanceBasedOutlineRootSignature\n");
	luminanceBasedOutlineRootSignature_->SetName(L"LuminanceBasedOutlineRootSignature");

	// DepthBasedOutline用ルートシグネチャの作成
	depthBasedOutlineRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 0)												// 0:FilterData
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 1)												// 1:MaterialData
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 0)	// 2:Texture
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 1)	// 3:DepthTexture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// SamplerLinearを追加
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 1, D3D12_SHADER_VISIBILITY_PIXEL)	// SamplerPointを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create DepthBasedOutlineRootSignature\n");
	depthBasedOutlineRootSignature_->SetName(L"DepthBasedOutlineRootSignature");

	// RadialBlur用ルートシグネチャの作成
	radialBlurRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 0)												// 0:BlurData
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 0)	// 1:Texture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// Samplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create RadialBlurRootSignature\n");
	radialBlurRootSignature_->SetName(L"RadialBlurRootSignature");

	// Dissolve用ルートシグネチャの作成
	dissolveRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 0)												// 0:DissolveData
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 0)	// 1:Texture
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 1)	// 2:MaskTexture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// Samplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create DissolveRootSignature\n");
	dissolveRootSignature_->SetName(L"DissolveRootSignature");

	// Noise用ルートシグネチャの作成
	noiseRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_PIXEL, 0)												// 0:NoiseData
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL, 0)	// 1:Texture
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_PIXEL)	// Samplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create NoiseRootSignature\n");
	noiseRootSignature_->SetName(L"NoiseRootSignature");

	// 深度ステンシルテクスチャコピー用ルートシグネチャの作成
	depthStencilCopyRootSignature_ = RootSignature()
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_ALL, 0)	// 0:DepthStencil
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, D3D12_SHADER_VISIBILITY_ALL, 0)	// 1:HiZMipMap
		.Create(logStream, device_);
	Logger::Log(logStream, "Create DepthStencilCopyRootSignature\n");
	depthStencilCopyRootSignature_->SetName(L"DepthStencilCopyRootSignature");

	// HiZミップマップ生成用ルートシグネチャの作成
	generateHiZMipMapRootSignature_ = RootSignature()
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_ALL, 0)	// 0:ReadHiZMipMap
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, D3D12_SHADER_VISIBILITY_ALL, 0)	// 1:WriteHiZMipMap
		.Create(logStream, device_);
	Logger::Log(logStream, "Create GenerateHiZMipMapRootSignature\n");
	generateHiZMipMapRootSignature_->SetName(L"GenerateHiZMipMapRootSignature");

	// オクルージョンカリング用ルートシグネチャの作成
	occlusionCullingRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_ALL, 0)												// 0:Frustum
		.AddCBuffer(D3D12_SHADER_VISIBILITY_ALL, 1)												// 1:ViewProjection
		.AddCBuffer(D3D12_SHADER_VISIBILITY_ALL, 2)												// 2:CameraPosition
		.Add32BitConstant(D3D12_SHADER_VISIBILITY_ALL, 3, 40)									// 3:Constant
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_ALL, 0)	// 4:Object
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_ALL, 1)	// 5:Mesh
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_ALL, 2)	// 6:MeshLOD
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_ALL, 3)	// 7:HiZTexture
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, D3D12_SHADER_VISIBILITY_ALL, 0)	// 8:ProcessedIndirectCommand
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, D3D12_SHADER_VISIBILITY_ALL, 1)	// 9:ProcessedIndirectCommandCounter
		.AddSampler(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_COMPARISON_FUNC_NEVER, D3D12_FLOAT32_MAX, 0, D3D12_SHADER_VISIBILITY_ALL)	// Samplerを追加
		.Create(logStream, device_);
	Logger::Log(logStream, "Create OcclusionCullingRootSignature\n");
	occlusionCullingRootSignature_->SetName(L"OcclusionCullingRootSignature");

	// フットプリント用ルートシグネチャの作成
	footprintRootSignature_ = RootSignature()
		.AddCBuffer(D3D12_SHADER_VISIBILITY_ALL, 0)												// 0:PerDispatch
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_ALL, 0)	// 1:Footprint
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, D3D12_SHADER_VISIBILITY_ALL, 0)	// 2:FootprintMap
		.Create(logStream, device_);
	Logger::Log(logStream, "Create FootprintRootSignature\n");
	footprintRootSignature_->SetName(L"FootprintRootSignature");

	// フットプリントマップ用ルートシグネチャの作成
	footprintMapRootSignature_ = RootSignature()
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_ALL, 0)	// 0:FootprintMap
		.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, D3D12_SHADER_VISIBILITY_ALL, 0)	// 1:FootprintResultl;kj
		.Create(logStream, device_);
	Logger::Log(logStream, "Create FootprintMapRootSignature\n");
	footprintMapRootSignature_->SetName(L"FootprintMapRootSignature");

	// ビューポートの設定
	viewport_.Width = static_cast<float>(width);	// ビューポートの幅
	viewport_.Height = static_cast<float>(height);	// ビューポートの高さ
	viewport_.TopLeftX = 0.0f;	// 左上X座標
	viewport_.TopLeftY = 0.0f;	// 左上Y座標
	viewport_.MinDepth = 0.0f;	// 最小深度
	viewport_.MaxDepth = 1.0f;	// 最大深度

	// シザー矩形の設定
	scissorRect_.left = 0;			// 左上X座標
	scissorRect_.top = 0;			// 左上Y座標
	scissorRect_.right = width;		// 右下X座標
	scissorRect_.bottom = height;	// 右下Y座標

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;	// 深度値のクリア値
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 深度24bit、ステンシル8bit

	// 深度ステンシルテクスチャを作成
	mainCameraDepthStencilTexture_ = Resource::CreateTexture2D(this, width, height, 1, D3D12_RESOURCE_STATE_DEPTH_WRITE, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, &depthClearValue);
	mainCameraDepthStencilTexture_->SetName("MainCameraDepthStencilTexture");
	previousMainCameraDepthStencilTexture_ = Resource::CreateTexture2D(this, width, height, 1, D3D12_RESOURCE_STATE_COPY_DEST, DXGI_FORMAT_D24_UNORM_S8_UINT);
	previousMainCameraDepthStencilTexture_->SetName("PreviousMainCameraDepthStencilTexture");
	debugCameraDepthStencilTexture_ = Resource::CreateTexture2D(this, width, height, 1, D3D12_RESOURCE_STATE_DEPTH_WRITE, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, &depthClearValue);
	debugCameraDepthStencilTexture_->SetName("DebugCameraDepthStencilTexture");

	// メインカメラ用DSVの作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;			// フォーマット
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;	// テクスチャ2D
	mainCameraDepthStencilTextureDSVHandle_ = dsvDescriptorHeap_.AllocateDescriptor();
	dsvDescriptorHeap_.CreateDepthStencilView(mainCameraDepthStencilTexture_->GetResource(), dsvDesc, mainCameraDepthStencilTextureDSVHandle_);
	Logger::Log(logStream, "MainCameraDepthStencilTexture DSVDescriptorIndex: " + std::to_string(mainCameraDepthStencilTextureDSVHandle_) + "\n");

	// デバッグカメラ用DSVの作成
	debugCameraDepthStencilTextureDSVHandle_ = dsvDescriptorHeap_.AllocateDescriptor();
	dsvDescriptorHeap_.CreateDepthStencilView(debugCameraDepthStencilTexture_->GetResource(), dsvDesc, debugCameraDepthStencilTextureDSVHandle_);
	Logger::Log(logStream, "DebugCameraDepthStencilTexture DSVDescriptorIndex: " + std::to_string(debugCameraDepthStencilTextureDSVHandle_) + "\n");

	// ImGuiの初期化
	ImGuiManager::Initialize(hwnd, device_, commandQueue_, swapChainDesc, rtvDesc, dsvDesc, gpuCbvSrvUavDescriptorHeap_, logStream);
}

void Device::NewFrame() {
	// DescriptorHeapを設定する
	ID3D12DescriptorHeap *descriptorHeaps[] = { gpuCbvSrvUavDescriptorHeap_.GetDescriptorHeap() };
	commandList_->SetDescriptorHeaps(1, descriptorHeaps);
}

void Device::SetupSwapChain() {
	// これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	// TransitionBarrierの設定(Present->RenderTarget)
	Resource::TransitionBarrier(commandList_, swapChainResources_[backBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// 描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle = rtvDescriptorHeap_.GetCPUDescriptorHandle(swapChainResourceRTVHandles_[backBufferIndex]);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUHandle = dsvDescriptorHeap_.GetCPUDescriptorHandle(debugCameraDepthStencilTextureDSVHandle_);
	commandList_->OMSetRenderTargets(1, &rtvCPUHandle, false, &dsvCPUHandle);

	// 指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
	commandList_->ClearRenderTargetView(rtvCPUHandle, clearColor, 0, nullptr);

	// ビューポートとシザー矩形の設定
	commandList_->RSSetViewports(1, &viewport_);		// ビューポートの設定
	commandList_->RSSetScissorRects(1, &scissorRect_);	// シザー矩形の設定
}

void Device::EndFrame() {
	// 前フレームのメインカメラ用深度ステンシルを保存する
	mainCameraDepthStencilTexture_->TransitionBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE);
	previousMainCameraDepthStencilTexture_->CopyFrom(mainCameraDepthStencilTexture_->GetResource());
	mainCameraDepthStencilTexture_->TransitionBarrier(D3D12_RESOURCE_STATE_DEPTH_WRITE);

	// これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	// TransitionBarrierの設定(RenderTarget->Present)
	Resource::TransitionBarrier(commandList_, swapChainResources_[backBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	// コマンドリストのクローズ
	HRESULT hr = commandList_->Close();
	assert(SUCCEEDED(hr));

	// コマンドリストを実行するための配列
	ID3D12CommandList *commandLists[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(1, commandLists);

	// スワップチェーンを表示
	hr = swapChain_->Present(1, 0);
	assert(SUCCEEDED(hr));

	// フェンスの値を更新
	fenceValue_++;

	// Signalを送る
	hr = commandQueue_->Signal(fence_.Get(), fenceValue_);
	assert(SUCCEEDED(hr));

	// フェンスの状態を監視
	if (fence_->GetCompletedValue() < fenceValue_) {
		hr = fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		assert(SUCCEEDED(hr));
		WaitForSingleObject(fenceEvent_, INFINITE);
	}

	// FPS固定の更新
	UpdateFixFPS();

	// 次のフレーム用のコマンドリストを準備
	hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

void Device::Finalize() const {
	ImGuiManager::Finalize();	// ImGuiの終了処理
	CloseHandle(fenceEvent_);	// フェンスイベントの解放
}

void Device::InitializeFixFPS() {
	// 基準時間を現在の時間に設定
	reference_ = std::chrono::steady_clock::now();
}

void Device::UpdateFixFPS() {
	// 1/60秒の時間（60FPS相当）
	constexpr std::chrono::microseconds kMinTime(static_cast<uint64_t>(1000000.0f / 60.0f));

	// 1/60秒よりも少し短い時間（65FPS相当）
	constexpr std::chrono::microseconds kMinCheckTime(static_cast<uint64_t>(1000000.0f / 65.0f));

	// 現在の時間を取得
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	// 経過時間を計算
	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	// 経過時間が1/60秒未満の場合
	if (elapsed < kMinCheckTime) {
		// 1/60秒経過するまで微小なスリープを繰り返す
		while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
			// 1マイクロ秒スリープ
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}

	// 基準時間を現在の時間に更新
	reference_ = std::chrono::steady_clock::now();
}

void Device::EnableDebugLayer() const {
    Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        debugController->SetEnableGPUBasedValidation(true);
    }
}