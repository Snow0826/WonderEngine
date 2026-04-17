#define NOMINMAX
#include "Particle.h"
#include "Device.h"
#include "Resource.h"
#include "Mesh.h"
#include "Texture.h"
#include "EntityComponentSystem.h"
#include "Camera.h"
#include "Logger.h"
#include "Random.h"
#include "RigidBody.h"

ParticleManager::ParticleManager(Device *device, TextureManager *textureManager, MeshManager *meshManager, std::ofstream *logStream)
	: device_(device)
	, textureManager_(textureManager)
	, meshManager_(meshManager)
	, logStream_(logStream) {}

ParticleManager::~ParticleManager() = default;

void ParticleManager::CreateParticleGroup(const std::string &name, const std::string &textureFileName) {
	// すでに読み込まれている場合は何もしない
	if (particleGroups_.contains(name)) {
		Logger::Log(*logStream_, "Particle already created: " + name + " with handle:" + std::to_string(particleGroups_.at(name).instanceHandle) + "\n");
		return;
	}

	// パーティクルグループの作成
	ParticleGroup particleGroup;
	particleGroup.meshHandle = meshManager_->CreatePlane();
	particleGroup.textureHandle = textureManager_->LoadTexture(textureFileName);
	particleGroup.textureFileName = textureFileName;
	particleGroup.particleDataName = name;

	// パーティクル用構造化バッファの作成
	std::unique_ptr<Resource> resource = Resource::CreateUploadBuffer(device_, sizeof(ParticleForGPU) * kMaxParticle);

	particleDataList_.insert(std::make_pair(particleGroup.particleDataName, nullptr));
	resource->Map(reinterpret_cast<void **>(&particleDataList_.at(particleGroup.particleDataName)));

	// パーティクル用SRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;										// 構造化バッファなのでフォーマットなし
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;							// バッファビュー
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;	// 標準設定
	srvDesc.Buffer.FirstElement = 0;											// 先頭の要素
	srvDesc.Buffer.NumElements = kMaxParticle;									// 要素数
	srvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);				// 構造体のサイズ
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;							// 特になし

	// パーティクル用SRVの作成
	particleGroup.instanceHandle = device_->GetGpuCbvSrvUavDescriptorHeap()->AllocateDescriptor();
	device_->GetGpuCbvSrvUavDescriptorHeap()->CreateShaderResourceView(resource->GetResource(), srvDesc, particleGroup.instanceHandle);
	particleResources_.insert(std::make_pair(particleGroup.particleDataName, std::move(resource)));

	// パーティクルグループの登録
	particleGroups_.insert(std::make_pair(particleGroup.particleDataName, particleGroup));
	Logger::Log(*logStream_, "Created particle: " + particleGroup.particleDataName + " with handle: " + std::to_string(particleGroup.instanceHandle) + "\n");
}

ParticleGroup ParticleManager::FindParticleGroup(const std::string &name) {
	ParticleGroup particleGroup;
	if (particleGroups_.contains(name)) {
		particleGroup = particleGroups_.at(name);
	}
	return particleGroup;
}

void ParticleManager::Emit(uint32_t entity) {
	Emitter *emitter = registry_->GetComponent<Emitter>(entity);
	ParticleGroup *particleGroup = registry_->GetComponent<ParticleGroup>(entity);
	if (!emitter || !particleGroup) {
		return;
	}

	emitter->frequencyTime += kDeltaTime;
	if (emitter->frequencyTime >= emitter->frequency) {
		emitter->frequencyTime -= emitter->frequency;
		for (size_t i = 0; i < emitter->count; i++) {
			Particle particle = CreateParticle(entity);
			particleGroup->particles.emplace_back(particle);
		}
	}
}

void ParticleManager::UpdateParticle() {
	registry_->ForEach<Transform, RenderingCamera>([&](uint32_t entity, Transform *transform, RenderingCamera *activeCamera) {
		registry_->ForEach<ParticleGroup>([&](uint32_t entity, ParticleGroup *particleGroup) {
			particleGroup->numInstance = 0;

			Matrix4x4 billboardMatrix;
			if (particleGroup->isBillboard) {
				billboardMatrix = transform->worldMatrix;
				billboardMatrix.m[3][0] = 0.0f;
				billboardMatrix.m[3][1] = 0.0f;
				billboardMatrix.m[3][2] = 0.0f;
			} else {
				billboardMatrix = MakeIdentity4x4();
			}

			Field *field = registry_->GetComponent<Field>(entity);
			for (std::list<Particle>::iterator particleIterator = particleGroup->particles.begin(); particleIterator != particleGroup->particles.end();) {
				if ((*particleIterator).age >= (*particleIterator).lifeTime) {
					particleIterator = particleGroup->particles.erase(particleIterator);
					continue;
				}

				if (particleGroup->numInstance < kMaxParticle) {
					if (field) {
						if (IsCollision(field->area, (*particleIterator).transform.translate)) {
							(*particleIterator).velocity += field->acceleration * kDeltaTime;
							(*particleIterator).collidedField = true;
						}

						if ((*particleIterator).collidedField) {
							(*particleIterator).angle += field->angularVelocity * kDeltaTime;
							Vector3 right, up;
							MakeBasis((*particleIterator).velocity, right, up);
							Vector3 circle = std::cos((*particleIterator).angle) * field->radius * right + std::sin((*particleIterator).angle) * field->radius * up;
							(*particleIterator).transform.translate += circle;
						}
					}

					(*particleIterator).transform.translate += (*particleIterator).velocity * kDeltaTime;
					(*particleIterator).age += kDeltaTime;
					Matrix4x4 scaleMatrix = MakeScaleMatrix((*particleIterator).transform.scale);
					Matrix4x4 translateMatrix = MakeTranslateMatrix((*particleIterator).transform.translate);
					(*particleIterator).transform.worldMatrix = scaleMatrix * billboardMatrix * translateMatrix;
					particleDataList_.at(particleGroup->particleDataName)[particleGroup->numInstance].worldMatrix = (*particleIterator).transform.worldMatrix;
					particleDataList_.at(particleGroup->particleDataName)[particleGroup->numInstance].color = (*particleIterator).color;
					float alpha = 1.0f - ((*particleIterator).age / (*particleIterator).lifeTime);
					particleDataList_.at(particleGroup->particleDataName)[particleGroup->numInstance].color.w = alpha;
					++particleGroup->numInstance;
				}

				++particleIterator;
			}
			}, exclude<Disabled>());
		}, exclude<Disabled>());
}

Particle ParticleManager::CreateParticle(uint32_t entity) {
	Emitter *emitter = registry_->GetComponent<Emitter>(entity);
	Particle particle;
	particle.transform.scale = Random::generate(emitter->scale.min, emitter->scale.max);
	particle.transform.translate = emitter->transform.translate + Random::generate(emitter->area.min, emitter->area.max);
	particle.velocity = Random::generate(emitter->velocity.min, emitter->velocity.max);
	particle.color = Random::generate(emitter->color.min, emitter->color.max);
	particle.lifeTime = Random::generate(emitter->lifeTime.min, emitter->lifeTime.max);
	particle.age = 0.0f;
	particle.angle = 0.0f;
	particle.collidedField = false;
	return particle;
}

void ParticleGroupInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("ParticleGroup")) {
		ParticleGroup *particleGroup = registry_->GetComponent<ParticleGroup>(entity);
		if (particleGroup) {
			ImGui::Checkbox("isBillboard", &particleGroup->isBillboard);
			particleGroup->textureHandle = textureManager_->EditTexture("Texture", particleGroup->textureFileName);
			ImGui::Text("NumParticle: %d", particleGroup->numInstance);
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
			ImGui::DragFloat3("Translate", &emitter->transform.translate.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::SliderAngle("RotateX", &emitter->transform.rotate.x, -360.0f, 360.0f);
			ImGui::SliderAngle("RotateY", &emitter->transform.rotate.y, -360.0f, 360.0f);
			ImGui::SliderAngle("RotateZ", &emitter->transform.rotate.z, -360.0f, 360.0f);
			ImGui::DragFloat3("Scale", &emitter->transform.scale.x, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat3("AreaMin", &emitter->area.min.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("AreaMax", &emitter->area.max.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("ScaleMin", &emitter->scale.min.x, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat3("ScaleMax", &emitter->scale.max.x, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat3("VelocityMin", &emitter->velocity.min.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("VelocityMax", &emitter->velocity.max.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
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
			ImGui::DragFloat3("AreaMin", &field->area.min.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("AreaMax", &field->area.max.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("Acceleration", &field->acceleration.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat("AngularVelocity", &field->angularVelocity, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat("Radius", &field->radius, 0.01f, 0.0f, std::numeric_limits<float>::max());
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}