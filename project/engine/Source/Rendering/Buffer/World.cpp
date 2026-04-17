#define NOMINMAX
#include "World.h"
#include "Device.h"
#include "EntityComponentSystem.h"
#include "Renderer.h"
#include "DebugRenderer.h"
#include "ConstantBuffer.h"
#include "Resource.h"
#include "IndirectCommand.h"
#include "Object.h"
#include "Transform.h"
#include "Material.h"
#include "Model.h"
#include "Sprite.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Frustum.h"
#include "AABB.h"
#include "Footprint.h"
#include "FootprintMap.h"
#include "Logger.h"
#include <d3dx12.h>

const UINT World::kCommandSizePerFrame = World::kMaxAABB * sizeof(IndirectCommand);
const UINT World::kCommandBufferCounterOffset = AlignForUavCounter(World::kCommandSizePerFrame);

World::World(Device *device, std::ofstream &logStream) {
	DescriptorHeap *gpuCbvSrvUavDescriptorHeap = device->GetGpuCbvSrvUavDescriptorHeap();

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
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kCamera)]->Initialize(device, sizeof(CameraForGPU), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kCamera)]->SetName("Camera");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kDirectionalLight)]->Initialize(device, sizeof(DirectionalLight), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kDirectionalLight)]->SetName("DirectionalLight");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kFrustum)]->Initialize(device, sizeof(Frustum), 2);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kFrustum)]->SetName("Frustum");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kFootprintMap)]->Initialize(device, sizeof(FootprintMap), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kFootprintMap)]->SetName("FootprintMap");

	// スプライト用のビュープロジェクションの初期データ設定
	ViewProjectionData viewProjection;
	viewProjection.view = MakeIdentity4x4();
	viewProjection.projection = MakeOrthographicMatrix(0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 100.0f);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kViewProjection)]->CopyData(&viewProjection, sizeof(ViewProjectionData), 0);

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
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kCulling)] = Resource::CreateUploadBuffer(device, sizeof(CullingData) * kMaxAABB);
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kCulling)]->SetName("Culling");
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kIndirectCommand)] = Resource::CreateBuffer(device, D3D12_HEAP_TYPE_DEFAULT, kCommandSizePerFrame, D3D12_RESOURCE_STATE_COMMON);
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kIndirectCommand)]->SetName("IndirectCommand");
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kBlendMode)] = Resource::CreateUploadBuffer(device, sizeof(BlendModeForGPU) * kMaxAABB);
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kBlendMode)]->SetName("BlendMode");
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kFootprint)] = Resource::CreateUploadBuffer(device, sizeof(FootprintForGPU) * kMaxFootprint);
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kFootprint)]->SetName("Footprint");
	structuredBuffers_[static_cast<size_t>(StructuredBufferType::kFootprint)]->Map(reinterpret_cast<void **>(&footprintData_));

	// ブレンドモード名リスト
	std::array<std::string, static_cast<size_t>(BlendMode::kCountOfBlendMode)> blendModeNames = {
		"None",
		"Normal",
		"Additive",
		"Subtractive",
		"Multiplicative",
		"Screen"
	};

	// ブレンド別処理済みコマンドバッファリストの初期化
	for (size_t i = 0; i < static_cast<size_t>(BlendMode::kCountOfBlendMode); ++i) {
		blendProcessedCommandBuffers_[i] = Resource::CreateRWBuffer(device, kCommandBufferCounterOffset + sizeof(UINT));
		const std::string name = blendModeNames[i] + "BlendProcessedCommandBuffer";
		blendProcessedCommandBuffers_[i]->SetName(name);
	}

	// 処理済みコマンドバッファカウンタリセット用バッファの作成
	processedCommandBufferCounterReset_ = Resource::CreateUploadBuffer(device, sizeof(UINT));
	processedCommandBufferCounterReset_->SetName("ProcessedCommandBufferCounterReset");

	// カウンタリセット用バッファに0を書き込む
	UINT8 *pMappedCounterReset = nullptr;
	processedCommandBufferCounterReset_->Map(reinterpret_cast<void **>(&pMappedCounterReset));
	ZeroMemory(pMappedCounterReset, sizeof(UINT));
	processedCommandBufferCounterReset_->Unmap();

	// コマンドバッファ転送用中間バッファの作成
	commandBufferUpload_ = Resource::CreateUploadBuffer(device, kCommandSizePerFrame);
	commandBufferUpload_->SetName("CommandBufferUpload");

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

	// Culling用SRVの設定
	srvBufferDesc.Buffer.NumElements = kMaxAABB;					// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(CullingData);	// 構造体のサイズ

	// Culling用SRVの作成
	cullingHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[static_cast<size_t>(StructuredBufferType::kCulling)]->GetResource(), srvBufferDesc, cullingHandle_);
	Logger::Log(logStream, "Culling SRVDescriptorIndex: " + std::to_string(cullingHandle_) + "\n");

	// 間接コマンド用SRVの設定
	srvBufferDesc.Buffer.StructureByteStride = sizeof(IndirectCommand);

	// 間接コマンド用SRVの作成
	indirectCommandHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[static_cast<size_t>(StructuredBufferType::kIndirectCommand)]->GetResource(), srvBufferDesc, indirectCommandHandle_);
	Logger::Log(logStream, "IndirectCommand SRVDescriptorIndex: " + std::to_string(indirectCommandHandle_) + "\n");

	// ブレンドモード用SRVの設定
	srvBufferDesc.Buffer.StructureByteStride = sizeof(BlendModeForGPU);

	// ブレンドモード用SRVの作成
	blendModeHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[static_cast<size_t>(StructuredBufferType::kBlendMode)]->GetResource(), srvBufferDesc, blendModeHandle_);
	Logger::Log(logStream, "BlendMode SRVDescriptorIndex: " + std::to_string(blendModeHandle_) + "\n");

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

	for (size_t i = 0; i < static_cast<size_t>(BlendMode::kCountOfBlendMode); i++) {
		// 処理済みコマンドバッファ用UAVの設定
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;								// バッファなのでフォーマットなし
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;					// バッファビュー
		uavDesc.Buffer.FirstElement = 0;									// 先頭の要素
		uavDesc.Buffer.NumElements = kMaxAABB;								// 要素数
		uavDesc.Buffer.StructureByteStride = sizeof(IndirectCommand);		// 構造体のサイズ
		uavDesc.Buffer.CounterOffsetInBytes = kCommandBufferCounterOffset;	// カウンタあり
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;					// 特になし

		// 処理済みコマンドバッファ用UAVの作成
		blendProcessedIndirectCommandHandle_[i] = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
		gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(blendProcessedCommandBuffers_[i]->GetResource(), blendProcessedCommandBuffers_[i]->GetResource(), uavDesc, blendProcessedIndirectCommandHandle_[i]);
		Logger::Log(logStream, blendModeNames[i] + "ProcessedIndirectCommand UAVDescriptorIndex: " + std::to_string(blendProcessedIndirectCommandHandle_[i]) + "\n");
	}

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
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavBufferDesc{};
	uavBufferDesc.Format = DXGI_FORMAT_UNKNOWN;					// バッファなのでフォーマットなし
	uavBufferDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;	// バッファビュー
	uavBufferDesc.Buffer.FirstElement = 0;						// 先頭の要素
	uavBufferDesc.Buffer.NumElements = 1;						// 要素数
	uavBufferDesc.Buffer.StructureByteStride = sizeof(Int4);	// 構造体のサイズ
	uavBufferDesc.Buffer.CounterOffsetInBytes = 0;				// カウンタなし
	uavBufferDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;	// 特になし

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
	// カリングカメラ
	registry_->ForEach<Camera, Transform, CullingCamera>([&](uint32_t entity, Camera *camera, Transform *transform, CullingCamera *cullingCamera) {
		ViewProjectionData viewProjection = MakeViewProjection(*camera, *transform);
		Frustum frustum = MakeFrustum(viewProjection);
		CameraForGPU cameraData = {
			.worldPosition = transform->translate * transform->worldMatrix
		};
		constantBuffers_[static_cast<uint32_t>(ConstantBufferType::kViewProjection)]->CopyData(&viewProjection, sizeof(ViewProjectionData), 1);
		constantBuffers_[static_cast<uint32_t>(ConstantBufferType::kFrustum)]->CopyData(&frustum, sizeof(Frustum), 0);
		constantBuffers_[static_cast<uint32_t>(ConstantBufferType::kCamera)]->CopyData(&cameraData, sizeof(CameraForGPU), 0);
		}, exclude<Disabled>());

	// デバッグカメラ
	registry_->ForEach<Camera, Transform, RenderingCamera>([&](uint32_t entity, Camera *camera, Transform *transform, RenderingCamera *renderingCamera) {
		ViewProjectionData viewProjection = MakeViewProjection(*camera, *transform);
		Frustum frustum = MakeFrustum(viewProjection);
		constantBuffers_[static_cast<uint32_t>(ConstantBufferType::kViewProjection)]->CopyData(&viewProjection, sizeof(ViewProjectionData), 2);
		constantBuffers_[static_cast<uint32_t>(ConstantBufferType::kFrustum)]->CopyData(&frustum, sizeof(Frustum), 1);
		}, exclude<Disabled, CullingCamera>());
}

void World::TransferTransform() {
	registry_->ForEach<Transform, Object, DirtyTransform>([&](uint32_t entity, Transform *transform, Object *object, DirtyTransform *dirtyTransform) {
		TransformationMatrix transformationMatrix;
		if (registry_->HasComponent<Model>(entity)) {
			Model *model = registry_->GetComponent<Model>(entity);
			Matrix4x4 localMatrix = ModelManager::MakeLocalMatrix(model->modelData.rootNode);
			transformationMatrix.worldMatrix = localMatrix * transform->worldMatrix;
		} else {
			transformationMatrix.worldMatrix = transform->worldMatrix;
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

void World::TransferFootprint() {
	ZeroMemory(footprintData_, sizeof(FootprintForGPU) * kMaxFootprint);
	registry_->ForEach<Footprint, Collision::Sphere>([&](uint32_t entity, Footprint *footprint, Collision::Sphere *sphere) {
		footprintData_[footprint->id].worldPos = sphere->center;
		footprintData_[footprint->id].radius = sphere->radius;
		footprintData_[footprint->id].color = footprint->color;
		}, exclude<Disabled>());

	registry_->ForEach<Footprint, Model>([&](uint32_t entity, Footprint *footprint, Model *model) {
		for (const MeshData &mesh : model->modelData.meshes) {
			footprintData_[footprint->id].worldPos = mesh.worldCollisionData.sphere.center;
			footprintData_[footprint->id].radius = mesh.worldCollisionData.sphere.radius;
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