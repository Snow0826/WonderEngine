#include "IndirectCommand.h"
#include "EntityComponentSystem.h"
#include "World.h"
#include "Resource.h"
#include "ConstantBuffer.h"
#include "BlendMode.h"
#include "Model.h"
#include "Object.h"
#include "Transform.h"

IndirectCommandManager::IndirectCommandManager(Registry *registry, World *world, MeshManager *meshManager) : registry_(registry), world_(world), meshManager_(meshManager) {
	world_->GetStructuredBuffer(StructuredBufferType::kCulling)->Map(reinterpret_cast<void **>(&cullingData_));
	world_->GetStructuredBuffer(StructuredBufferType::kBlendMode)->Map(reinterpret_cast<void **>(&blendModeData_));
	world_->GetCommandBufferUpload()->Map(reinterpret_cast<void **>(&indirectCommandData_));
}

IndirectCommandHandle IndirectCommandManager::AddIndirectCommand(uint32_t entity) {
	IndirectCommandHandle indirectCommandHandle;
	BlendMode *blendMode = registry_->GetComponent<BlendMode>(entity);
	Model *model = registry_->GetComponent<Model>(entity);
	Object *object = registry_->GetComponent<Object>(entity);
	if (!blendMode || !model || !object) {
		return indirectCommandHandle;
	}

	for (size_t i = 0; i < model->modelData.meshes.size(); i++) {
		uint32_t handle = static_cast<uint32_t>(entities_.size());
		entities_.emplace_back(entity);
		blendModeData_[handle].blendMode = *blendMode;
		indirectCommandData_[handle].cbv[0] = world_->GetConstantBuffer(ConstantBufferType::kTransform)->GetGPUVirtualAddress(object->handle);
		indirectCommandData_[handle].cbv[1] = world_->GetConstantBuffer(ConstantBufferType::kMaterial)->GetGPUVirtualAddress(object->handle);
		indirectCommandData_[handle].textureData.textureHandle = model->textureHandle[model->modelData.meshes[i].materialIndex];
		indirectCommandData_[handle].textureData.enableMipMaps = model->enableMipMaps[model->modelData.meshes[i].materialIndex];
		indirectCommandData_[handle].vertexBufferView = meshManager_->GetVertexBufferView(model->meshHandle[i]);
		indirectCommandData_[handle].indexBufferView = meshManager_->GetIndexBufferView(model->meshHandle[i]);
		indirectCommandData_[handle].drawIndexedArguments.IndexCountPerInstance = meshManager_->GetIndexCount(model->meshHandle[i]);
		indirectCommandData_[handle].drawIndexedArguments.InstanceCount = 1;
		indirectCommandData_[handle].drawIndexedArguments.StartIndexLocation = 0;
		indirectCommandData_[handle].drawIndexedArguments.BaseVertexLocation = 0;
		indirectCommandData_[handle].drawIndexedArguments.StartInstanceLocation = 0;
		indirectCommandHandle.handles.emplace_back(handle);
	}
	return indirectCommandHandle;
}

void IndirectCommandManager::RemoveIndirectCommand(uint32_t entity) {
	IndirectCommandHandle *indirectCommandHandle = registry_->GetComponent<IndirectCommandHandle>(entity);
	IndirectCommandHandle *lastIndirectCommandHandle = registry_->GetComponent<IndirectCommandHandle>(entities_.back());
	if (!indirectCommandHandle || !lastIndirectCommandHandle) {
		return;
	}
	uint32_t lastIndex = static_cast<uint32_t>(entities_.size() - 1);
	for (uint32_t removeIndex : indirectCommandHandle->handles) {
		if (removeIndex != lastIndex) {
			// 最後尾のコマンドと入れ替える
			cullingData_[removeIndex] = cullingData_[lastIndex];
			blendModeData_[removeIndex] = blendModeData_[lastIndex];
			indirectCommandData_[removeIndex] = indirectCommandData_[lastIndex];
			entities_[removeIndex] = entities_.back();

			// 入れ替えた間接コマンドのハンドルを更新する
			for (uint32_t &swappedIndex : lastIndirectCommandHandle->handles) {
				if (swappedIndex == lastIndex) {
					swappedIndex = removeIndex;
					break;
				}
			}
		}
		entities_.pop_back();
	}
	indirectCommandHandle->handles.clear();
}

void IndirectCommandManager::SetBlendModeData(uint32_t entity) {
	IndirectCommandHandle *indirectCommandHandle = registry_->GetComponent<IndirectCommandHandle>(entity);
	BlendMode *blendMode = registry_->GetComponent<BlendMode>(entity);
	if (!indirectCommandHandle || !blendMode) {
		return;
	}
	for (uint32_t handle : indirectCommandHandle->handles) {
		blendModeData_[handle].blendMode = *blendMode;
	}
}

void IndirectCommandManager::UpdateCullingData() {
	registry_->ForEach<Model, IndirectCommandHandle, DirtyTransform>([this](uint32_t entity, Model *model, IndirectCommandHandle *indirectCommandHandle, DirtyTransform *dirtyTransform) {
		for (size_t i = 0; i < model->modelData.meshes.size(); i++) {
			cullingData_[indirectCommandHandle->handles[i]] = {
				.aabb = {
					.min = {
						model->modelData.meshes[i].worldCollisionData.aabb.min.x,
						model->modelData.meshes[i].worldCollisionData.aabb.min.y,
						model->modelData.meshes[i].worldCollisionData.aabb.min.z,
						1.0f
					},
					.max = {
						model->modelData.meshes[i].worldCollisionData.aabb.max.x,
						model->modelData.meshes[i].worldCollisionData.aabb.max.y,
						model->modelData.meshes[i].worldCollisionData.aabb.max.z,
						1.0f
					}
				},
				.useCulling = registry_->HasComponent<UseCulling>(entity) ? 1u : 0
			};
		}
		});
}

void IndirectCommandManager::Debug() const {
#ifdef USE_IMGUI
	ImGui::Text("IndirectCommand Count: %zu", entities_.size());
#endif // USE_IMGUI
}