#define NOMINMAX
#include "Particle.h"
#include "Device.h"
#include "Mesh.h"
#include "Texture.h"
#include "Resource.h"
#include "EntityComponentSystem.h"
#include "Logger.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

ParticleManager::ParticleManager(Device *device, TextureManager *textureManager, MeshManager *meshManager, std::ofstream *logStream)
	: device_(device)
	, textureManager_(textureManager)
	, meshManager_(meshManager)
	, logStream_(logStream) {
}

ParticleManager::~ParticleManager() = default;

void ParticleManager::CreateParticleGroup(const std::string &name, MeshType meshType, const std::string &textureFileName) {
	// すでに読み込まれている場合は何もしない
	if (particleGroups_.contains(name)) {
		Logger::Log(*logStream_, "Particle already created: " + name + "\n");
		return;
	}

	// パーティクルグループの作成
	DescriptorHeap *gpuCbvSrvUavDescriptorHeap = device_->GetGpuCbvSrvUavDescriptorHeap();
	ParticleGroup particleGroup;
	switch (meshType) {
		case MeshType::kModel:
			break;
		case MeshType::kPlane:
			particleGroup.meshHandle = meshManager_->CreatePlane();
			break;
		case MeshType::kBox:
			particleGroup.meshHandle = meshManager_->CreateBox();
			break;
		case MeshType::kRing:
			particleGroup.meshHandle = meshManager_->CreateRing(32, 0.5f, 0.1f);
			break;
		case MeshType::kCylinder:
			particleGroup.meshHandle = meshManager_->CreateCylinder(32, 1.0f, 1.0f, 3.0f, false);
			break;
		case MeshType::kCountOfMeshType:
			break;
		default:
			break;
	}
	particleGroup.textureHandle = textureManager_->LoadTexture(textureFileName);
	particleGroup.srvHandle = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	particleGroup.uavHandle = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();
	particleGroup.textureFileName = textureFileName;

	// パーティクルリソースの作成
	std::unique_ptr<Resource> particleResource = Resource::CreateRWBuffer(device_, sizeof(Particle) * kMaxParticle);

	// パーティクル用SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = kMaxParticle;
	srvDesc.Buffer.StructureByteStride = sizeof(Particle);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(particleResource->GetResource(), srvDesc, particleGroup.srvHandle);
	Logger::Log(*logStream_, "Particle SRVDescriptorIndex: " + std::to_string(particleGroup.srvHandle) + "\n");

	// パーティクル用UAVの作成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = kMaxParticle;
	uavDesc.Buffer.StructureByteStride = sizeof(Particle);
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	gpuCbvSrvUavDescriptorHeap->CreateUnorderedAccessView(particleResource->GetResource(), uavDesc, particleGroup.uavHandle);
	Logger::Log(*logStream_, "Particle UAVDescriptorIndex: " + std::to_string(particleGroup.uavHandle) + "\n");

	// パーティクルグループとバッファをマップに登録
	particleResources_.insert(std::make_pair(name, std::move(particleResource)));
	particleGroups_.insert(std::make_pair(name, particleGroup));
	Logger::Log(*logStream_, "Created particle: " + name + "\n");
}

ParticleGroup ParticleManager::FindParticleGroup(const std::string &name) {
	ParticleGroup particleGroup;
	if (particleGroups_.contains(name)) {
		particleGroup = particleGroups_.at(name);
	}
	return particleGroup;
}

void ParticleGroupInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("ParticleGroup")) {
		ParticleGroup *particleGroup = registry_->GetComponent<ParticleGroup>(entity);
		if (particleGroup) {
			particleGroup->textureHandle = textureManager_->EditTexture("Texture", particleGroup->textureFileName);
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}

void EmitterInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Emitter")) {
		Emitter *emitter = registry_->GetComponent<Emitter>(entity);
		if (emitter) {
			ImGui::DragFloat3("Translate", &emitter->transform.translate.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::SliderAngle("RotateX", &emitter->transform.rotate.x, -360.0f, 360.0f);
			ImGui::SliderAngle("RotateY", &emitter->transform.rotate.y, -360.0f, 360.0f);
			ImGui::SliderAngle("RotateZ", &emitter->transform.rotate.z, -360.0f, 360.0f);
			ImGui::DragFloat3("Scale", &emitter->transform.scale.x, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat3("AreaMin", &emitter->area.min.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("AreaMax", &emitter->area.max.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("ScaleMin", &emitter->scale.min.x, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat3("ScaleMax", &emitter->scale.max.x, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::SliderAngle("RotateMinX", &emitter->rotate.min.x, -360.0f, 360.0f);
			ImGui::SliderAngle("RotateMinY", &emitter->rotate.min.y, -360.0f, 360.0f);
			ImGui::SliderAngle("RotateMinZ", &emitter->rotate.min.z, -360.0f, 360.0f);
			ImGui::SliderAngle("RotateMaxX", &emitter->rotate.max.x, -360.0f, 360.0f);
			ImGui::SliderAngle("RotateMaxY", &emitter->rotate.max.y, -360.0f, 360.0f);
			ImGui::SliderAngle("RotateMaxZ", &emitter->rotate.max.z, -360.0f, 360.0f);
			ImGui::DragFloat3("VelocityMin", &emitter->velocity.min.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("VelocityMax", &emitter->velocity.max.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::ColorEdit4("ColorMin", &emitter->color.min.x);
			ImGui::ColorEdit4("ColorMax", &emitter->color.max.x);
			ImGui::DragFloat("LifeTimeMin", &emitter->lifeTime.min, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat("LifeTimeMax", &emitter->lifeTime.max, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat("Frequency", &emitter->frequency, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragInt("Count", reinterpret_cast<int *>(&emitter->count), 1.0f, 1, std::numeric_limits<int>	::max());
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}

void FieldInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Field")) {
		Field *field = registry_->GetComponent<Field>(entity);
		if (field) {
			ImGui::DragFloat3("AreaMin", &field->area.min.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("AreaMax", &field->area.max.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("Acceleration", &field->acceleration.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat("AngularVelocity", &field->angularVelocity, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat("Radius", &field->radius, 0.01f, 0.0f, std::numeric_limits<float>::max());
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}