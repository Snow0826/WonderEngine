#define NOMINMAX
#include "World.h"
#include "Device.h"
#include "Window.h"
#include "EntityComponentSystem.h"
#include "Renderer.h"
#include "DebugRenderer.h"
#include "ConstantBuffer.h"
#include "Resource.h"
#include "IndirectCommand.h"
#include "Object.h"
#include "Material.h"
#include "Model.h"
#include "Sprite.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "FrustumRenderer.h"
#include "AABBRenderer.h"
#include "Footprint.h"
#include "FootprintMap.h"
#include "Logger.h"
#include <d3dx12.h>

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

namespace {
	std::vector<std::string> postEffectNames = {
		"None",
		"GrayScale",
		"Vignette",
		"BoxFilter",
		"GaussianFilter"
	};
}

World::World(Device *device, std::ofstream &logStream) {
	DescriptorHeap *gpuCbvSrvUavDescriptorHeap = device->GetGpuCbvSrvUavDescriptorHeap();
	DescriptorHeap *cpuCbvSrvUavDescriptorHeap = device->GetCpuCbvSrvUavDescriptorHeap();

	// 定数バッファの初期化
	for (auto &constantBuffer : constantBuffers_) {
		constantBuffer = std::make_unique<ConstantBuffer>();
	}

	constantBuffers_[static_cast<size_t>(ConstantBufferType::kTransform)]->Initialize(device, sizeof(TransformationMatrix), kMaxObject);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kTransform)]->SetName("Transform");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kViewProjection)]->Initialize(device, sizeof(ViewProjectionData), 3);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kViewProjection)]->SetName("ViewProjection");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kMaterial)]->Initialize(device, sizeof(Material), kMaxObject);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kMaterial)]->SetName("Material");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kCameraPosition)]->Initialize(device, sizeof(CameraPosition), 2);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kCameraPosition)]->SetName("CameraPosition");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kDirectionalLight)]->Initialize(device, sizeof(DirectionalLight), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kDirectionalLight)]->SetName("DirectionalLight");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kFrustum)]->Initialize(device, sizeof(Frustum), 2);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kFrustum)]->SetName("Frustum");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kGrayscaleColor)]->Initialize(device, sizeof(GrayscaleColor), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kGrayscaleColor)]->SetName("GrayscaleColor");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kVignetteParam)]->Initialize(device, sizeof(VignetteParam), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kVignetteParam)]->SetName("VignetteParam");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kBoxFilterParam)]->Initialize(device, sizeof(BoxFilterParam), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kBoxFilterParam)]->SetName("BoxFilterParam");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kGaussianFilterParam)]->Initialize(device, sizeof(GaussianFilterParam), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kGaussianFilterParam)]->SetName("GaussianFilterParam");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kFootprintMap)]->Initialize(device, sizeof(FootprintMap), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kFootprintMap)]->SetName("FootprintMap");

	// スプライト用のビュープロジェクションの初期データ設定
	ViewProjectionData viewProjection;
	viewProjection.view = MakeIdentity4x4();
	viewProjection.projection = MakeOrthographicMatrix(0.0f, 0.0f, static_cast<float>(Window::GetClientWidth()), static_cast<float>(Window::GetClientHeight()), 0.0f, 100.0f);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kViewProjection)]->CopyData(&viewProjection, sizeof(ViewProjectionData), 0);

	// BoxFilter用のパラメータの初期データ設定
	boxFilterParam_.texelSize = Vector2{ 1.0f / static_cast<float>(Window::GetClientWidth()), 1.0f / static_cast<float>(Window::GetClientHeight()) };

	// GaussianFilter用のパラメータの初期データ設定
	gaussianFilterParam_.texelSize = Vector2{ 1.0f / static_cast<float>(Window::GetClientWidth()), 1.0f / static_cast<float>(Window::GetClientHeight()) };

	// 構造化バッファの初期化
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kLine)] = Resource::CreateUploadBuffer(device, sizeof(Rendering::Line) * kMaxLine);
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kLine)]->SetName("Line");
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kLine)]->Map(reinterpret_cast<void **>(&lineData_));
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kPointLight)] = Resource::CreateUploadBuffer(device, sizeof(PointLight) * kMaxPointLight);
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kPointLight)]->SetName("PointLight");
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kPointLight)]->Map(reinterpret_cast<void **>(&pointLightData_));
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kSpotLight)] = Resource::CreateUploadBuffer(device, sizeof(SpotLight) * kMaxSpotLight);
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kSpotLight)]->SetName("SpotLight");
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kSpotLight)]->Map(reinterpret_cast<void **>(&spotLightData_));
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kObject)] = Resource::CreateUploadBuffer(device, sizeof(CullingObjectData) * kMaxObject);
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kObject)]->SetName("Object");
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kMesh)] = Resource::CreateUploadBuffer(device, sizeof(CullingMeshData) * kMaxAABB);
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kMesh)]->SetName("Mesh");
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kMeshLOD)] = Resource::CreateBuffer(device, D3D12_HEAP_TYPE_DEFAULT, sizeof(MeshLOD) * kMaxAABB, D3D12_RESOURCE_STATE_COMMON);
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kMeshLOD)]->SetName("MeshLOD");
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kFootprint)] = Resource::CreateUploadBuffer(device, sizeof(FootprintForGPU) * kMaxFootprint);
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kFootprint)]->SetName("Footprint");
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kFootprint)]->Map(reinterpret_cast<void **>(&footprintData_));

	// コマンドバッファ転送用中間バッファの作成
	commandBufferUpload_ = Resource::CreateUploadBuffer(device, sizeof(MeshLOD) * kMaxAABB);
	commandBufferUpload_->SetName("CommandBufferUpload");

	// カリング済みコマンドバッファの初期化
	processedCommandBuffer_ = Resource::CreateRWBuffer(device, sizeof(IndirectCommand) * kMaxAABB);
	processedCommandBuffer_->SetName("ProcessedCommandBuffer");

	// コマンドカウンターバッファの初期化
	constexpr size_t kMaxQueue = static_cast<size_t>(MeshType::kCountOfMeshType) * static_cast<size_t>(BlendMode::kCountOfBlendMode);
	commandCounterBuffer_ = Resource::CreateRWBuffer(device, sizeof(uint32_t) * kMaxQueue);
	commandCounterBuffer_->SetName("CommandCounterBuffer");

	// キューオフセットリストの初期化
	for (uint32_t i = 0; i < queueOffsets_.size(); i++) {
		uint32_t queueIndex = i * 4;
		queueOffsets_[i].x = queueIndex * kMaxCommandPerQueue;
		queueOffsets_[i].y = (queueIndex + 1) * kMaxCommandPerQueue;
		queueOffsets_[i].z = (queueIndex + 2) * kMaxCommandPerQueue;
		queueOffsets_[i].w = (queueIndex + 3) * kMaxCommandPerQueue;
	}

	// Line用SRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvBufferDesc{};
	srvBufferDesc.Format = DXGI_FORMAT_UNKNOWN;											// バッファなのでフォーマットなし
	srvBufferDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;							// バッファビュー
	srvBufferDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;	// 標準設定
	srvBufferDesc.Buffer.FirstElement = 0;												// 先頭の要素
	srvBufferDesc.Buffer.NumElements = kMaxLine;										// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(Rendering::Line);					// 構造体のサイズ
	srvBufferDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;							// 特になし

	// Line用SRVの作成
	lineHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[static_cast<size_t>(StructuredBufferType::kLine)]->GetResource(), srvBufferDesc, lineHandle_);
	Logger::Log(logStream, "Line SRVDescriptorIndex: " + std::to_string(lineHandle_) + "\n");

	// PointLight用SRVの設定
	srvBufferDesc.Buffer.NumElements = kMaxPointLight;				// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(PointLight);	// 構造体のサイズ

	// PointLight用SRVの作成
	pointLightHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[static_cast<size_t>(StructuredBufferType::kPointLight)]->GetResource(), srvBufferDesc, pointLightHandle_);
	Logger::Log(logStream, "PointLight SRVDescriptorIndex: " + std::to_string(pointLightHandle_) + "\n");

	// SpotLight用SRVの設定
	srvBufferDesc.Buffer.NumElements = kMaxSpotLight;				// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(SpotLight);	// 構造体のサイズ

	// SpotLight用SRVの作成
	spotLightHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[static_cast<size_t>(StructuredBufferType::kSpotLight)]->GetResource(), srvBufferDesc, spotLightHandle_);
	Logger::Log(logStream, "SpotLight SRVDescriptorIndex: " + std::to_string(spotLightHandle_) + "\n");

	// カリングオブジェクト用SRVの設定
	srvBufferDesc.Buffer.NumElements = kMaxAABB;							// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(CullingObjectData);	// 構造体のサイズ

	// カリングオブジェクト用SRVの作成
	cullingObjectHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[static_cast<size_t>(StructuredBufferType::kObject)]->GetResource(), srvBufferDesc, cullingObjectHandle_);
	Logger::Log(logStream, "CullingObject SRVDescriptorIndex: " + std::to_string(cullingObjectHandle_) + "\n");

	// カリングメッシュ用SRVの設定
	srvBufferDesc.Buffer.NumElements = kMaxAABB;						// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(CullingMeshData);	// 構造体のサイズ

	// カリングメッシュ用SRVの作成
	cullingMeshHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[static_cast<size_t>(StructuredBufferType::kMesh)]->GetResource(), srvBufferDesc, cullingMeshHandle_);
	Logger::Log(logStream, "CullingMesh SRVDescriptorIndex: " + std::to_string(cullingMeshHandle_) + "\n");

	// メッシュLOD用SRVの設定
	srvBufferDesc.Buffer.StructureByteStride = sizeof(MeshLOD);	// 構造体のサイズ

	// メッシュLOD用SRVの作成
	meshLODHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[static_cast<size_t>(StructuredBufferType::kMeshLOD)]->GetResource(), srvBufferDesc, meshLODHandle_);
	Logger::Log(logStream, "MeshLOD SRVDescriptorIndex: " + std::to_string(meshLODHandle_) + "\n");

	// レンダーテクスチャの作成
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	clearValue.Color[0] = 1.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 1.0f;
	renderTexture_ = Resource::CreateTexture2D(device, Window::GetClientWidth(), Window::GetClientHeight(), 1, D3D12_RESOURCE_STATE_RENDER_TARGET, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &clearValue);

	// レンダーテクスチャ用のRTVの作成
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	renderTextureRTVHandle_ = device->GetRTVDescriptorHeap()->AllocateDescriptor();
	device->GetRTVDescriptorHeap()->CreateRenderTargetView(renderTexture_->GetResource(), rtvDesc, renderTextureRTVHandle_);
	Logger::Log(logStream, "RenderTexture RTVDescriptorIndex: " + std::to_string(renderTextureRTVHandle_) + "\n");

	// レンダーテクスチャ用のSRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvRenderTextureDesc{};
	srvRenderTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvRenderTextureDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvRenderTextureDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvRenderTextureDesc.Texture2D.MostDetailedMip = 0;
	srvRenderTextureDesc.Texture2D.MipLevels = 1;
	srvRenderTextureDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	renderTextureSRVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(renderTexture_->GetResource(), srvRenderTextureDesc, renderTextureSRVHandle_);
	Logger::Log(logStream, "RenderTexture SRVDescriptorIndex: " + std::to_string(renderTextureSRVHandle_) + "\n");

	// 深度バッファ用SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDepthStencilCopyDesc{};
	srvDepthStencilCopyDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDepthStencilCopyDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDepthStencilCopyDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDepthStencilCopyDesc.Texture2D.MostDetailedMip = 0;
	srvDepthStencilCopyDesc.Texture2D.MipLevels = 1;
	srvDepthStencilCopyDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	depthStencilCopySourceHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(device->GetPreviousDepthStencilTexture()->GetResource(), srvDepthStencilCopyDesc, depthStencilCopySourceHandle_);
	Logger::Log(logStream, "DepthStencilCopySource SRVDescriptorIndex: " + std::to_string(depthStencilCopySourceHandle_) + "\n");

#pragma region HiZMipMap
	const int32_t width = static_cast<int32_t>(device->GetViewport().Width);
	const int32_t height = static_cast<int32_t>(device->GetViewport().Height);
	mipLevels_ = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	hiZTexture_ = Resource::CreateTexture2D(device, width, height, mipLevels_, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	hiZTexture_->SetName("hiZTexture");

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDepthStencilCopyDesc{};
	uavDepthStencilCopyDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavDepthStencilCopyDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDepthStencilCopyDesc.Texture2D.MipSlice = 0;
	uavDepthStencilCopyDesc.Texture2D.PlaneSlice = 0;
	depthStencilCopyDestHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(hiZTexture_->GetResource(), uavDepthStencilCopyDesc, depthStencilCopyDestHandle_);
	Logger::Log(logStream, "DepthStencilCopyDest UAVDescriptorIndex: " + std::to_string(depthStencilCopyDestHandle_) + "\n");

	for (uint32_t mip = 0; mip < mipLevels_ - 1; ++mip) {
		D3D12_SHADER_RESOURCE_VIEW_DESC srvHiZTextureDesc{};
		srvHiZTextureDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvHiZTextureDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvHiZTextureDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvHiZTextureDesc.Texture2D.MostDetailedMip = mip;
		srvHiZTextureDesc.Texture2D.MipLevels = 1;
		srvHiZTextureDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		uint32_t srvHandle = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
		gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(hiZTexture_->GetResource(), srvHiZTextureDesc, srvHandle);
		hiZMipMapReadHandles_.emplace_back(srvHandle);
		Logger::Log(logStream, "mip" + std::to_string(mip) + " HiZMipMap SRVDescriptorIndex: " + std::to_string(srvHandle) + "\n");

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavHiZTextureDesc{};
		uavHiZTextureDesc.Format = DXGI_FORMAT_R32_FLOAT;
		uavHiZTextureDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavHiZTextureDesc.Texture2D.MipSlice = mip + 1;
		uavHiZTextureDesc.Texture2D.PlaneSlice = 0;
		uint32_t uavHandle = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
		gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(hiZTexture_->GetResource(), uavHiZTextureDesc, uavHandle);
		hiZMipMapWriteHandles_.emplace_back(uavHandle);
		Logger::Log(logStream, "mip" + std::to_string(mip + 1) + " HiZMipMap UAVDescriptorIndex: " + std::to_string(uavHandle) + "\n");
	}
#pragma endregion

	D3D12_SHADER_RESOURCE_VIEW_DESC srvHiZTextureDesc{};
	srvHiZTextureDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvHiZTextureDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvHiZTextureDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvHiZTextureDesc.Texture2D.MostDetailedMip = 0;
	srvHiZTextureDesc.Texture2D.MipLevels = mipLevels_;
	srvHiZTextureDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	hiZTextureHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(hiZTexture_->GetResource(), srvHiZTextureDesc, hiZTextureHandle_);
	Logger::Log(logStream, "HiZTexture SRVDescriptorIndex: " + std::to_string(hiZTextureHandle_) + "\n");

	// カリング済みコマンドバッファ用UAVの設定
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavBufferDesc{};
	uavBufferDesc.Format = DXGI_FORMAT_UNKNOWN;							// バッファなのでフォーマットなし
	uavBufferDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;			// バッファビュー
	uavBufferDesc.Buffer.FirstElement = 0;								// 先頭の要素
	uavBufferDesc.Buffer.NumElements = kMaxAABB;						// 要素数
	uavBufferDesc.Buffer.StructureByteStride = sizeof(IndirectCommand);	// 構造体のサイズ
	uavBufferDesc.Buffer.CounterOffsetInBytes = 0;						// カウンタなし
	uavBufferDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;			// 特になし

	// カリング済みコマンドバッファ用UAVの作成
	processedCommandHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(processedCommandBuffer_->GetResource(), uavBufferDesc, processedCommandHandle_);
	Logger::Log(logStream, "ProcessedCommand UAVDescriptorIndex: " + std::to_string(processedCommandHandle_) + "\n");

	// コマンドカウンターバッファ用UAVの設定
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavCounterBufferDesc{};
	uavCounterBufferDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;	// バッファビュー
	uavCounterBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;				// カウンターバッファはフォーマットR32_TYPELESSで作成し、UAVではR32_UINTとして扱う
	uavCounterBufferDesc.Buffer.FirstElement = 0;						// 先頭の要素
	uavCounterBufferDesc.Buffer.NumElements = kMaxQueue;				// 要素数
	uavCounterBufferDesc.Buffer.StructureByteStride = 0;				// 構造体のサイズ
	uavCounterBufferDesc.Buffer.CounterOffsetInBytes = 0;				// カウンタなし
	uavCounterBufferDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;		// RAWフラグを設定して、バッファ全体を1つの要素として扱う

	// コマンドカウンターバッファ用UAVの作成
	commandCounterHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(commandCounterBuffer_->GetResource(), uavCounterBufferDesc, commandCounterHandle_);
	cpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(commandCounterBuffer_->GetResource(), uavCounterBufferDesc, commandCounterHandle_);
	Logger::Log(logStream, "CommandCounter UAVDescriptorIndex: " + std::to_string(commandCounterHandle_) + "\n");

	// フットプリント用SRVの設定
	srvBufferDesc.Buffer.NumElements = kMaxFootprint;					// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(FootprintForGPU);	// 構造体のサイズ

	// フットプリント用SRVの作成
	footprintHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[static_cast<size_t>(StructuredBufferType::kFootprint)]->GetResource(), srvBufferDesc, footprintHandle_);
	Logger::Log(logStream, "Footprint SRVDescriptorIndex: " + std::to_string(footprintHandle_) + "\n");

	// フットプリントマップ用バッファの作成
	footprintMapBuffer_ = Resource::CreateRWBuffer(device, sizeof(Int4));
	footprintMapBuffer_->SetName("FootprintMapBuffer");
	footprintMapBuffer_->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// フットプリントマップ用UAVの設定
	uavBufferDesc.Buffer.NumElements = 1;						// 要素数
	uavBufferDesc.Buffer.StructureByteStride = sizeof(Int4);	// 構造体のサイズ

	// フットプリントマップ用UAVの作成
	footprintMapHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(footprintMapBuffer_->GetResource(), uavBufferDesc, footprintMapHandle_);
	Logger::Log(logStream, "FootprintMap UAVDescriptorIndex: " + std::to_string(footprintMapHandle_) + "\n");

	// フットプリントマップ読み戻しバッファの作成
	footprintMapReadbackBuffer_ = Resource::CreateReadbackBuffer(device, sizeof(Int4));
	footprintMapReadbackBuffer_->SetName("FootprintMapReadbackBuffer");
	footprintMapReadbackBuffer_->Map(reinterpret_cast<void **>(&colorData_));
}

World::~World() = default;

void World::Update() {
	TransferDirectionalLight();
	TransferPointLight();
	TransferSpotLight();
	TransferCamera();
	TransferTransform();
	TransferMaterial();
	TransferGrayscaleColor();
	TransferVignetteParam();
	TransferBoxFilterParam();
	TransferGaussianFilterParam();
	TransferFootprint();
	TransferFootprintMap();
}

void World::Edit() {
#ifdef USE_IMGUI
	if (ImGui::Checkbox("Culling", &isCulling_)) {
		TransformSystem transformSystem{ registry_ };
		if (isCulling_) {
			registry_->ForEach<Model>([&](uint32_t entity, Model *model) {
				transformSystem.MarkDirty(entity);
				registry_->AddComponent(entity, UseCulling{});
				}, exclude<Disabled, UseCulling>());
		} else {
			registry_->ForEach<Model, UseCulling>([&](uint32_t entity, Model *model, UseCulling *useCulling) {
				transformSystem.MarkDirty(entity);
				registry_->RemoveComponent<UseCulling>(entity);
				}, exclude<Disabled>());
		}
	}

	ImGui::Checkbox("Result", &isResult_);
	ImGui::DragInt4("Color", &colorData_->x, 1.0f, 0, 100);

	if (ImGui::BeginCombo("PostEffect", postEffectNames[static_cast<size_t>(postEffect_)].c_str())) {
		for (size_t i = 0; i < postEffectNames.size(); i++) {
			bool selected = (postEffectNames[i] == postEffectNames[static_cast<size_t>(postEffect_)]);
			if (ImGui::Selectable(postEffectNames[i].c_str(), selected)) {
				postEffect_ = static_cast<PostEffect>(i);
			}
			if (selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::TreeNode("Grayscale")) {
		ImGui::ColorEdit3("Color", &grayscaleColor_.r, ImGuiColorEditFlags_Float);
		if (ImGui::Button("Reset")) {
			grayscaleColor_ = { .r = 1.0f, .g = 1.0f, .b = 1.0f };
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Vignette")) {
		ImGui::DragFloat("Scale", &vignetteParam_.scale, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		ImGui::DragFloat("Intensity", &vignetteParam_.intensity, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		if (ImGui::Button("Reset")) {
			vignetteParam_ = { .scale = 16.0f, .intensity = 0.8f };
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("BoxFilter")) {
		ImGui::DragInt("KernelRadius", &boxFilterParam_.kernelRadius, 1.0f, 1, 16);
		if (ImGui::Button("Reset")) {
			boxFilterParam_.kernelRadius = 1;
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("GaussianFilter")) {
		ImGui::DragInt("KernelRadius", &gaussianFilterParam_.kernelRadius, 1.0f, 1, 16);
		ImGui::DragFloat("Sigma", &gaussianFilterParam_.sigma, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		if (ImGui::Button("Reset")) {
			gaussianFilterParam_.kernelRadius = 1;
			gaussianFilterParam_.sigma = 2.0f;
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}

void World::CopyFootprintMapBuffer() {
	footprintMapBuffer_->TransitionBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE);
	footprintMapReadbackBuffer_->CopyFrom(footprintMapBuffer_->GetResource(), 0, 0, sizeof(Int4));
	footprintMapBuffer_->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

void World::CopyLineData(const Rendering::Line *data, size_t size) {
	ZeroMemory(lineData_, sizeof(Rendering::Line) * kMaxLine);
	memcpy(lineData_, data, size);
}

void World::TransferDirectionalLight() {
	uint32_t directionalLightCounter = 0;
	registry_->ForEach<DirectionalLight>([&](uint32_t entity, DirectionalLight *directionalLight) {
		constantBuffers_[static_cast<size_t>(ConstantBufferType::kDirectionalLight)]->CopyData(directionalLight, sizeof(DirectionalLight), directionalLightCounter);
		directionalLightCounter++;
		}, exclude<Disabled>());
}

void World::TransferPointLight() {
	uint32_t pointLightCounter = 0;
	registry_->ForEach<PointLight>([&](uint32_t entity, PointLight *pointLight) {
		pointLightData_[pointLightCounter] = *pointLight;
		pointLightCounter++;
		}, exclude<Disabled>());
}

void World::TransferSpotLight() {
	uint32_t spotLightCounter = 0;
	registry_->ForEach<SpotLight>([&](uint32_t entity, SpotLight *spotLight) {
		spotLightData_[spotLightCounter] = *spotLight;
		spotLightCounter++;
		}, exclude<Disabled>());
}

void World::TransferCamera() {
	TransformSystem transformSystem{ registry_ };

	// カリングカメラ
	registry_->ForEach<Camera, QuaternionTransform, CullingCamera>([&](uint32_t entity, Camera *camera, QuaternionTransform *transform, CullingCamera *cullingCamera) {
		ViewProjectionData viewProjection = MakeViewProjection(*camera, *transform);
		Frustum frustum = MakeFrustum(viewProjection);
		CameraPosition cameraPosition = {
			.worldPosition = transformSystem.GetWorldPosition(entity)
		};
		constantBuffers_[static_cast<uint32_t>(ConstantBufferType::kViewProjection)]->CopyData(&viewProjection, sizeof(ViewProjectionData), 1);
		constantBuffers_[static_cast<uint32_t>(ConstantBufferType::kFrustum)]->CopyData(&frustum, sizeof(Frustum), 0);
		constantBuffers_[static_cast<uint32_t>(ConstantBufferType::kCameraPosition)]->CopyData(&cameraPosition, sizeof(CameraPosition), 0);
		}, exclude<Disabled>());

	// デバッグカメラ
	registry_->ForEach<Camera, QuaternionTransform, RenderingCamera>([&](uint32_t entity, Camera *camera, QuaternionTransform *transform, RenderingCamera *renderingCamera) {
		ViewProjectionData viewProjection = MakeViewProjection(*camera, *transform);
		Frustum frustum = MakeFrustum(viewProjection);
		CameraPosition cameraPosition = {
			.worldPosition = transformSystem.GetWorldPosition(entity)
		};
		constantBuffers_[static_cast<uint32_t>(ConstantBufferType::kViewProjection)]->CopyData(&viewProjection, sizeof(ViewProjectionData), 2);
		constantBuffers_[static_cast<uint32_t>(ConstantBufferType::kFrustum)]->CopyData(&frustum, sizeof(Frustum), 1);
		constantBuffers_[static_cast<uint32_t>(ConstantBufferType::kCameraPosition)]->CopyData(&cameraPosition, sizeof(CameraPosition), 1);

		}, exclude<Disabled, CullingCamera>());
}

void World::TransferTransform() {
	registry_->ForEach<Object, DirtyTransform>([&](uint32_t entity, Object *object, DirtyTransform *dirtyTransform) {
		TransformationMatrix transformationMatrix;
		Model *model = registry_->GetComponent<Model>(entity);
		if (model && !registry_->HasComponent<Skeleton>(entity)) {
			transformationMatrix.worldMatrix = ModelManager::MakeLocalMatrix(model->modelData.rootNode);
		}
		EulerTransform *eulerTransform = registry_->GetComponent<EulerTransform>(entity);
		QuaternionTransform *quaternionTransform = registry_->GetComponent<QuaternionTransform>(entity);
		if (eulerTransform) {
			transformationMatrix.worldMatrix *= eulerTransform->worldMatrix;
		} else if (quaternionTransform) {
			transformationMatrix.worldMatrix *= quaternionTransform->worldMatrix;
		}
		transformationMatrix.worldInverseTransposeMatrix = transformationMatrix.worldMatrix.inverse().transpose();
		constantBuffers_[static_cast<size_t>(ConstantBufferType::kTransform)]->CopyData(&transformationMatrix, sizeof(TransformationMatrix), object->handle);
		registry_->RemoveComponent<DirtyTransform>(entity);
		}, exclude<Disabled>());
}

void World::TransferMaterial() {
	registry_->ForEach<Material, Object, DirtyMaterial>([&](uint32_t entity, Material *material, Object *object, DirtyMaterial *dirtyMaterial) {
		constantBuffers_[static_cast<size_t>(ConstantBufferType::kMaterial)]->CopyData(material, sizeof(Material), object->handle);
		registry_->RemoveComponent<DirtyMaterial>(entity);
		}, exclude<Disabled>());
}

void World::TransferGrayscaleColor() {
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kGrayscaleColor)]->CopyData(&grayscaleColor_, sizeof(GrayscaleColor), 0);
}

void World::TransferVignetteParam() {
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kVignetteParam)]->CopyData(&vignetteParam_, sizeof(VignetteParam), 0);
}

void World::TransferBoxFilterParam() {
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kBoxFilterParam)]->CopyData(&boxFilterParam_, sizeof(BoxFilterParam), 0);
}

void World::TransferGaussianFilterParam() {
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kGaussianFilterParam)]->CopyData(&gaussianFilterParam_, sizeof(GaussianFilterParam), 0);
}

void World::TransferFootprint() {
	ZeroMemory(footprintData_, sizeof(FootprintForGPU) * kMaxFootprint);
	registry_->ForEach<Footprint, Collision::Sphere>([&](uint32_t entity, Footprint *footprint, Collision::Sphere *sphere) {
		footprintData_[footprint->id].worldPos = sphere->center;
		footprintData_[footprint->id].radius = sphere->radius;
		footprintData_[footprint->id].color = footprint->color;
		}, exclude<Disabled>());

	registry_->ForEach<Footprint, Model>([&](uint32_t entity, Footprint *footprint, Model *model) {
		for (const MeshData &mesh : model->modelData.meshes) {
			footprintData_[footprint->id].worldPos = mesh.sphere.center;
			footprintData_[footprint->id].radius = mesh.sphere.radius;
			footprintData_[footprint->id].color = footprint->color;
		}
		}, exclude<Disabled>());
}

void World::TransferFootprintMap() {
	uint32_t footprintMapCounter = 0;
	registry_->ForEach<FootprintMap>([&](uint32_t entity, FootprintMap *footprintMap) {
		constantBuffers_[static_cast<size_t>(ConstantBufferType::kFootprintMap)]->CopyData(footprintMap, sizeof(FootprintMap), footprintMapCounter);
		footprintMapCounter++;
		}, exclude<Disabled>());
}