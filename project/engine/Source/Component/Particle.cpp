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
			particleGroup.meshName = name + "Plane";
			meshManager_->CreatePlane(particleGroup.meshName);
			break;
		case MeshType::kBox:
			particleGroup.meshName = name + "Box";
			meshManager_->CreateBox(particleGroup.meshName);
			break;
		case MeshType::kRing:
			particleGroup.meshName = name + "Ring";
			meshManager_->CreateRing(particleGroup.meshName, 32, 0.5f, 0.1f);
			break;
		case MeshType::kCylinder:
			particleGroup.meshName = name + "Cylinder";
			meshManager_->CreateCylinder(particleGroup.meshName, 32, 1.0f, 1.0f, 3.0f, false);
			break;
		case MeshType::kCountOfMeshType:
			break;
		default:
			break;
	}
	particleGroup.textureHandle = textureManager_->LoadTexture(textureFileName);
	particleGroup.resourceHandle = static_cast<uint32_t>(particleResources_.size());
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
	particleResources_.emplace_back(std::move(particleResource));
	particleGroups_.insert(std::make_pair(name, particleGroup));
	Logger::Log(*logStream_, "Created particle: " + name + "\n");
}

ParticleGroup ParticleManager::FindParticleGroup(const std::string &name) {
	if (particleGroups_.contains(name)) {
		return particleGroups_.at(name);
	}
	return ParticleGroup{};
}

Resource *ParticleManager::GetParticleResource(uint32_t handle) {
	return particleResources_[handle].get();
}

void ParticleManager::UpdateEmitterSphere(float deltaTime) {
	registry_->ForEach<EmitterSphere>([&](uint32_t entity, EmitterSphere *emitterSphere) {
		emitterSphere->frequencyTime += deltaTime;
		if (emitterSphere->frequency <= emitterSphere->frequencyTime) {
			emitterSphere->frequencyTime -= emitterSphere->frequency;
			emitterSphere->emit = true;
		} else {
			emitterSphere->emit = false;
		}
		}, exclude<Disabled>());
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

void EmitterSphereInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("EmitterSphere")) {
		EmitterSphere *emitterSphere = registry_->GetComponent<EmitterSphere>(entity);
		if (emitterSphere) {
			ImGui::DragFloat3("Translate", &emitterSphere->translate.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat("Radius", &emitterSphere->radius, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragInt("Count", reinterpret_cast<int *>(&emitterSphere->count), 1.0f, 1, std::numeric_limits<int>::max());
			ImGui::DragFloat("Frequency", &emitterSphere->frequency, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::Text("FrequencyTime: %.3f", emitterSphere->frequencyTime);
			ImGui::Checkbox("Emit", reinterpret_cast<bool *>(&emitterSphere->emit));
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