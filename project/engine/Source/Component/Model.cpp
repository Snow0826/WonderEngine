#define NOMINMAX
#include "Model.h"
#include "EntityComponentSystem.h"
#include "VertexBuffer.h"
#include "Texture.h"
#include "SkinCluster.h"
#include "Matrix3x3.h"
#include "Logger.h"
#include "StringConverter.h"
#include "QuadricErrorMetrics.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <unordered_set>
#include <queue>

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void ModelManager::LoadModel(const std::string &fileName) {
	// すでに読み込まれている場合は何もしない
	if (models_.contains(fileName)) {
		Logger::Log(*logStream_, "Model already loaded: " + fileName + "\n");
		return;
	}

	std::unique_ptr<Model> model = std::make_unique<Model>();

	// モデルデータの読み込み
	model->modelData = LoadModelData(fileName);

	// テクスチャの読み込み
	for (const MaterialData &materialData : model->modelData.materials) {
		model->textureHandle.emplace_back(textureManager_->LoadTexture(materialData.textureFilePath));
		model->enableMipMaps.emplace_back(true);
	}
	
	// スキンクラスターの作成
	model->skinClusterHandle = skinClusterManager_->CreateSkinCluster(model->modelData);

	// モデル名の設定
	model->name = fileName;
	Logger::Log(*logStream_, "Loaded model: " + fileName + "\n");
	models_.insert(std::make_pair(fileName, std::move(model)));
}

Model ModelManager::FindModel(const std::string &fileName) {
	Model model;
	if (models_.contains(fileName)) {
		model = *models_.at(fileName);
		for (MeshData &mesh : model.modelData.meshes) {
			for (const MeshLODData &lod : mesh.lods) {
				meshManager_->CreateMesh(lod.meshName, lod);
				mesh.aabb = meshManager_->CreateLocalAABB(lod.meshName);
				mesh.obb = meshManager_->CreateLocalOBB(lod.meshName);
				mesh.sphere = meshManager_->CreateLocalSphere(lod.meshName);
			}
		}
	}
	return model;
}

bool ModelManager::Combo(const std::string &label, Model *model) {
	bool changed = false;
#ifdef USE_IMGUI
	if (ImGui::BeginCombo(label.c_str(), model ? model->name.c_str() : "Select Model")) {
		for (auto &[name, storedModel] : models_) {
			bool isSelected = (model && storedModel->name == model->name);
			if (ImGui::Selectable(storedModel->name.c_str(), isSelected)) {
				*model = *storedModel;
				changed = true;
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
#endif // USE_IMGUI
	return changed;
}

void ModelManager::UpdateSkeleton(Skeleton &skeleton) {
	for (Joint &joint : skeleton.joints) {
		Matrix4x4 localMatrix = MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
		if (joint.parent) {
			joint.skeletonSpaceMatrix = localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix;
		} else {
			joint.skeletonSpaceMatrix = localMatrix;
		}
	}
}

Matrix4x4 ModelManager::MakeLocalMatrix(const Node &node) {
	Matrix4x4 result = MakeAffineMatrix(node.transform.scale, node.transform.rotate, node.transform.translate);
	for (const Node &child : node.children) {
		result *= MakeLocalMatrix(child);
	}
	return result;
}

ModelData ModelManager::LoadModelData(const std::string &fileName) {
	ModelData modelData;

	// シーンの読み込み
	Assimp::Importer importer;
	auto [fileNameWithoutExtension, extension] = StringConverter::SeparateFileNameAndExtension(fileName);
	std::string filePath = "resources/" + extension + "/" + fileNameWithoutExtension + "/" + fileName;
	const aiScene *scene = importer.ReadFile(filePath.c_str(), aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_FlipUVs);

	// メタデータの読み込み
	aiMetadata *metadata = scene->mMetaData;
	if (metadata) {
		aiString format;
		if (metadata->Get(metadata->mKeys->C_Str(), format)) {
			modelData.format = format.C_Str();
		}
	}

	// メッシュの読み込み
	assert(scene->HasMeshes());	// メッシュがないのは対応しない
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh *mesh = scene->mMeshes[meshIndex];
		MeshLODData meshLODData;

		// 頂点データの読み込み
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D position = mesh->mVertices[vertexIndex];
			aiVector3D texcoord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][vertexIndex] : aiVector3D(0, 0, 0);
			aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[vertexIndex] : aiVector3D(0, 0, 0);
			VertexData vertex = {
				.position = { -position.x, position.y, position.z, 1.0f },
				.texcoord = { texcoord.x, texcoord.y },
				.normal = { -normal.x, normal.y, normal.z }
			};
			meshLODData.vertices.emplace_back(vertex);
		}

		// インデックスデータの読み込み
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace &face = mesh->mFaces[faceIndex];
			for (uint32_t index = 0; index < face.mNumIndices; ++index) {
				meshLODData.indices.emplace_back(face.mIndices[index]);
			}
		}

		// スキンクラスターの読み込み
		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			aiBone *bone = mesh->mBones[boneIndex];
			aiVector3D scale, translate;
			aiQuaternion rotate;
			bone->mOffsetMatrix.Inverse().Decompose(scale, rotate, translate);
			Matrix4x4 bindPoseMatrix = MakeAffineMatrix(Vector3{ scale.x, scale.y, scale.z }, Quaternion{ rotate.x, -rotate.y, -rotate.z, rotate.w }, Vector3{ -translate.x, translate.y, translate.z });
			JointWeightData jointWeightData;
			jointWeightData.inverseBindPoseMatrix = bindPoseMatrix.inverse();
			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				aiVertexWeight weight = bone->mWeights[weightIndex];
				VertexWeightData vertexWeightData = {
					.weight = weight.mWeight,
					.vertexIndex = weight.mVertexId
				};
				jointWeightData.vertexWeights.emplace_back(vertexWeightData);
			}
			modelData.skinClusterData.emplace(bone->mName.C_Str(), jointWeightData);
		}

		meshLODData = meshManager_->ReIndexMeshLODData(meshLODData);
		meshLODData.error = 0.0f;
		meshLODData.meshName = fileName + mesh->mName.C_Str();
		MeshData meshData;
		meshData.lods.emplace_back(meshLODData);
		meshData.materialIndex = mesh->mMaterialIndex;
		modelData.meshes.emplace_back(meshData);
	}

	// マテリアルの読み込み
	assert(scene->HasMaterials()); // マテリアルがないのは対応しない
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial *material = scene->mMaterials[materialIndex];
		MaterialData materialData;
		if (std::string(material->GetName().C_Str()) == "DefaultMaterial") {
			materialData.textureFilePath = "white8x8.png";
		}

		if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString textureFilePath;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath) == AI_SUCCESS) {
				materialData.textureFilePath = textureFilePath.C_Str();
			}
		}
		modelData.materials.emplace_back(materialData);
	}

	// アニメーションの読み込み
	if (scene->HasAnimations()) {
		for (uint32_t animationIndex = 0; animationIndex < scene->mNumAnimations; ++animationIndex) {
			aiAnimation *animation = scene->mAnimations[animationIndex];
			AnimationClip animationClip;
			animationClip.name = animation->mName.C_Str();
			animationClip.duration = static_cast<float>(animation->mDuration / animation->mTicksPerSecond);

			// アニメーションノードの読み込み
			for (uint32_t channelIndex = 0; channelIndex < animation->mNumChannels; ++channelIndex) {
				aiNodeAnim *channel = animation->mChannels[channelIndex];
				NodeAnimation &nodeAnimation = animationClip.nodeAnimations[channel->mNodeName.C_Str()];

				// キーフレームの読み込み
				for (uint32_t keyIndex = 0; keyIndex < channel->mNumPositionKeys; ++keyIndex) {
					aiVectorKey key = channel->mPositionKeys[keyIndex];
					KeyFrameVector3 keyframe = {
						.time = static_cast<float>(key.mTime / animation->mTicksPerSecond),
						.value = { -key.mValue.x, key.mValue.y, key.mValue.z }
					};
					nodeAnimation.translate.keyframes.emplace_back(keyframe);
				}

				for (uint32_t keyIndex = 0; keyIndex < channel->mNumRotationKeys; ++keyIndex) {
					aiQuatKey key = channel->mRotationKeys[keyIndex];
					float time = static_cast<float>(key.mTime / animation->mTicksPerSecond);
					aiQuaternion rotate = key.mValue;
					KeyFrameQuaternion keyframe = {
						.time = time,
						.value = { rotate.x, -rotate.y, -rotate.z, rotate.w }
					};
					nodeAnimation.rotate.keyframes.emplace_back(keyframe);
				}

				for (uint32_t keyIndex = 0; keyIndex < channel->mNumScalingKeys; ++keyIndex) {
					aiVectorKey key = channel->mScalingKeys[keyIndex];
					float time = static_cast<float>(key.mTime / animation->mTicksPerSecond);
					aiVector3D scale = key.mValue;
					KeyFrameVector3 keyframe = {
						.time = time,
						.value = { scale.x, scale.y, scale.z }
					};
					nodeAnimation.scale.keyframes.emplace_back(keyframe);
				}
			}
			modelData.animations.emplace_back(animationClip);
		}
	}

	// ノードの読み込み
	modelData.rootNode = ReadNode(scene->mRootNode);

	// スケルトンの作成
	modelData.skeleton = CreateSkeleton(modelData.rootNode);

	return modelData;
}

Node ModelManager::ReadNode(const aiNode *node) {
	Node resultNode;
	aiVector3D scale, translate;
	aiQuaternion rotate;

	// ノード名の設定
	resultNode.name = node->mName.C_Str();

	// 変換行列の分解
	node->mTransformation.Decompose(scale, rotate, translate);

	// 平行移動の設定
	resultNode.transform.translate = { -translate.x, translate.y, translate.z };

	// 回転の設定
	resultNode.transform.rotate = { rotate.x, -rotate.y, -rotate.z, rotate.w };

	// スケーリングの設定
	resultNode.transform.scale = { scale.x, scale.y, scale.z };

	// 子ノードの読み込み
	resultNode.children.resize(node->mNumChildren);
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		resultNode.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}
	return resultNode;
}

Skeleton ModelManager::CreateSkeleton(const Node &rootNode) {
	Skeleton skeleton;
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);
	for (const Joint &joint : skeleton.joints) {
		skeleton.jointIndexByName.emplace(joint.name, joint.index);
	}
	UpdateSkeleton(skeleton);
	return skeleton;
}

int32_t ModelManager::CreateJoint(const Node &node, const std::optional<int32_t> &parent, std::vector<Joint> &joints) {
	Joint joint;
	joint.name = node.name;
	joint.skeletonSpaceMatrix = MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = static_cast<int32_t>(joints.size());
	joint.parent = parent;
	joints.emplace_back(joint);
	for (const Node &child : node.children) {
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joints[joint.index].children.emplace_back(childIndex);
	}
	return joint.index;
}

void ModelInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Model")) {
		Model *model = registry_->GetComponent<Model>(entity);
		if (model) {
			if (modelManager_->Combo(model->modelData.format, model)) {
				TransformSystem transformSystem{ registry_ };
				transformSystem.MarkDirty(entity);
			}

			for (size_t i = 0; i < model->modelData.meshes.size(); i++) {
				if (ImGui::TreeNode(("Mesh" + std::to_string(i)).c_str())) {
					for (size_t j = 0; j < model->modelData.meshes[i].lods.size(); j++) {
						if (ImGui::TreeNode(("LOD" + std::to_string(j)).c_str())) {
							ImGui::Text("vertices: %zu", model->modelData.meshes[i].lods[j].vertices.size());
							ImGui::Text("indices: %zu", model->modelData.meshes[i].lods[j].indices.size());
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
			}
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}