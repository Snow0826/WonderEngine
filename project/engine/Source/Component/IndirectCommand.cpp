#include "IndirectCommand.h"
#include "EntityComponentSystem.h"
#include "World.h"
#include "Resource.h"
#include "ConstantBuffer.h"
#include "SkinCluster.h"
#include "BlendMode.h"
#include "Model.h"
#include "Primitive.h"
#include "Object.h"
#include "Skybox.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

IndirectCommandManager::IndirectCommandManager(Registry *registry, World *world, MeshManager *meshManager, SkinClusterManager *skinClusterManager)
	: registry_(registry), world_(world), meshManager_(meshManager), skinClusterManager_(skinClusterManager) {
	world_->GetStructuredBuffer(StructuredBufferType::kCullingMeshData)->Map(reinterpret_cast<void **>(&cullingMeshData_));
	world_->GetStructuredBuffer(StructuredBufferType::kCullingObjectData)->Map(reinterpret_cast<void **>(&cullingObjectData_));
	world_->GetCommandBufferUpload()->Map(reinterpret_cast<void **>(&meshLODData_));
}

IndirectCommandHandle IndirectCommandManager::AddIndirectCommand(uint32_t entity) {
	IndirectCommandHandle indirectCommandHandle;
	Model *model = registry_->GetComponent<Model>(entity);
	Primitive *primitive = registry_->GetComponent<Primitive>(entity);
	Object *object = registry_->GetComponent<Object>(entity);
	if (!object) {
		return indirectCommandHandle;
	}

	if (model) {
		for (const MeshData &mesh : model->modelData.meshes) {
			meshCounter_++;
			for (const MeshLODData &meshLODData : mesh.lods) {
				uint32_t handle = static_cast<uint32_t>(entities_.size());
				entities_.emplace_back(entity);
				meshLODData_[handle].indirectCommand.cbv[0] = world_->GetConstantBuffer(ConstantBufferType::kTransform)->GetGPUVirtualAddress(object->handle);
				meshLODData_[handle].indirectCommand.cbv[1] = world_->GetConstantBuffer(ConstantBufferType::kMaterial)->GetGPUVirtualAddress(object->handle);
				meshLODData_[handle].indirectCommand.textureData.textureHandle = model->textureHandle[mesh.materialIndex];
				meshLODData_[handle].indirectCommand.textureData.enableMipMaps = model->enableMipMaps[mesh.materialIndex];
				if (registry_->HasComponent<SkinMesh>(entity)) {
					meshLODData_[handle].indirectCommand.vertexBufferView = skinClusterManager_->GetOutputVertexBufferView(model->skinClusterHandle);
				} else {
					meshLODData_[handle].indirectCommand.vertexBufferView = meshManager_->GetVertexBufferView(meshLODData.handle);
				}
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
	} else if (primitive) {
		meshCounter_++;
		uint32_t handle = static_cast<uint32_t>(entities_.size());
		entities_.emplace_back(entity);
		meshLODData_[handle].indirectCommand.cbv[0] = world_->GetConstantBuffer(ConstantBufferType::kTransform)->GetGPUVirtualAddress(object->handle);
		meshLODData_[handle].indirectCommand.cbv[1] = world_->GetConstantBuffer(ConstantBufferType::kMaterial)->GetGPUVirtualAddress(object->handle);
		meshLODData_[handle].indirectCommand.textureData.textureHandle = primitive->textureHandle;
		meshLODData_[handle].indirectCommand.textureData.enableMipMaps = primitive->enableMipMaps;
		meshLODData_[handle].indirectCommand.vertexBufferView = meshManager_->GetVertexBufferView(primitive->meshHandle);
		meshLODData_[handle].indirectCommand.indexBufferView = meshManager_->GetIndexBufferView(primitive->meshHandle);
		meshLODData_[handle].indirectCommand.drawIndexedArguments.IndexCountPerInstance = meshManager_->GetIndexCount(primitive->meshHandle);
		meshLODData_[handle].indirectCommand.drawIndexedArguments.InstanceCount = 1;
		meshLODData_[handle].indirectCommand.drawIndexedArguments.StartIndexLocation = 0;
		meshLODData_[handle].indirectCommand.drawIndexedArguments.BaseVertexLocation = 0;
		meshLODData_[handle].indirectCommand.drawIndexedArguments.StartInstanceLocation = 0;
		meshLODData_[handle].error = primitive->error;
		indirectCommandHandle.handles.emplace_back(handle);
	}
	return indirectCommandHandle;
}

void IndirectCommandManager::RemoveIndirectCommand(uint32_t entity) {
	IndirectCommandHandle *indirectCommandHandle = registry_->GetComponent<IndirectCommandHandle>(entity);
	if (!indirectCommandHandle) {
		return;
	}

	while (!indirectCommandHandle->handles.empty()) {
		uint32_t removeIndex = indirectCommandHandle->handles.back();
		indirectCommandHandle->handles.pop_back();

		uint32_t lastIndex = static_cast<uint32_t>(entities_.size() - 1);
		if (removeIndex != lastIndex) {
			uint32_t movedEntity = entities_[lastIndex];
			cullingObjectData_[removeIndex] = cullingObjectData_[lastIndex];
			cullingMeshData_[removeIndex] = cullingMeshData_[lastIndex];
			meshLODData_[removeIndex] = meshLODData_[lastIndex];
			entities_[removeIndex] = movedEntity;
			if (auto *movedHandle = registry_->GetComponent<IndirectCommandHandle>(movedEntity)) {
				for (uint32_t &index : movedHandle->handles) {
					if (index == lastIndex) {
						index = removeIndex;
						break;
					}
				}
			}
		}

		entities_.pop_back();
	}
}

void IndirectCommandManager::UpdateCullingData() {
	uint32_t cullingMeshDataOffset = 0;
	uint32_t meshLODDataOffset = 0;
	registry_->ForEach<Model, MeshType, BlendMode, Object>([&](uint32_t entity, Model *model, MeshType *meshType, BlendMode *blendMode, Object *object) {
		EulerTransform *eulerTransform = registry_->GetComponent<EulerTransform>(entity);
		QuaternionTransform *quaternionTransform = registry_->GetComponent<QuaternionTransform>(entity);
		if (eulerTransform) {
			cullingObjectData_[object->handle].worldMatrix = eulerTransform->worldMatrix;
		} else if (quaternionTransform) {
			cullingObjectData_[object->handle].worldMatrix = quaternionTransform->worldMatrix;
		} else {
			cullingObjectData_[object->handle].worldMatrix = MakeIdentity4x4();
		}
		cullingObjectData_[object->handle].meshType = *meshType;
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

	registry_->ForEach<Primitive, MeshType, BlendMode, Object>([&](uint32_t entity, Primitive *primitive, MeshType *meshType, BlendMode *blendMode, Object *object) {
		EulerTransform *eulerTransform = registry_->GetComponent<EulerTransform>(entity);
		QuaternionTransform *quaternionTransform = registry_->GetComponent<QuaternionTransform>(entity);
		if (eulerTransform) {
			cullingObjectData_[object->handle].worldMatrix = eulerTransform->worldMatrix;
		} else if (quaternionTransform) {
			cullingObjectData_[object->handle].worldMatrix = quaternionTransform->worldMatrix;
		} else {
			cullingObjectData_[object->handle].worldMatrix = MakeIdentity4x4();
		}
		cullingObjectData_[object->handle].meshType = *meshType;
		cullingObjectData_[object->handle].blendMode = *blendMode;
		cullingMeshData_[cullingMeshDataOffset] = {
			.aabb = {
				.min = { primitive->aabb.min.x, primitive->aabb.min.y, primitive->aabb.min.z, 1.0f },
				.max = { primitive->aabb.max.x, primitive->aabb.max.y, primitive->aabb.max.z, 1.0f }
			},
			.objectHandle = object->handle,
			.lodOffset = meshLODDataOffset,
			.lodCount = 1,
			.useCulling = registry_->HasComponent<UseCulling>(entity) ? 1u : 0
		};
		meshLODDataOffset++;
		cullingMeshDataOffset++;
		});
}

void IndirectCommandManager::Debug() const {
#ifdef USE_IMGUI
	ImGui::Text("IndirectCommand Count: %zu", entities_.size());
#endif // USE_IMGUI
}