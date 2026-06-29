#include "IndirectCommand.h"
#include "EntityComponentSystem.h"
#include "World.h"
#include "Resource.h"
#include "ConstantBuffer.h"
#include "SkinCluster.h"
#include "BlendMode.h"
#include "Model.h"
#include "Plane.h"
#include "Box.h"
#include "Cylinder.h"
#include "Ring.h"
#include "Object.h"
#include "Skybox.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

IndirectCommandManager::IndirectCommandManager(Registry *registry, World *world, MeshManager *meshManager, SkinClusterManager *skinClusterManager)
	: registry_(registry), world_(world), meshManager_(meshManager), skinClusterManager_(skinClusterManager) {
	world_->GetStructuredBuffer(StructuredBufferType::kObject)->Map(reinterpret_cast<void **>(&cullingObjectData_));
	world_->GetStructuredBuffer(StructuredBufferType::kMesh)->Map(reinterpret_cast<void **>(&cullingMeshData_));
	world_->GetCommandBufferUpload()->Map(reinterpret_cast<void **>(&meshLODData_));
}

IndirectCommandHandle IndirectCommandManager::AddIndirectCommand(uint32_t entity) {
	IndirectCommandHandle indirectCommandHandle;
	Model *model = registry_->GetComponent<Model>(entity);
	Plane *plane = registry_->GetComponent<Plane>(entity);
	Box *box = registry_->GetComponent<Box>(entity);
	Ring *ring = registry_->GetComponent<Ring>(entity);
	Cylinder *cylinder = registry_->GetComponent<Cylinder>(entity);
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
				meshLODData_[handle].indirectCommand.matrixPalettehandle = skinClusterManager_->GetPaletteSRVHandle(model->skinClusterHandle);
				meshLODData_[handle].indirectCommand.textureData.textureHandle = model->textureHandle[mesh.materialIndex];
				registry_->ForEach<Skybox>([&](uint32_t skyboxEntity, Skybox *skybox) {
					meshLODData_[handle].indirectCommand.textureData.environmentMapHandle = skybox->textureHandle;
					}, exclude<Disabled>());
				meshLODData_[handle].indirectCommand.textureData.enableMipMaps = model->enableMipMaps[mesh.materialIndex];
				meshLODData_[handle].indirectCommand.vertexBufferView = meshManager_->GetVertexBufferView(meshLODData.handle);
				meshLODData_[handle].indirectCommand.influenceBufferView = skinClusterManager_->GetInfluenceBufferView(model->skinClusterHandle);
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
	} else if (plane) {
		meshCounter_++;
		uint32_t handle = static_cast<uint32_t>(entities_.size());
		entities_.emplace_back(entity);
		meshLODData_[handle].indirectCommand.cbv[0] = world_->GetConstantBuffer(ConstantBufferType::kTransform)->GetGPUVirtualAddress(object->handle);
		meshLODData_[handle].indirectCommand.cbv[1] = world_->GetConstantBuffer(ConstantBufferType::kMaterial)->GetGPUVirtualAddress(object->handle);
		meshLODData_[handle].indirectCommand.textureData.textureHandle = plane->textureHandle;
		registry_->ForEach<Skybox>([&](uint32_t skyboxEntity, Skybox *skybox) {
			meshLODData_[handle].indirectCommand.textureData.environmentMapHandle = skybox->textureHandle;
			}, exclude<Disabled>());
		meshLODData_[handle].indirectCommand.textureData.enableMipMaps = plane->enableMipMaps;
		meshLODData_[handle].indirectCommand.vertexBufferView = meshManager_->GetVertexBufferView(plane->meshHandle);
		meshLODData_[handle].indirectCommand.indexBufferView = meshManager_->GetIndexBufferView(plane->meshHandle);
		meshLODData_[handle].indirectCommand.drawIndexedArguments.IndexCountPerInstance = meshManager_->GetIndexCount(plane->meshHandle);
		meshLODData_[handle].indirectCommand.drawIndexedArguments.InstanceCount = 1;
		meshLODData_[handle].indirectCommand.drawIndexedArguments.StartIndexLocation = 0;
		meshLODData_[handle].indirectCommand.drawIndexedArguments.BaseVertexLocation = 0;
		meshLODData_[handle].indirectCommand.drawIndexedArguments.StartInstanceLocation = 0;
		meshLODData_[handle].error = plane->error;
		indirectCommandHandle.handles.emplace_back(handle);
	} else if (box) {
		meshCounter_++;
		uint32_t handle = static_cast<uint32_t>(entities_.size());
		entities_.emplace_back(entity);
		meshLODData_[handle].indirectCommand.cbv[0] = world_->GetConstantBuffer(ConstantBufferType::kTransform)->GetGPUVirtualAddress(object->handle);
		meshLODData_[handle].indirectCommand.cbv[1] = world_->GetConstantBuffer(ConstantBufferType::kMaterial)->GetGPUVirtualAddress(object->handle);
		meshLODData_[handle].indirectCommand.textureData.textureHandle = box->textureHandle;
		registry_->ForEach<Skybox>([&](uint32_t skyboxEntity, Skybox *skybox) {
			meshLODData_[handle].indirectCommand.textureData.environmentMapHandle = skybox->textureHandle;
			}, exclude<Disabled>());
		meshLODData_[handle].indirectCommand.textureData.enableMipMaps = box->enableMipMaps;
		meshLODData_[handle].indirectCommand.vertexBufferView = meshManager_->GetVertexBufferView(box->meshHandle);
		meshLODData_[handle].indirectCommand.indexBufferView = meshManager_->GetIndexBufferView(box->meshHandle);
		meshLODData_[handle].indirectCommand.drawIndexedArguments.IndexCountPerInstance = meshManager_->GetIndexCount(box->meshHandle);
		meshLODData_[handle].indirectCommand.drawIndexedArguments.InstanceCount = 1;
		meshLODData_[handle].indirectCommand.drawIndexedArguments.StartIndexLocation = 0;
		meshLODData_[handle].indirectCommand.drawIndexedArguments.BaseVertexLocation = 0;
		meshLODData_[handle].indirectCommand.drawIndexedArguments.StartInstanceLocation = 0;
		meshLODData_[handle].error = box->error;
		indirectCommandHandle.handles.emplace_back(handle);
	} else if (ring) {
		meshCounter_++;
		uint32_t handle = static_cast<uint32_t>(entities_.size());
		entities_.emplace_back(entity);
		meshLODData_[handle].indirectCommand.cbv[0] = world_->GetConstantBuffer(ConstantBufferType::kTransform)->GetGPUVirtualAddress(object->handle);
		meshLODData_[handle].indirectCommand.cbv[1] = world_->GetConstantBuffer(ConstantBufferType::kMaterial)->GetGPUVirtualAddress(object->handle);
		meshLODData_[handle].indirectCommand.textureData.textureHandle = ring->textureHandle;
		registry_->ForEach<Skybox>([&](uint32_t skyboxEntity, Skybox *skybox) {
			meshLODData_[handle].indirectCommand.textureData.environmentMapHandle = skybox->textureHandle;
			}, exclude<Disabled>());
		meshLODData_[handle].indirectCommand.textureData.enableMipMaps = ring->enableMipMaps;
		meshLODData_[handle].indirectCommand.vertexBufferView = meshManager_->GetVertexBufferView(ring->meshHandle);
		meshLODData_[handle].indirectCommand.indexBufferView = meshManager_->GetIndexBufferView(ring->meshHandle);
		meshLODData_[handle].indirectCommand.drawIndexedArguments.IndexCountPerInstance = meshManager_->GetIndexCount(ring->meshHandle);
		meshLODData_[handle].indirectCommand.drawIndexedArguments.InstanceCount = 1;
		meshLODData_[handle].indirectCommand.drawIndexedArguments.StartIndexLocation = 0;
		meshLODData_[handle].indirectCommand.drawIndexedArguments.BaseVertexLocation = 0;
		meshLODData_[handle].indirectCommand.drawIndexedArguments.StartInstanceLocation = 0;
		meshLODData_[handle].error = ring->error;
		indirectCommandHandle.handles.emplace_back(handle);
	} else if (cylinder) {
		meshCounter_++;
		uint32_t handle = static_cast<uint32_t>(entities_.size());
		entities_.emplace_back(entity);
		meshLODData_[handle].indirectCommand.cbv[0] = world_->GetConstantBuffer(ConstantBufferType::kTransform)->GetGPUVirtualAddress(object->handle);
		meshLODData_[handle].indirectCommand.cbv[1] = world_->GetConstantBuffer(ConstantBufferType::kMaterial)->GetGPUVirtualAddress(object->handle);
		meshLODData_[handle].indirectCommand.textureData.textureHandle = cylinder->textureHandle;
		registry_->ForEach<Skybox>([&](uint32_t skyboxEntity, Skybox *skybox) {
			meshLODData_[handle].indirectCommand.textureData.environmentMapHandle = skybox->textureHandle;
			}, exclude<Disabled>());
		meshLODData_[handle].indirectCommand.textureData.enableMipMaps = cylinder->enableMipMaps;
		meshLODData_[handle].indirectCommand.vertexBufferView = meshManager_->GetVertexBufferView(cylinder->meshHandle);
		meshLODData_[handle].indirectCommand.indexBufferView = meshManager_->GetIndexBufferView(cylinder->meshHandle);
		meshLODData_[handle].indirectCommand.drawIndexedArguments.IndexCountPerInstance = meshManager_->GetIndexCount(cylinder->meshHandle);
		meshLODData_[handle].indirectCommand.drawIndexedArguments.InstanceCount = 1;
		meshLODData_[handle].indirectCommand.drawIndexedArguments.StartIndexLocation = 0;
		meshLODData_[handle].indirectCommand.drawIndexedArguments.BaseVertexLocation = 0;
		meshLODData_[handle].indirectCommand.drawIndexedArguments.StartInstanceLocation = 0;
		meshLODData_[handle].error = cylinder->error;
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

void IndirectCommandManager::UpdateCullingData() {
	uint32_t cullingMeshDataOffset = 0;
	uint32_t meshLODDataOffset = 0;
	registry_->ForEach<Model, BlendMode, Object>([&](uint32_t entity, Model *model, BlendMode *blendMode, Object *object) {
		EulerTransform *eulerTransform = registry_->GetComponent<EulerTransform>(entity);
		QuaternionTransform *quaternionTransform = registry_->GetComponent<QuaternionTransform>(entity);
		if (eulerTransform) {
			cullingObjectData_[object->handle].worldMatrix = eulerTransform->worldMatrix;
		} else if (quaternionTransform) {
			cullingObjectData_[object->handle].worldMatrix = quaternionTransform->worldMatrix;
		} else {
			cullingObjectData_[object->handle].worldMatrix = MakeIdentity4x4();
		}
		if (registry_->HasComponent<SkinMesh>(entity)) {
			cullingObjectData_[object->handle].meshType = MeshType::kSkinned;
		} else {
			cullingObjectData_[object->handle].meshType = MeshType::kModel;
		}
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

	registry_->ForEach<Plane, BlendMode, Object>([&](uint32_t entity, Plane *plane, BlendMode *blendMode, Object *object) {
		EulerTransform *eulerTransform = registry_->GetComponent<EulerTransform>(entity);
		QuaternionTransform *quaternionTransform = registry_->GetComponent<QuaternionTransform>(entity);
		if (eulerTransform) {
			cullingObjectData_[object->handle].worldMatrix = eulerTransform->worldMatrix;
		} else if (quaternionTransform) {
			cullingObjectData_[object->handle].worldMatrix = quaternionTransform->worldMatrix;
		} else {
			cullingObjectData_[object->handle].worldMatrix = MakeIdentity4x4();
		}
		cullingObjectData_[object->handle].meshType = MeshType::kPlane;
		cullingObjectData_[object->handle].blendMode = *blendMode;
		cullingMeshData_[cullingMeshDataOffset] = {
			.aabb = {
				.min = { plane->aabb.min.x, plane->aabb.min.y, plane->aabb.min.z, 1.0f },
				.max = { plane->aabb.max.x, plane->aabb.max.y, plane->aabb.max.z, 1.0f }
			},
			.objectHandle = object->handle,
			.lodOffset = meshLODDataOffset,
			.lodCount = 1,
			.useCulling = registry_->HasComponent<UseCulling>(entity) ? 1u : 0
		};
		meshLODDataOffset++;
		cullingMeshDataOffset++;
		});

	registry_->ForEach<Box, BlendMode, Object>([&](uint32_t entity, Box *box, BlendMode *blendMode, Object *object) {
		EulerTransform *eulerTransform = registry_->GetComponent<EulerTransform>(entity);
		QuaternionTransform *quaternionTransform = registry_->GetComponent<QuaternionTransform>(entity);
		if (eulerTransform) {
			cullingObjectData_[object->handle].worldMatrix = eulerTransform->worldMatrix;
		} else if (quaternionTransform) {
			cullingObjectData_[object->handle].worldMatrix = quaternionTransform->worldMatrix;
		} else {
			cullingObjectData_[object->handle].worldMatrix = MakeIdentity4x4();
		}
		cullingObjectData_[object->handle].meshType = MeshType::kBox;
		cullingObjectData_[object->handle].blendMode = *blendMode;
		cullingMeshData_[cullingMeshDataOffset] = {
			.aabb = {
				.min = { box->aabb.min.x, box->aabb.min.y, box->aabb.min.z, 1.0f },
				.max = { box->aabb.max.x, box->aabb.max.y, box->aabb.max.z, 1.0f }
			},
			.objectHandle = object->handle,
			.lodOffset = meshLODDataOffset,
			.lodCount = 1,
			.useCulling = registry_->HasComponent<UseCulling>(entity) ? 1u : 0
		};
		meshLODDataOffset++;
		cullingMeshDataOffset++;
		});

	registry_->ForEach<Ring, BlendMode, Object>([&](uint32_t entity, Ring *ring, BlendMode *blendMode, Object *object) {
		EulerTransform *eulerTransform = registry_->GetComponent<EulerTransform>(entity);
		QuaternionTransform *quaternionTransform = registry_->GetComponent<QuaternionTransform>(entity);
		if (eulerTransform) {
			cullingObjectData_[object->handle].worldMatrix = eulerTransform->worldMatrix;
		} else if (quaternionTransform) {
			cullingObjectData_[object->handle].worldMatrix = quaternionTransform->worldMatrix;
		} else {
			cullingObjectData_[object->handle].worldMatrix = MakeIdentity4x4();
		}
		cullingObjectData_[object->handle].meshType = MeshType::kRing;
		cullingObjectData_[object->handle].blendMode = *blendMode;
		cullingMeshData_[cullingMeshDataOffset] = {
			.aabb = {
				.min = { ring->aabb.min.x, ring->aabb.min.y, ring->aabb.min.z, 1.0f },
				.max = { ring->aabb.max.x, ring->aabb.max.y, ring->aabb.max.z, 1.0f }
			},
			.objectHandle = object->handle,
			.lodOffset = meshLODDataOffset,
			.lodCount = 1,
			.useCulling = registry_->HasComponent<UseCulling>(entity) ? 1u : 0
		};
		meshLODDataOffset++;
		cullingMeshDataOffset++;
		});

	registry_->ForEach<Cylinder, BlendMode, Object>([&](uint32_t entity, Cylinder *cylinder, BlendMode *blendMode, Object *object) {
		EulerTransform *eulerTransform = registry_->GetComponent<EulerTransform>(entity);
		QuaternionTransform *quaternionTransform = registry_->GetComponent<QuaternionTransform>(entity);
		if (eulerTransform) {
			cullingObjectData_[object->handle].worldMatrix = eulerTransform->worldMatrix;
		} else if (quaternionTransform) {
			cullingObjectData_[object->handle].worldMatrix = quaternionTransform->worldMatrix;
		} else {
			cullingObjectData_[object->handle].worldMatrix = MakeIdentity4x4();
		}
		cullingObjectData_[object->handle].meshType = MeshType::kCylinder;
		cullingObjectData_[object->handle].blendMode = *blendMode;
		cullingMeshData_[cullingMeshDataOffset] = {
			.aabb = {
				.min = { cylinder->aabb.min.x, cylinder->aabb.min.y, cylinder->aabb.min.z, 1.0f },
				.max = { cylinder->aabb.max.x, cylinder->aabb.max.y, cylinder->aabb.max.z, 1.0f }
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