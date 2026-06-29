#include "SkinCluster.h"
#include "Device.h"
#include "EntityComponentSystem.h"
#include "Model.h"
#include "Resource.h"

uint32_t SkinClusterManager::CreateSkinCluster(const ModelData &modelData) {
	if (modelData.skeleton.joints.size() < 2) {
		return 0;
	}

	uint32_t skinClusterIndex = static_cast<uint32_t>(skinClusters_.size());
	DescriptorHeap *gpuCbvSrvUavDescriptorHeap = device_->GetGpuCbvSrvUavDescriptorHeap();
	std::unique_ptr<SkinCluster> skinCluster = std::make_unique<SkinCluster>();

	// Palette用のResourceを確保
	WellForGPU *mappedPalette = nullptr;
	skinCluster->paletteResource = Resource::CreateUploadBuffer(device_, sizeof(WellForGPU) * modelData.skeleton.joints.size());
	skinCluster->paletteResource->Map(reinterpret_cast<void **>(&mappedPalette));
	skinCluster->mappedPalette = { mappedPalette, modelData.skeleton.joints.size() };
	skinCluster->paletteSRVHandle = gpuCbvSrvUavDescriptorHeap->AllocateDescriptor();

	// Palette用のSRVを作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = static_cast<UINT>(modelData.skeleton.joints.size());
	srvDesc.Buffer.StructureByteStride = sizeof(WellForGPU);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	gpuCbvSrvUavDescriptorHeap->CreateShaderResourceView(skinCluster->paletteResource->GetResource(), srvDesc, skinCluster->paletteSRVHandle);

	// Influence用のResourceを確保
	VertexInfluence *mappedInfluence = nullptr;
	skinCluster->influenceResource = Resource::CreateUploadBuffer(device_, sizeof(VertexInfluence) * modelData.meshes.back().lods.back().vertices.size());
	skinCluster->influenceResource->Map(reinterpret_cast<void **>(&mappedInfluence));
	std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * modelData.meshes.back().lods.back().vertices.size());
	skinCluster->mappedInfluence = { mappedInfluence, modelData.meshes.back().lods.back().vertices.size() };

	// Influence用のVertexBufferViewを作成
	skinCluster->influenceBufferView.BufferLocation = skinCluster->influenceResource->GetResource()->GetGPUVirtualAddress();
	skinCluster->influenceBufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexInfluence) * modelData.meshes.back().lods.back().vertices.size());
	skinCluster->influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

	// InverseBindPoseMatrixを格納する場所を作成して、単位行列で埋める
	skinCluster->inverseBindPoseMatrices.resize(modelData.skeleton.joints.size());
	std::generate(skinCluster->inverseBindPoseMatrices.begin(), skinCluster->inverseBindPoseMatrices.end(), MakeIdentity4x4);

	for (const auto &jointWeight : modelData.skinClusterData) {
		auto it = modelData.skeleton.jointIndexByName.find(jointWeight.first);
		if (it == modelData.skeleton.jointIndexByName.end()) {
			continue;
		}
		skinCluster->inverseBindPoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
		for (const VertexWeightData &vertexWeight : jointWeight.second.vertexWeights) {
			VertexInfluence &currentInfluence = skinCluster->mappedInfluence[vertexWeight.vertexIndex];
			for (uint32_t index = 0; index < kNumMaxInfluence; ++index) {
				if (currentInfluence.weights[index] == 0.0f) {
					currentInfluence.weights[index] = vertexWeight.weight;
					currentInfluence.jointIndices[index] = static_cast<uint32_t>((*it).second);
					break;
				}
			}
		}
	}
	skinClusters_.emplace_back(std::move(skinCluster));
	return skinClusterIndex;
}

void SkinClusterManager::Update() {
	registry_->ForEach<Model>([this](uint32_t entity, Model *model) {
		SkinCluster *skinCluster = skinClusters_[model->skinClusterHandle].get();
		for (size_t jointIndex = 0; jointIndex < model->modelData.skeleton.joints.size(); ++jointIndex) {
			assert(jointIndex < skinCluster->inverseBindPoseMatrices.size());
			skinCluster->mappedPalette[jointIndex].skeletonSpaceMatrix = skinCluster->inverseBindPoseMatrices[jointIndex] * model->modelData.skeleton.joints[jointIndex].skeletonSpaceMatrix;
			skinCluster->mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix = skinCluster->mappedPalette[jointIndex].skeletonSpaceMatrix.inverse().transpose();
		}
		}, exclude<Disabled>());
}

D3D12_VERTEX_BUFFER_VIEW SkinClusterManager::GetInfluenceBufferView(uint32_t skinClusterIndex) const {
	assert(skinClusterIndex < skinClusters_.size());
	return skinClusters_[skinClusterIndex]->influenceBufferView;
}

uint32_t SkinClusterManager::GetPaletteSRVHandle(uint32_t skinClusterIndex) const {
	assert(skinClusterIndex < skinClusters_.size());
	return skinClusters_[skinClusterIndex]->paletteSRVHandle;
}