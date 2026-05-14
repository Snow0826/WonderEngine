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
	world_->GetStructuredBuffer(StructuredBufferType::kObject)->Map(reinterpret_cast<void **>(&cullingObjectData_));
	world_->GetStructuredBuffer(StructuredBufferType::kMesh)->Map(reinterpret_cast<void **>(&cullingMeshData_));
	world_->GetCommandBufferUpload()->Map(reinterpret_cast<void **>(&meshLODData_));
}

IndirectCommandHandle IndirectCommandManager::AddIndirectCommand(uint32_t entity) {
	IndirectCommandHandle indirectCommandHandle;
	Model *model = registry_->GetComponent<Model>(entity);
	Object *object = registry_->GetComponent<Object>(entity);
	if (!model || !object) {
		return indirectCommandHandle;
	}

	for (const MeshData &mesh : model->modelData.meshes) {
		meshCounter_++;
		for (const MeshLODData &meshLODData : mesh.lods) {
			uint32_t handle = static_cast<uint32_t>(entities_.size());
			entities_.emplace_back(entity);
			meshLODData_[handle].indirectCommand.cbv[0] = world_->GetConstantBuffer(ConstantBufferType::kTransform)->GetGPUVirtualAddress(object->handle);
			meshLODData_[handle].indirectCommand.cbv[1] = world_->GetConstantBuffer(ConstantBufferType::kMaterial)->GetGPUVirtualAddress(object->handle);
			meshLODData_[handle].indirectCommand.textureData.textureHandle = model->textureHandle[mesh.materialIndex];
			meshLODData_[handle].indirectCommand.textureData.enableMipMaps = model->enableMipMaps[mesh.materialIndex];
			meshLODData_[handle].indirectCommand.vertexBufferView = meshManager_->GetVertexBufferView(meshLODData.handle);
			meshLODData_[handle].indirectCommand.indexBufferView = meshManager_->GetIndexBufferView(meshLODData.handle);
			meshLODData_[handle].indirectCommand.drawIndexedArguments.IndexCountPerInstance = meshManager_->GetIndexCount(meshLODData.handle);
			meshLODData_[handle].indirectCommand.drawIndexedArguments.InstanceCount = 1;
			meshLODData_[handle].indirectCommand.drawIndexedArguments.StartIndexLocation = 0;
			meshLODData_[handle].indirectCommand.drawIndexedArguments.BaseVertexLocation = 0;
			meshLODData_[handle].indirectCommand.drawIndexedArguments.StartInstanceLocation = 0;
			meshLODData_[handle].error = meshLODData.error;
			indirectCommandHandle.handles.emplace_back(handle);
		}
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
			cullingObjectData_[removeIndex] = cullingObjectData_[lastIndex];
			cullingMeshData_[removeIndex] = cullingMeshData_[lastIndex];
			meshLODData_[removeIndex] = meshLODData_[lastIndex];
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
		cullingObjectData_[handle].blendMode = *blendMode;
	}
}

void IndirectCommandManager::UpdateCullingData() {
	uint32_t cullingMeshDataOffset = 0;
	uint32_t meshLODDataOffset = 0;
	registry_->ForEach<Model, Transform, BlendMode, Object, DirtyTransform>([&](uint32_t entity, Model *model, Transform *transform, BlendMode *blendMode, Object *object, DirtyTransform *dirtyTransform) {
		cullingObjectData_[object->handle].worldMatrix = transform->worldMatrix;
		cullingObjectData_[object->handle].blendMode = *blendMode;
		for (const MeshData &mesh : model->modelData.meshes) {
			cullingMeshData_[cullingMeshDataOffset] = {
				.aabb = {
					.min = { mesh.aabb.min.x, mesh.aabb.min.y, mesh.aabb.min.z, 1.0f },
					.max = { mesh.aabb.max.x, mesh.aabb.max.y, mesh.aabb.max.z, 1.0f }
				},
				.objectHandle = object->handle,
				.lodOffset = meshLODDataOffset,
				.lodCount = static_cast<uint32_t>(mesh.lods.size()),
				.useCulling = registry_->HasComponent<UseCulling>(entity) ? 1u : 0
			};
			meshLODDataOffset += static_cast<uint32_t>(mesh.lods.size());
			cullingMeshDataOffset++;
		}
		});
}

void IndirectCommandManager::Debug() const {
#ifdef USE_IMGUI
	ImGui::Text("IndirectCommand Count: %zu", entities_.size());
#endif // USE_IMGUI
}