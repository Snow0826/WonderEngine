#define NOMINMAX
#include "World.h"
#include "Device.h"
#include "Window.h"
#include "EntityComponentSystem.h"
#include "Renderer.h"
#include "DebugRenderer.h"
#include "ConstantBuffer.h"
#include "Resource.h"
#include "SkinCluster.h"
#include "InstanceAllocator.h"
#include "Material.h"
#include "Model.h"
#include "Primitive.h"
#include "Sprite.h"
#include "Particle.h"
#include "Skybox.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "FrustumRenderer.h"
#include "AABBRenderer.h"
#include "TreeGenerator.h"
#include "Footprint.h"
#include "FootprintMap.h"
#include "Logger.h"
#include <d3dx12.h>

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

namespace {
	// ポストエフェクト名リスト
	std::vector<std::string> postEffectNames = {
		"None",
		"GrayScale",
		"Vignette",
		"BoxFilter",
		"GaussianFilter",
		"LuminanceBasedOutline",
		"DepthBasedOutline",
		"RadialBlur",
		"Dissolve",
		"Noise"
	};
}

World::World(Device *device, MeshManager *meshManager, SkinClusterManager *skinClusterManager, std::ofstream &logStream)
	: meshManager_(meshManager), skinClusterManager_(skinClusterManager) {
	DescriptorHeap *gpuCbvSrvUavDescriptorHeap = device->GetGpuCbvSrvUavDescriptorHeap();
	DescriptorHeap *cpuCbvSrvUavDescriptorHeap = device->GetCpuCbvSrvUavDescriptorHeap();

	// 定数バッファの初期化
	for (auto &constantBuffer : constantBuffers_) {
		constantBuffer = std::make_unique<ConstantBuffer>();
	}

	constantBuffers_[static_cast<size_t>(ConstantBufferType::kViewProjection)]->Initialize(device, sizeof(ViewProjectionData), 3);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kViewProjection)]->SetName("ViewProjection");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kParticlePerView)]->Initialize(device, sizeof(ParticlePerViewData), 2);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kParticlePerView)]->SetName("ParticlePerView");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kCameraPosition)]->Initialize(device, sizeof(CameraPosition), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kCameraPosition)]->SetName("CameraPosition");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kDirectionalLight)]->Initialize(device, sizeof(DirectionalLight), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kDirectionalLight)]->SetName("DirectionalLight");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kFrustum)]->Initialize(device, sizeof(Frustum), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kFrustum)]->SetName("Frustum");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kSkybox)]->Initialize(device, sizeof(SkyboxForGPU), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kSkybox)]->SetName("Skybox");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kGrayscaleColor)]->Initialize(device, sizeof(GrayscaleColor), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kGrayscaleColor)]->SetName("GrayscaleColor");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kVignetteParam)]->Initialize(device, sizeof(VignetteParam), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kVignetteParam)]->SetName("VignetteParam");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kBoxFilterParam)]->Initialize(device, sizeof(BoxFilterParam), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kBoxFilterParam)]->SetName("BoxFilterParam");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kGaussianFilterParam)]->Initialize(device, sizeof(GaussianFilterParam), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kGaussianFilterParam)]->SetName("GaussianFilterParam");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kPrewittFilterParam)]->Initialize(device, sizeof(PrewittFilterParam), 2);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kPrewittFilterParam)]->SetName("PrewittFilterParam");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kDepthMaterial)]->Initialize(device, sizeof(DepthMaterial), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kDepthMaterial)]->SetName("DepthMaterial");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kRadialBlurParam)]->Initialize(device, sizeof(RadialBlurParam), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kRadialBlurParam)]->SetName("RadialBlurParam");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kDissolveParam)]->Initialize(device, sizeof(DissolveParam), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kDissolveParam)]->SetName("DissolveParam");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kPerFrame)]->Initialize(device, sizeof(PerFrame), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kPerFrame)]->SetName("PerFrame");
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kEmitterSphere)]->Initialize(device, sizeof(EmitterSphere), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kEmitterSphere)]->SetName("EmitterSphere");
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

	// PrewittFilter用のパラメータの初期データ設定
	luminancePrewittFilterParam_.texelSize = Vector2{ 1.0f / static_cast<float>(Window::GetClientWidth()), 1.0f / static_cast<float>(Window::GetClientHeight()) };
	depthPrewittFilterParam_.texelSize = Vector2{ 1.0f / static_cast<float>(Window::GetClientWidth()), 1.0f / static_cast<float>(Window::GetClientHeight()) };

	// キューオフセットリストの初期化
	for (uint32_t i = 0; i < cullingConstantsData_.queueOffsets.size(); i++) {
		uint32_t queueIndex = i * 4;
		cullingConstantsData_.queueOffsets[i].x = queueIndex * kMaxCommandPerQueue;
		cullingConstantsData_.queueOffsets[i].y = (queueIndex + 1) * kMaxCommandPerQueue;
		cullingConstantsData_.queueOffsets[i].z = (queueIndex + 2) * kMaxCommandPerQueue;
		cullingConstantsData_.queueOffsets[i].w = (queueIndex + 3) * kMaxCommandPerQueue;
	}

	// InstanceData用StructuredBufferの作成
	uint32_t structuredBufferIndex = static_cast<size_t>(StructuredBufferType::kInstanceData);
	structuredBuffers_[structuredBufferIndex] = Resource::CreateUploadBuffer(device, sizeof(InstanceData) * kMaxObject);
	structuredBuffers_[structuredBufferIndex]->SetName("InstanceData");
	structuredBuffers_[structuredBufferIndex]->Map(reinterpret_cast<void **>(&instanceData_));
	structuredBufferHandles_[structuredBufferIndex] = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// InstanceData用SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvBufferDesc{};
	srvBufferDesc.Format = DXGI_FORMAT_UNKNOWN;											// バッファなのでフォーマットなし
	srvBufferDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;							// バッファビュー
	srvBufferDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;	// 標準設定
	srvBufferDesc.Buffer.FirstElement = 0;												// 先頭の要素
	srvBufferDesc.Buffer.NumElements = kMaxObject;										// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(InstanceData);					// 構造体のサイズ
	srvBufferDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;							// 特になし
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[structuredBufferIndex]->GetResource(), srvBufferDesc, structuredBufferHandles_[structuredBufferIndex]);

	// Material用StructuredBufferの作成
	structuredBufferIndex = static_cast<size_t>(StructuredBufferType::kMaterial);
	structuredBuffers_[structuredBufferIndex] = Resource::CreateUploadBuffer(device, sizeof(Material) * kMaxObject);
	structuredBuffers_[structuredBufferIndex]->SetName("Material");
	structuredBuffers_[structuredBufferIndex]->Map(reinterpret_cast<void **>(&materialData_));
	structuredBufferHandles_[structuredBufferIndex] = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// Material用SRVの作成
	srvBufferDesc.Buffer.StructureByteStride = sizeof(Material);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[structuredBufferIndex]->GetResource(), srvBufferDesc, structuredBufferHandles_[structuredBufferIndex]);

	// TextureData用StructuredBufferの作成
	structuredBufferIndex = static_cast<size_t>(StructuredBufferType::kTextureData);
	structuredBuffers_[structuredBufferIndex] = Resource::CreateUploadBuffer(device, sizeof(TextureData) * kMaxAABB);
	structuredBuffers_[structuredBufferIndex]->SetName("TextureData");
	structuredBuffers_[structuredBufferIndex]->Map(reinterpret_cast<void **>(&textureData_));
	structuredBufferHandles_[structuredBufferIndex] = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// TextureData用SRVの作成
	srvBufferDesc.Buffer.NumElements = kMaxAABB;					// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(TextureData);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[structuredBufferIndex]->GetResource(), srvBufferDesc, structuredBufferHandles_[structuredBufferIndex]);

	// Line用StructuredBufferの作成
	structuredBufferIndex = static_cast<size_t>(StructuredBufferType::kLine);
	structuredBuffers_[structuredBufferIndex] = Resource::CreateUploadBuffer(device, sizeof(Rendering::Line) * kMaxLine);
	structuredBuffers_[structuredBufferIndex]->SetName("Line");
	structuredBuffers_[structuredBufferIndex]->Map(reinterpret_cast<void **>(&lineData_));
	structuredBufferHandles_[structuredBufferIndex] = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// Line用SRVの作成
	srvBufferDesc.Buffer.NumElements = kMaxLine;						// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(Rendering::Line);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[structuredBufferIndex]->GetResource(), srvBufferDesc, structuredBufferHandles_[structuredBufferIndex]);
	Logger::Log(logStream, "Line SRVDescriptorIndex: " + std::to_string(structuredBufferHandles_[structuredBufferIndex]) + "\n");

	// PointLight用StructuredBufferの作成
	structuredBufferIndex = static_cast<size_t>(StructuredBufferType::kPointLight);
	structuredBuffers_[structuredBufferIndex] = Resource::CreateUploadBuffer(device, sizeof(PointLight) * kMaxPointLight);
	structuredBuffers_[structuredBufferIndex]->SetName("PointLight");
	structuredBuffers_[structuredBufferIndex]->Map(reinterpret_cast<void **>(&pointLightData_));
	structuredBufferHandles_[structuredBufferIndex] = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// PointLight用SRVの作成
	srvBufferDesc.Buffer.NumElements = kMaxPointLight;				// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(PointLight);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[structuredBufferIndex]->GetResource(), srvBufferDesc, structuredBufferHandles_[structuredBufferIndex]);
	Logger::Log(logStream, "PointLight SRVDescriptorIndex: " + std::to_string(structuredBufferHandles_[structuredBufferIndex]) + "\n");

	// SpotLight用StructuredBufferの作成
	structuredBufferIndex = static_cast<size_t>(StructuredBufferType::kSpotLight);
	structuredBuffers_[structuredBufferIndex] = Resource::CreateUploadBuffer(device, sizeof(SpotLight) * kMaxSpotLight);
	structuredBuffers_[structuredBufferIndex]->SetName("SpotLight");
	structuredBuffers_[structuredBufferIndex]->Map(reinterpret_cast<void **>(&spotLightData_));
	structuredBufferHandles_[structuredBufferIndex] = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// SpotLight用SRVの作成
	srvBufferDesc.Buffer.NumElements = kMaxSpotLight;				// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(SpotLight);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[structuredBufferIndex]->GetResource(), srvBufferDesc, structuredBufferHandles_[structuredBufferIndex]);
	Logger::Log(logStream, "SpotLight SRVDescriptorIndex: " + std::to_string(structuredBufferHandles_[structuredBufferIndex]) + "\n");

	// Cylinder用StructuredBufferの作成
	structuredBufferIndex = static_cast<size_t>(StructuredBufferType::kCylinder);
	structuredBuffers_[structuredBufferIndex] = Resource::CreateUploadBuffer(device, sizeof(Cylinder) * kMaxAABB);
	structuredBuffers_[structuredBufferIndex]->SetName("Cylinder");
	structuredBuffers_[structuredBufferIndex]->Map(reinterpret_cast<void **>(&cylinderData_));
	structuredBufferHandles_[structuredBufferIndex] = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// Cylinder用SRVの作成
	srvBufferDesc.Buffer.NumElements = kMaxAABB;					// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(Cylinder);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[structuredBufferIndex]->GetResource(), srvBufferDesc, structuredBufferHandles_[structuredBufferIndex]);
	Logger::Log(logStream, "Cylinder SRVDescriptorIndex: " + std::to_string(structuredBufferHandles_[structuredBufferIndex]) + "\n");

	// MeshInfoForAABB用StructuredBufferの作成
	structuredBufferIndex = static_cast<size_t>(StructuredBufferType::kMeshInfoForAABB);
	structuredBuffers_[structuredBufferIndex] = Resource::CreateUploadBuffer(device, sizeof(MeshInfoForAABB) * kMaxAABB);
	structuredBuffers_[structuredBufferIndex]->SetName("MeshInfoForAABB");
	structuredBuffers_[structuredBufferIndex]->Map(reinterpret_cast<void **>(&meshInfoForAABB_));
	structuredBufferHandles_[structuredBufferIndex] = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// MeshInfoForAABB用SRVの作成
	srvBufferDesc.Buffer.StructureByteStride = sizeof(MeshInfoForAABB);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[structuredBufferIndex]->GetResource(), srvBufferDesc, structuredBufferHandles_[structuredBufferIndex]);
	Logger::Log(logStream, "MeshInfoForAABB SRVDescriptorIndex: " + std::to_string(structuredBufferHandles_[structuredBufferIndex]) + "\n");

	// VertexDataForAABB用StructuredBufferの作成
	structuredBufferIndex = static_cast<size_t>(StructuredBufferType::kVertexDataForAABB);
	structuredBuffers_[structuredBufferIndex] = Resource::CreateUploadBuffer(device, sizeof(VertexDataForAABB) * kMaxVertices);
	structuredBuffers_[structuredBufferIndex]->SetName("VertexDataForAABB");
	structuredBuffers_[structuredBufferIndex]->Map(reinterpret_cast<void **>(&vertexDataForAABB_));
	structuredBufferHandles_[structuredBufferIndex] = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// VertexDataForAABB用SRVの作成
	srvBufferDesc.Buffer.NumElements = kMaxVertices;						// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(VertexDataForAABB);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[structuredBufferIndex]->GetResource(), srvBufferDesc, structuredBufferHandles_[structuredBufferIndex]);
	Logger::Log(logStream, "VertexDataForAABB SRVDescriptorIndex: " + std::to_string(structuredBufferHandles_[structuredBufferIndex]) + "\n");

	// CullingMeshData用StructuredBufferの作成
	structuredBufferIndex = static_cast<size_t>(StructuredBufferType::kCullingMeshData);
	structuredBuffers_[structuredBufferIndex] = Resource::CreateUploadBuffer(device, sizeof(CullingMeshData) * kMaxAABB);
	structuredBuffers_[structuredBufferIndex]->SetName("CullingMeshData");
	structuredBuffers_[structuredBufferIndex]->Map(reinterpret_cast<void **>(&cullingMeshData_));
	structuredBufferHandles_[structuredBufferIndex] = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// CullingMeshData用SRVの作成
	srvBufferDesc.Buffer.NumElements = kMaxAABB;						// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(CullingMeshData);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[structuredBufferIndex]->GetResource(), srvBufferDesc, structuredBufferHandles_[structuredBufferIndex]);
	Logger::Log(logStream, "CullingMeshData SRVDescriptorIndex: " + std::to_string(structuredBufferHandles_[structuredBufferIndex]) + "\n");

	// CullingObjectData用StructuredBufferの作成
	structuredBufferIndex = static_cast<size_t>(StructuredBufferType::kCullingObjectData);
	structuredBuffers_[structuredBufferIndex] = Resource::CreateUploadBuffer(device, sizeof(CullingObjectData) * kMaxObject);
	structuredBuffers_[structuredBufferIndex]->SetName("CullingObjectData");
	structuredBuffers_[structuredBufferIndex]->Map(reinterpret_cast<void **>(&cullingObjectData_));
	structuredBufferHandles_[structuredBufferIndex] = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// CullingObjectData用SRVの作成
	srvBufferDesc.Buffer.NumElements = kMaxAABB;							// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(CullingObjectData);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[structuredBufferIndex]->GetResource(), srvBufferDesc, structuredBufferHandles_[structuredBufferIndex]);
	Logger::Log(logStream, "CullingObjectData SRVDescriptorIndex: " + std::to_string(structuredBufferHandles_[structuredBufferIndex]) + "\n");

	// MeshLOD用StructuredBufferの作成
	structuredBufferIndex = static_cast<size_t>(StructuredBufferType::kMeshLOD);
	structuredBuffers_[structuredBufferIndex] = Resource::CreateBuffer(device, D3D12_HEAP_TYPE_DEFAULT, sizeof(MeshLOD) * kMaxAABB, D3D12_RESOURCE_STATE_COMMON);
	structuredBuffers_[structuredBufferIndex]->SetName("MeshLOD");
	structuredBufferHandles_[structuredBufferIndex] = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// MeshLOD用SRVの作成
	srvBufferDesc.Buffer.StructureByteStride = sizeof(MeshLOD);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[structuredBufferIndex]->GetResource(), srvBufferDesc, structuredBufferHandles_[structuredBufferIndex]);
	Logger::Log(logStream, "MeshLOD SRVDescriptorIndex: " + std::to_string(structuredBufferHandles_[structuredBufferIndex]) + "\n");

	// Footprint用StructuredBufferの作成
	structuredBufferIndex = static_cast<size_t>(StructuredBufferType::kFootprint);
	structuredBuffers_[structuredBufferIndex] = Resource::CreateUploadBuffer(device, sizeof(FootprintForGPU) * kMaxFootprint);
	structuredBuffers_[structuredBufferIndex]->SetName("Footprint");
	structuredBuffers_[structuredBufferIndex]->Map(reinterpret_cast<void **>(&footprintData_));
	structuredBufferHandles_[structuredBufferIndex] = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// Footprint用SRVの作成
	srvBufferDesc.Buffer.NumElements = kMaxFootprint;					// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(FootprintForGPU);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(structuredBuffers_[structuredBufferIndex]->GetResource(), srvBufferDesc, structuredBufferHandles_[structuredBufferIndex]);
	Logger::Log(logStream, "Footprint SRVDescriptorIndex: " + std::to_string(structuredBufferHandles_[structuredBufferIndex]) + "\n");

	// シーンのレンダーテクスチャの作成
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	clearValue.Color[0] = 1.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 1.0f;
	sceneRenderTexture_ = Resource::CreateTexture2D(device, Window::GetClientWidth(), Window::GetClientHeight(), 1, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &clearValue);
	sceneRenderTexture_->SetName("SceneRenderTexture");

	// シーンのレンダーテクスチャ用のRTVの作成
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	sceneRenderTextureRTVHandle_ = device->GetRTVDescriptorHeap()->AllocateDescriptor();
	device->GetRTVDescriptorHeap()->CreateRenderTargetView(sceneRenderTexture_->GetResource(), rtvDesc, sceneRenderTextureRTVHandle_);
	Logger::Log(logStream, "SceneRenderTexture RTVDescriptorIndex: " + std::to_string(sceneRenderTextureRTVHandle_) + "\n");

	// シーンのレンダーテクスチャ用のSRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvRenderTextureDesc{};
	srvRenderTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvRenderTextureDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvRenderTextureDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvRenderTextureDesc.Texture2D.MostDetailedMip = 0;
	srvRenderTextureDesc.Texture2D.MipLevels = 1;
	srvRenderTextureDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	sceneRenderTextureSRVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(sceneRenderTexture_->GetResource(), srvRenderTextureDesc, sceneRenderTextureSRVHandle_);
	Logger::Log(logStream, "SceneRenderTexture SRVDescriptorIndex: " + std::to_string(sceneRenderTextureSRVHandle_) + "\n");

	// ゲームのレンダーテクスチャの作成
	gameRenderTexture_ = Resource::CreateTexture2D(device, Window::GetClientWidth(), Window::GetClientHeight(), 1, D3D12_RESOURCE_STATE_RENDER_TARGET, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &clearValue);
	gameRenderTexture_->SetName("GameRenderTexture");

	// ゲームのレンダーテクスチャ用のRTVの作成
	gameRenderTextureRTVHandle_ = device->GetRTVDescriptorHeap()->AllocateDescriptor();
	device->GetRTVDescriptorHeap()->CreateRenderTargetView(gameRenderTexture_->GetResource(), rtvDesc, gameRenderTextureRTVHandle_);
	Logger::Log(logStream, "GameRenderTexture RTVDescriptorIndex: " + std::to_string(gameRenderTextureRTVHandle_) + "\n");

	// ゲームのレンダーテクスチャ用のSRVの作成
	gameRenderTextureSRVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(gameRenderTexture_->GetResource(), srvRenderTextureDesc, gameRenderTextureSRVHandle_);
	Logger::Log(logStream, "GameRenderTexture SRVDescriptorIndex: " + std::to_string(gameRenderTextureSRVHandle_) + "\n");

	// ポストエフェクトのレンダーテクスチャの作成
	postEffectRenderTexture_ = Resource::CreateTexture2D(device, Window::GetClientWidth(), Window::GetClientHeight(), 1, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &clearValue);
	postEffectRenderTexture_->SetName("PostEffectRenderTexture");

	// ポストエフェクトのレンダーテクスチャ用のRTVの作成
	postEffectRenderTextureRTVHandle_ = device->GetRTVDescriptorHeap()->AllocateDescriptor();
	device->GetRTVDescriptorHeap()->CreateRenderTargetView(postEffectRenderTexture_->GetResource(), rtvDesc, postEffectRenderTextureRTVHandle_);
	Logger::Log(logStream, "PostEffectRenderTexture RTVDescriptorIndex: " + std::to_string(postEffectRenderTextureRTVHandle_) + "\n");

	// ポストエフェクトのレンダーテクスチャ用のSRVの作成
	postEffectRenderTextureSRVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(postEffectRenderTexture_->GetResource(), srvRenderTextureDesc, postEffectRenderTextureSRVHandle_);
	Logger::Log(logStream, "PostEffectRenderTexture SRVDescriptorIndex: " + std::to_string(postEffectRenderTextureSRVHandle_) + "\n");

	// 深度バッファ用SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDepthStencilTextureDesc{};
	srvDepthStencilTextureDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDepthStencilTextureDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDepthStencilTextureDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDepthStencilTextureDesc.Texture2D.MostDetailedMip = 0;
	srvDepthStencilTextureDesc.Texture2D.MipLevels = 1;
	srvDepthStencilTextureDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	// 前フレームのメインカメラの深度ステンシルテクスチャ用SRVの作成
	previousMainCameraDepthStencilTextureSRVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(device->GetPreviousMainCameraDepthStencilTexture()->GetResource(), srvDepthStencilTextureDesc, previousMainCameraDepthStencilTextureSRVHandle_);
	Logger::Log(logStream, "PreviousMainCameraDepthStencilTexture SRVDescriptorIndex: " + std::to_string(previousMainCameraDepthStencilTextureSRVHandle_) + "\n");

	// メインカメラの深度ステンシルテクスチャ用SRVの作成
	mainCameraDepthStencilTextureSRVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(device->GetMainCameraDepthStencilTexture()->GetResource(), srvDepthStencilTextureDesc, mainCameraDepthStencilTextureSRVHandle_);
	Logger::Log(logStream, "MainCameraDepthStencilTexture SRVDescriptorIndex: " + std::to_string(mainCameraDepthStencilTextureSRVHandle_) + "\n");

#pragma region HiZMipMap
	const int32_t width = static_cast<int32_t>(device->GetViewport().Width);
	const int32_t height = static_cast<int32_t>(device->GetViewport().Height);
	mipLevels_ = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	hiZTexture_ = Resource::CreateTexture2D(device, width, height, mipLevels_, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	hiZTexture_->SetName("hiZTexture");

	// HiZテクスチャ用SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvHiZTextureDesc{};
	srvHiZTextureDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvHiZTextureDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvHiZTextureDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvHiZTextureDesc.Texture2D.MostDetailedMip = 0;
	srvHiZTextureDesc.Texture2D.MipLevels = mipLevels_;
	srvHiZTextureDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	hiZTextureSRVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(hiZTexture_->GetResource(), srvHiZTextureDesc, hiZTextureSRVHandle_);
	Logger::Log(logStream, "HiZTexture SRVDescriptorIndex: " + std::to_string(hiZTextureSRVHandle_) + "\n");

	// HiZテクスチャ用UAVの作成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavHiZTextureDesc{};
	uavHiZTextureDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavHiZTextureDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavHiZTextureDesc.Texture2D.MipSlice = 0;
	uavHiZTextureDesc.Texture2D.PlaneSlice = 0;
	hiZTextureUAVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(hiZTexture_->GetResource(), uavHiZTextureDesc, hiZTextureUAVHandle_);
	Logger::Log(logStream, "HiZTexture UAVDescriptorIndex: " + std::to_string(hiZTextureUAVHandle_) + "\n");

	// 各ミップレベルのSRVとUAVの作成
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

	// AABB用RWStructuredBufferの作成
	aabbBuffer_ = Resource::CreateRWBuffer(device, sizeof(AABBForGPU) * kMaxAABB);
	aabbBuffer_->SetName("AABBBuffer");
	aabbSRVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	aabbUAVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// AABB用SRVの作成
	srvBufferDesc.Buffer.NumElements = kMaxAABB;					// 要素数
	srvBufferDesc.Buffer.StructureByteStride = sizeof(AABBForGPU);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(aabbBuffer_->GetResource(), srvBufferDesc, aabbSRVHandle_);
	Logger::Log(logStream, "AABB SRVDescriptorIndex: " + std::to_string(aabbSRVHandle_) + "\n");

	// AABB用UAVの作成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavBufferDesc{};
	uavBufferDesc.Format = DXGI_FORMAT_UNKNOWN;						// バッファなのでフォーマットなし
	uavBufferDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;		// バッファビュー
	uavBufferDesc.Buffer.FirstElement = 0;							// 先頭の要素
	uavBufferDesc.Buffer.NumElements = kMaxAABB;					// 要素数
	uavBufferDesc.Buffer.StructureByteStride = sizeof(AABBForGPU);	// 構造体のサイズ
	uavBufferDesc.Buffer.CounterOffsetInBytes = 0;					// カウンタなし
	uavBufferDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;		// 特になし
	gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(aabbBuffer_->GetResource(), uavBufferDesc, aabbUAVHandle_);
	Logger::Log(logStream, "AABB UAVDescriptorIndex: " + std::to_string(aabbUAVHandle_) + "\n");

	// コマンドアップロードバッファの作成
	commandUploadBuffer_ = Resource::CreateUploadBuffer(device, sizeof(MeshLOD) * kMaxAABB);
	commandUploadBuffer_->SetName("CommandBufferUpload");
	commandUploadBuffer_->Map(reinterpret_cast<void **>(&meshLODData_));

	// カリング済みコマンド用RWStructuredBufferの作成
	processedCommandBuffer_ = Resource::CreateRWBuffer(device, sizeof(IndirectCommand) * kMaxAABB);
	processedCommandBuffer_->SetName("ProcessedCommandBuffer");
	processedCommandHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// カリング済みコマンド用UAVの作成
	uavBufferDesc.Buffer.StructureByteStride = sizeof(IndirectCommand);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(processedCommandBuffer_->GetResource(), uavBufferDesc, processedCommandHandle_);
	Logger::Log(logStream, "ProcessedCommand UAVDescriptorIndex: " + std::to_string(processedCommandHandle_) + "\n");

	// メッシュコマンドステート用RWStructuredBufferの作成
	meshCommandStateBuffer_ = Resource::CreateRWBuffer(device, sizeof(MeshCommandState) * kMaxAABB);
	meshCommandStateBuffer_->SetName("MeshCommandStateBuffer");
	meshCommandStateSRVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	meshCommandStateUAVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// メッシュコマンドステート用SRVの作成
	srvBufferDesc.Buffer.StructureByteStride = sizeof(MeshCommandState);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(meshCommandStateBuffer_->GetResource(), srvBufferDesc, meshCommandStateSRVHandle_);
	Logger::Log(logStream, "MeshCommandState SRVDescriptorIndex: " + std::to_string(meshCommandStateSRVHandle_) + "\n");

	// メッシュコマンドステート用UAVの作成
	uavBufferDesc.Buffer.StructureByteStride = sizeof(MeshCommandState);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(meshCommandStateBuffer_->GetResource(), uavBufferDesc, meshCommandStateUAVHandle_);
	Logger::Log(logStream, "MeshCommandState UAVDescriptorIndex: " + std::to_string(meshCommandStateUAVHandle_) + "\n");

	// メッシュLODステート用RWStructuredBufferの作成
	meshLODStateBuffer_ = Resource::CreateRWBuffer(device, sizeof(MeshLODState) * kMaxAABB);
	meshLODStateBuffer_->SetName("MeshLODStateBuffer");
	meshLODStateSRVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	meshLODStateUAVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// メッシュLODステート用SRVの作成
	srvBufferDesc.Buffer.StructureByteStride = sizeof(MeshLODState);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(meshLODStateBuffer_->GetResource(), srvBufferDesc, meshLODStateSRVHandle_);
	Logger::Log(logStream, "MeshLODState SRVDescriptorIndex: " + std::to_string(meshLODStateSRVHandle_) + "\n");

	// メッシュLODステート用UAVの作成
	uavBufferDesc.Buffer.StructureByteStride = sizeof(MeshLODState);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(meshLODStateBuffer_->GetResource(), uavBufferDesc, meshLODStateUAVHandle_);
	Logger::Log(logStream, "MeshLODState UAVDescriptorIndex: " + std::to_string(meshLODStateUAVHandle_) + "\n");

	// カリング済みインスタンスインデックス用RWStructuredBufferの作成
	processedInstanceIndexBuffer_ = Resource::CreateRWBuffer(device, sizeof(uint32_t) * kMaxAABB);
	processedInstanceIndexBuffer_->SetName("ProcessedInstanceIndexBuffer");
	processedInstanceIndexSRVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	processedInstanceIndexUAVHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// カリング済みインスタンスインデックス用SRVの作成
	srvBufferDesc.Buffer.StructureByteStride = sizeof(uint32_t);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(processedInstanceIndexBuffer_->GetResource(), srvBufferDesc, processedInstanceIndexSRVHandle_);
	Logger::Log(logStream, "ProcessedInstanceIndex SRVDescriptorIndex: " + std::to_string(processedInstanceIndexSRVHandle_) + "\n");

	// カリング済みインスタンスインデックス用UAVの作成
	uavBufferDesc.Buffer.StructureByteStride = sizeof(uint32_t);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(processedInstanceIndexBuffer_->GetResource(), uavBufferDesc, processedInstanceIndexUAVHandle_);
	Logger::Log(logStream, "ProcessedInstanceIndex UAVDescriptorIndex: " + std::to_string(processedInstanceIndexUAVHandle_) + "\n");

	// コマンドカウンター用RWByteAddressBufferの作成
	constexpr size_t kMaxQueue = static_cast<size_t>(MeshType::kCountOfMeshType) * static_cast<size_t>(BlendMode::kCountOfBlendMode);
	commandCounterBuffer_ = Resource::CreateRWBuffer(device, sizeof(uint32_t) * kMaxQueue);
	commandCounterBuffer_->SetName("CommandCounterBuffer");
	commandCounterHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// コマンドカウンター用UAVの作成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavCounterBufferDesc{};
	uavCounterBufferDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;	// バッファビュー
	uavCounterBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;				// カウンターバッファはフォーマットR32_TYPELESSで作成し、UAVではR32_UINTとして扱う
	uavCounterBufferDesc.Buffer.FirstElement = 0;						// 先頭の要素
	uavCounterBufferDesc.Buffer.NumElements = kMaxQueue;				// 要素数
	uavCounterBufferDesc.Buffer.StructureByteStride = 0;				// 構造体のサイズ
	uavCounterBufferDesc.Buffer.CounterOffsetInBytes = 0;				// カウンタなし
	uavCounterBufferDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;		// RAWフラグを設定して、バッファ全体を1つの要素として扱う
	gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(commandCounterBuffer_->GetResource(), uavCounterBufferDesc, commandCounterHandle_);
	cpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(commandCounterBuffer_->GetResource(), uavCounterBufferDesc, commandCounterHandle_);
	Logger::Log(logStream, "CommandCounter UAVDescriptorIndex: " + std::to_string(commandCounterHandle_) + "\n");

	// フリーリストインデックス用RWStructuredBufferの作成
	freeListIndexBuffer_ = Resource::CreateRWBuffer(device, sizeof(int32_t));
	freeListIndexBuffer_->SetName("FreeListIndexBuffer");
	freeListIndexHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// フリーリストインデックス用UAVの作成
	uavBufferDesc.Buffer.NumElements = 1;						// 要素数
	uavBufferDesc.Buffer.StructureByteStride = sizeof(int32_t);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(freeListIndexBuffer_->GetResource(), uavBufferDesc, freeListIndexHandle_);
	Logger::Log(logStream, "FreeListIndex UAVDescriptorIndex: " + std::to_string(freeListIndexHandle_) + "\n");

	// フリーリスト用RWStructuredBufferの作成
	freeListBuffer_ = Resource::CreateRWBuffer(device, sizeof(uint32_t) * ParticleManager::kMaxParticle);
	freeListBuffer_->SetName("FreeListBuffer");
	freeListHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// フリーリスト用UAVの作成
	uavBufferDesc.Buffer.NumElements = ParticleManager::kMaxParticle;	// 要素数
	uavBufferDesc.Buffer.StructureByteStride = sizeof(uint32_t);		// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(freeListBuffer_->GetResource(), uavBufferDesc, freeListHandle_);
	Logger::Log(logStream, "FreeList UAVDescriptorIndex: " + std::to_string(freeListHandle_) + "\n");

	// フットプリントマップ用RWStructuredBufferの作成
	footprintMapBuffer_ = Resource::CreateRWBuffer(device, sizeof(Int4));
	footprintMapBuffer_->SetName("FootprintMapBuffer");
	footprintMapBuffer_->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	footprintMapHandle_ = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// フットプリントマップ用UAVの作成
	uavBufferDesc.Buffer.NumElements = 1;						// 要素数
	uavBufferDesc.Buffer.StructureByteStride = sizeof(Int4);	// 構造体のサイズ
	gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(footprintMapBuffer_->GetResource(), uavBufferDesc, footprintMapHandle_);
	Logger::Log(logStream, "FootprintMap UAVDescriptorIndex: " + std::to_string(footprintMapHandle_) + "\n");

	// フットプリントマップ読み戻しバッファの作成
	footprintMapReadbackBuffer_ = Resource::CreateReadbackBuffer(device, sizeof(Int4));
	footprintMapReadbackBuffer_->SetName("FootprintMapReadbackBuffer");
	footprintMapReadbackBuffer_->Map(reinterpret_cast<void **>(&colorData_));
}

World::~World() = default;

void World::Update(float deltaTime) {
	TransferDirectionalLight();
	TransferPointLight();
	TransferSpotLight();
	TransferCamera();
	TransferWorldTransform();
	TransferMaterial();
	TransferTextureData();
	TransferMeshLODData();
	TransferCullingData();
	TransferSkybox();
	TransferGrayscaleColor();
	TransferVignetteParam();
	TransferBoxFilterParam();
	TransferGaussianFilterParam();
	TransferLuminanceBasedOutlineData();
	TransferDepthBasedOutlineData();
	TransferRadialBlurParam();
	TransferDissolveParam();
	TransferPerFrame(deltaTime);
	TransferEmitterSphere();
	TransferFootprint();
	TransferFootprintMap();
}

void World::Edit() {
#ifdef USE_IMGUI
	ImGui::Text("Time: %.1f", perFrame_.time);
	ImGui::Text("DeltaTime: %.4f", perFrame_.deltaTime);
	ImGui::Text("MeshLODCount: %d", meshLODCounter_);
	ImGui::Text("CylinderCount: %d", cylinderCounter_);
	ImGui::Text("MeshInfoForAABBCount: %d", meshInfoForAABBCounter_);
	ImGui::Text("CullingMeshCount: %d", cullingConstantsData_.meshCount);

	if (ImGui::Checkbox("Culling", &isCulling_)) {
		if (isCulling_) {
			registry_->AddComponentToAll<UseCulling, Model>();
			registry_->AddComponentToAll<UseCulling, Primitive>();
			registry_->AddComponentToAll<DirtyTransform, Model>();
			registry_->AddComponentToAll<DirtyTransform, Primitive>();
		} else {
			registry_->ClearComponent<UseCulling>();
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

	if (ImGui::TreeNode("LuminanceBasedOutline")) {
		ImGui::DragFloat("Scale", &luminancePrewittFilterParam_.scale, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		if (ImGui::Button("Reset")) {
			luminancePrewittFilterParam_.scale = 1.0f;
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("DepthBasedOutline")) {
		ImGui::DragFloat("Scale", &depthPrewittFilterParam_.scale, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		if (ImGui::Button("Reset")) {
			depthPrewittFilterParam_.scale = 1.0f;
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("RadialBlur")) {
		int32_t sampleCount = static_cast<int32_t>(radialBlurParam_.sampleCount);
		ImGui::DragFloat2("Center", &radialBlurParam_.center.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		ImGui::DragFloat("BlurWidth", &radialBlurParam_.blurWidth, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		ImGui::DragInt("SampleCount", &sampleCount, 1.0f, 1, 64);
		radialBlurParam_.sampleCount = static_cast<uint32_t>(sampleCount);
		if (ImGui::Button("Reset")) {
			radialBlurParam_.center = { 0.5f, 0.5f };
			radialBlurParam_.blurWidth = 0.01f;
			radialBlurParam_.sampleCount = 10;
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Dissolve")) {
		ImGui::SliderFloat("Threshold", &dissolveParam_.threshold, 0.0f, 1.0f);
		ImGui::DragFloat("EdgeWidth", &dissolveParam_.edgeWidth, 0.01f, 0.0f, 1.0f);
		ImGui::ColorEdit3("EdgeColor", &dissolveParam_.edgeColor.x, ImGuiColorEditFlags_Float);
		if (ImGui::Button("Reset")) {
			dissolveParam_.threshold = 0.5f;
			dissolveParam_.edgeWidth = 0.03f;
			dissolveParam_.edgeColor = { 1.0f, 1.0f, 1.0f };
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
	ZeroMemory(pointLightData_, sizeof(PointLight) * kMaxPointLight);
	registry_->ForEach<PointLight>([&](uint32_t entity, PointLight *pointLight) {
		pointLightData_[pointLightCounter] = *pointLight;
		pointLightCounter++;
		}, exclude<Disabled>());
}

void World::TransferSpotLight() {
	uint32_t spotLightCounter = 0;
	ZeroMemory(spotLightData_, sizeof(SpotLight) * kMaxSpotLight);
	registry_->ForEach<SpotLight>([&](uint32_t entity, SpotLight *spotLight) {
		spotLightData_[spotLightCounter] = *spotLight;
		spotLightCounter++;
		}, exclude<Disabled>());
}

void World::TransferCamera() {
	TransformSystem transformSystem{ registry_ };
	uint32_t cameraIndex = 0;
	registry_->ForEach<Camera, QuaternionTransform>([&](uint32_t entity, Camera *camera, QuaternionTransform *transform) {
		ViewProjectionData viewProjection = MakeViewProjection(*camera, *transform);
		ParticlePerViewData particlePerViewData = {
			.viewProjection = viewProjection,
			.billboardMatrix = transform->worldMatrix
		};
		particlePerViewData.billboardMatrix.m[3][0] = 0.0f;
		particlePerViewData.billboardMatrix.m[3][1] = 0.0f;
		particlePerViewData.billboardMatrix.m[3][2] = 0.0f;
		CameraPosition cameraPosition = {
			.worldPosition = transformSystem.GetWorldPosition(entity)
		};
		Frustum frustum = MakeFrustum(viewProjection);
		if (registry_->HasComponent<MainCamera>(entity)) {
			depthMaterial_.projectionInverse = viewProjection.projection.inverse();
			constantBuffers_[static_cast<uint32_t>(ConstantBufferType::kCameraPosition)]->CopyData(&cameraPosition, sizeof(CameraPosition), 0);
			constantBuffers_[static_cast<uint32_t>(ConstantBufferType::kFrustum)]->CopyData(&frustum, sizeof(Frustum), 0);
		}
		constantBuffers_[static_cast<uint32_t>(ConstantBufferType::kViewProjection)]->CopyData(&viewProjection, sizeof(ViewProjectionData), cameraIndex + 1);
		constantBuffers_[static_cast<uint32_t>(ConstantBufferType::kParticlePerView)]->CopyData(&particlePerViewData, sizeof(ParticlePerViewData), cameraIndex);
		cameraIndex++;
		}, exclude<Disabled>());
}

void World::TransferWorldTransform() {
	registry_->ForEach<InstanceHandle, DirtyTransform>([&](uint32_t entity, InstanceHandle *instanceHandle, DirtyTransform *dirtyTransform) {
		TransformationMatrix transformationMatrix;
		Model *model = registry_->GetComponent<Model>(entity);
		if (model && !registry_->HasComponent<SkinMesh>(entity)) {
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
		instanceData_[instanceHandle->value].transformationMatrix = transformationMatrix;
		if (auto branchData = registry_->GetComponent<BranchData>(entity)) {
			instanceData_[instanceHandle->value].bottomRadius = branchData->bottomRadius;
			instanceData_[instanceHandle->value].topRadius = branchData->topRadius;
			instanceData_[instanceHandle->value].inverseBranchLength = branchData->inverseLength;
		}
		}, exclude<Disabled>());
}

void World::TransferMaterial() {
	registry_->ForEach<Material, InstanceHandle, DirtyMaterial>([&](uint32_t entity, Material *material, InstanceHandle *instanceHandle, DirtyMaterial *dirtyMaterial) {
		materialData_[instanceHandle->value] = *material;
		}, exclude<Disabled>());
}

void World::TransferTextureData() {
	registry_->ForEach<InstanceHandle, DirtyTextureData>([&](uint32_t entity, InstanceHandle *instanceHandle, DirtyTextureData *dirtyTextureData) {
		if (auto model = registry_->GetComponent<Model>(entity)) {
			for (const MeshData &mesh : model->modelData.meshes) {
				TextureData textureData{
					.textureHandle = model->textureHandle[mesh.materialIndex],
					.enableMipMaps = model->enableMipMaps[mesh.materialIndex]
				};
				textureData_[instanceHandle->value] = textureData;
			}
		} else if (auto primitive = registry_->GetComponent<Primitive>(entity)) {
			TextureData textureData{
				.textureHandle = primitive->textureHandle,
				.enableMipMaps = primitive->enableMipMaps
			};
			textureData_[instanceHandle->value] = textureData;
		}
		}, exclude<Disabled>());
}

void World::TransferMeshLODData() {
	if (registry_->GetComponentCount<DirtyMeshLOD>() > 0) {
		meshLODCounter_ = 0;
		cullingConstantsData_.meshCount = 0;
		registry_->AddComponentToAll<DirtyMeshLOD, InstanceHandle>();
	}

	for (size_t i = 0; i < static_cast<size_t>(BlendMode::kCountOfBlendMode); i++) {
		std::unordered_map<std::string, uint32_t> meshLODOffsets;
		registry_->ForEach<BlendMode, Model, DirtyMeshLOD>([&](uint32_t entity, BlendMode *blendMode, Model *model, DirtyMeshLOD *dirtyMeshLOD) {
			if (static_cast<BlendMode>(i) == *blendMode) {
				for (const MeshData &mesh : model->modelData.meshes) {
					for (const MeshLODData &lod : mesh.lods) {
						auto it = meshLODOffsets.find(lod.meshName);
						if (it == meshLODOffsets.end()) {
							meshLODOffsets.emplace(lod.meshName, meshLODCounter_);
							cullingMeshData_[cullingConstantsData_.meshCount].lodOffset = meshLODCounter_;

							meshLODData_[meshLODCounter_].indirectCommand.meshOffset = 0;
							if (registry_->HasComponent<SkinMesh>(entity)) {
								meshLODData_[meshLODCounter_].indirectCommand.vertexBufferView = skinClusterManager_->GetOutputVertexBufferView(model->skinClusterHandle);
							} else {
								meshLODData_[meshLODCounter_].indirectCommand.vertexBufferView = meshManager_->GetVertexBufferView(lod.meshName);
							}
							meshLODData_[meshLODCounter_].indirectCommand.indexBufferView = meshManager_->GetIndexBufferView(lod.meshName);
							meshLODData_[meshLODCounter_].indirectCommand.drawIndexedArguments.IndexCountPerInstance = meshManager_->GetIndexCount(lod.meshName);
							meshLODData_[meshLODCounter_].indirectCommand.drawIndexedArguments.InstanceCount = 0;
							meshLODData_[meshLODCounter_].indirectCommand.drawIndexedArguments.StartIndexLocation = 0;
							meshLODData_[meshLODCounter_].indirectCommand.drawIndexedArguments.BaseVertexLocation = 0;
							meshLODData_[meshLODCounter_].indirectCommand.drawIndexedArguments.StartInstanceLocation = 0;
							meshLODData_[meshLODCounter_].error = lod.error;
							meshLODCounter_++;
						} else {
							cullingMeshData_[cullingConstantsData_.meshCount].lodOffset = it->second;
						}
					}
					cullingConstantsData_.meshCount++;
				}
			}
			}, exclude<Disabled>());
	}

	for (size_t i = 0; i < static_cast<size_t>(BlendMode::kCountOfBlendMode); i++) {
		std::unordered_map<std::string, uint32_t> meshLODOffsets;
		registry_->ForEach<BlendMode, Primitive, DirtyMeshLOD>([&](uint32_t entity, BlendMode *blendMode, Primitive *primitive, DirtyMeshLOD *dirtyMeshLOD) {
			if (static_cast<BlendMode>(i) == *blendMode) {
				auto it = meshLODOffsets.find(primitive->meshName);
				if (it == meshLODOffsets.end()) {
					meshLODOffsets.emplace(primitive->meshName, meshLODCounter_);
					cullingMeshData_[cullingConstantsData_.meshCount].lodOffset = meshLODCounter_;

					meshLODData_[meshLODCounter_].indirectCommand.meshOffset = 0;
					meshLODData_[meshLODCounter_].indirectCommand.vertexBufferView = meshManager_->GetVertexBufferView(primitive->meshName);
					meshLODData_[meshLODCounter_].indirectCommand.indexBufferView = meshManager_->GetIndexBufferView(primitive->meshName);
					meshLODData_[meshLODCounter_].indirectCommand.drawIndexedArguments.IndexCountPerInstance = meshManager_->GetIndexCount(primitive->meshName);
					meshLODData_[meshLODCounter_].indirectCommand.drawIndexedArguments.InstanceCount = 0;
					meshLODData_[meshLODCounter_].indirectCommand.drawIndexedArguments.StartIndexLocation = 0;
					meshLODData_[meshLODCounter_].indirectCommand.drawIndexedArguments.BaseVertexLocation = 0;
					meshLODData_[meshLODCounter_].indirectCommand.drawIndexedArguments.StartInstanceLocation = 0;
					meshLODData_[meshLODCounter_].error = primitive->error;
					meshLODCounter_++;
				} else {
					cullingMeshData_[cullingConstantsData_.meshCount].lodOffset = it->second;
				}
				cullingConstantsData_.meshCount++;
			}
			}, exclude<Disabled>());
	}
}

void World::TransferCullingData() {
	if (registry_->GetComponentCount<DirtyCullingData>() > 0) {
		meshInfoForAABBCounter_ = 0;
		cylinderCounter_ = 0;
		registry_->AddComponentToAll<DirtyCullingData, InstanceHandle>();
	}

	uint32_t cullingMeshDataOffset = 0;
	uint32_t vertexOffset = 0;
	registry_->ForEach<Model, MeshType, BlendMode, InstanceHandle, DirtyCullingData>([&](uint32_t entity, Model *model, MeshType *meshType, BlendMode *blendMode, InstanceHandle *instanceHandle, DirtyCullingData *dirtyCullingData) {
		MeshInfoForAABB *meshInfoForAABB = &meshInfoForAABB_[meshInfoForAABBCounter_];
		for (const MeshData &mesh : model->modelData.meshes) {
			for (const MeshLODData &lod : mesh.lods) {
				meshInfoForAABB->vertexOffset = vertexOffset;
				meshInfoForAABB->vertexCount = static_cast<uint32_t>(lod.vertices.size());
				meshInfoForAABB->aabbIndex = cullingMeshDataOffset;
				vertexOffset += meshInfoForAABB->vertexCount;
				for (uint32_t i = 0; i < meshInfoForAABB->vertexCount; i++) {
					vertexDataForAABB_[meshInfoForAABB->vertexOffset + i].position = lod.vertices[i].position;
				}
			}
		}

		for (const MeshData &mesh : model->modelData.meshes) {
			cullingMeshData_[cullingMeshDataOffset].objectIndex = instanceHandle->value;
			cullingMeshData_[cullingMeshDataOffset].lodCount = static_cast<uint32_t>(mesh.lods.size());
			cullingMeshData_[cullingMeshDataOffset].useCulling = registry_->HasComponent<UseCulling>(entity) ? 1u : 0;
			cullingMeshDataOffset++;
		}
		meshInfoForAABBCounter_++;
		}, exclude<Disabled>());

	registry_->ForEach<Primitive, MeshType, BlendMode, InstanceHandle, DirtyCullingData>([&](uint32_t entity, Primitive *primitive, MeshType *meshType, BlendMode *blendMode, InstanceHandle *instanceHandle, DirtyCullingData *dirtyCullingData) {
		if (auto cylinder = registry_->GetComponent<Cylinder>(entity)) {
			cylinder->aabbIndex = cullingMeshDataOffset;
			cylinderData_[cylinderCounter_] = *cylinder;
			cylinderCounter_++;
		}

		cullingMeshData_[cullingMeshDataOffset].objectIndex = instanceHandle->value;
		cullingMeshData_[cullingMeshDataOffset].lodCount = 1;
		cullingMeshData_[cullingMeshDataOffset].useCulling = registry_->HasComponent<UseCulling>(entity) ? 1u : 0;
		cullingMeshDataOffset++;
		}, exclude<Disabled>());

	registry_->ForEach<MeshType, BlendMode, InstanceHandle, DirtyTransform>([&](uint32_t entity, MeshType *meshType, BlendMode *blendMode, InstanceHandle *instanceHandle, DirtyTransform *dirtyTransform) {
		if (auto eulerTransform = registry_->GetComponent<EulerTransform>(entity)) {
			cullingObjectData_[instanceHandle->value].worldMatrix = eulerTransform->worldMatrix;
		} else if (auto quaternionTransform = registry_->GetComponent<QuaternionTransform>(entity)) {
			cullingObjectData_[instanceHandle->value].worldMatrix = quaternionTransform->worldMatrix;
		} else {
			cullingObjectData_[instanceHandle->value].worldMatrix = MakeIdentity4x4();
		}

		cullingObjectData_[instanceHandle->value].meshType = *meshType;
		cullingObjectData_[instanceHandle->value].blendMode = *blendMode;
		}, exclude<Disabled>());
}

void World::TransferSkybox() {
	registry_->ForEach<Skybox, EulerTransform>([&](uint32_t entity, Skybox *skybox, EulerTransform *eulerTransform) {
		SkyboxForGPU skyboxForGPU{
			.worldMatrix = eulerTransform->worldMatrix,
			.color = skybox->color,
		};
		constantBuffers_[static_cast<size_t>(ConstantBufferType::kSkybox)]->CopyData(&skyboxForGPU, sizeof(SkyboxForGPU), 0);
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

void World::TransferLuminanceBasedOutlineData() {
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kPrewittFilterParam)]->CopyData(&luminancePrewittFilterParam_, sizeof(PrewittFilterParam), 0);
}

void World::TransferDepthBasedOutlineData() {
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kPrewittFilterParam)]->CopyData(&depthPrewittFilterParam_, sizeof(PrewittFilterParam), 1);
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kDepthMaterial)]->CopyData(&depthMaterial_, sizeof(DepthMaterial), 0);
}

void World::TransferRadialBlurParam() {
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kRadialBlurParam)]->CopyData(&radialBlurParam_, sizeof(RadialBlurParam), 0);
}

void World::TransferDissolveParam() {
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kDissolveParam)]->CopyData(&dissolveParam_, sizeof(DissolveParam), 0);
}

void World::TransferPerFrame(float deltaTime) {
	perFrame_.time++;
	perFrame_.deltaTime = deltaTime;
	constantBuffers_[static_cast<size_t>(ConstantBufferType::kPerFrame)]->CopyData(&perFrame_, sizeof(PerFrame), 0);
}

void World::TransferEmitterSphere() {
	uint32_t emitterSphereCounter = 0;
	registry_->ForEach<EmitterSphere>([&](uint32_t entity, EmitterSphere *emitterSphere) {
		constantBuffers_[static_cast<size_t>(ConstantBufferType::kEmitterSphere)]->CopyData(emitterSphere, sizeof(EmitterSphere), emitterSphereCounter);
		emitterSphereCounter++;
		}, exclude<Disabled>());
}

void World::TransferFootprint() {
	ZeroMemory(footprintData_, sizeof(FootprintForGPU) * kMaxFootprint);
	registry_->ForEach<Footprint, Collision::Sphere>([&](uint32_t entity, Footprint *footprint, Collision::Sphere *sphere) {
		footprintData_[footprint->id].worldPos = sphere->center;
		footprintData_[footprint->id].radius = sphere->radius;
		footprintData_[footprint->id].color = footprint->color;
		}, exclude<Disabled>());
}

void World::TransferFootprintMap() {
	uint32_t footprintMapCounter = 0;
	registry_->ForEach<FootprintMap>([&](uint32_t entity, FootprintMap *footprintMap) {
		constantBuffers_[static_cast<size_t>(ConstantBufferType::kFootprintMap)]->CopyData(footprintMap, sizeof(FootprintMap), footprintMapCounter);
		footprintMapCounter++;
		}, exclude<Disabled>());
}