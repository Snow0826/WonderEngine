#include "Texture.h"
#include "Resource.h"
#include "StringConverter.h"
#include "Logger.h"
#include "Device.h"
#include "d3dx12.h"
#include "ImGuiManager.h"

TextureManager::TextureManager(Device *device, std::ofstream *logStream) : device_(device), logStream_(logStream) {}
TextureManager::~TextureManager() = default;

uint32_t TextureManager::LoadTexture(const std::string &fileName) {
	// すでに読み込まれている場合は何もしない
	if (textures_.contains(fileName)) {
		Logger::Log(*logStream_, "Texture already loaded: " + fileName + "\n");
		return textures_.at(fileName)->readHandle;
	}

	// ファイル名が空の場合は白テクスチャを返す
	if (fileName.empty()) {
		Logger::Log(*logStream_, "Error: Texture file name is empty.\n");
		return textures_.at("white8x8.png")->readHandle;
	}

	// Textureを読み込む
	std::unique_ptr<Texture> texture = std::make_unique<Texture>();
	mipImages_ = LoadTexture("resources/texture", fileName);
	const DirectX::TexMetadata &metadata = mipImages_.GetMetadata();
	texture->resource = Resource::CreateTexture(device_, metadata);
	std::vector<D3D12_SUBRESOURCE_DATA> subResources;
	DirectX::PrepareUpload(device_->GetDevice(), mipImages_.GetImages(), mipImages_.GetImageCount(), metadata, subResources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture->resource->GetResource(), 0, static_cast<UINT>(subResources.size()));
	texture->intermediateResource = Resource::CreateUploadBuffer(device_, intermediateSize);
	UpdateSubresources(device_->GetCommandList(), texture->resource->GetResource(), texture->intermediateResource->GetResource(), 0, 0, static_cast<UINT>(subResources.size()), subResources.data());

	// TransitionBarrierの設定(CopyDest->GenericRead)
	texture->resource->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);

	// SRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;											// フォーマット
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;						// テクスチャ2D
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;	// シェーダーのコンポーネントマッピング
	srvDesc.Texture2D.MostDetailedMip = 0;										// 最も詳細なミップレベル
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);		// ミップレベル数
	srvDesc.Texture2D.PlaneSlice = 0;											// プレーンスライス

	// SRVの作成
	texture->readHandle = device_->GetGpuCbvSrvUavDescriptorHeap_()->AllocateDescriptor();
	device_->GetGpuCbvSrvUavDescriptorHeap_()->CreateShaderResourceView(texture->resource->GetResource(), srvDesc, texture->readHandle);
	Logger::Log(*logStream_, "Created SRV for texture: " + std::to_string(texture->readHandle) + "\n");

	if (metadata.format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ||
		metadata.format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ||
		metadata.format == DXGI_FORMAT_BC1_TYPELESS ||
		metadata.format == DXGI_FORMAT_BC2_TYPELESS ||
		metadata.format == DXGI_FORMAT_BC3_TYPELESS ||
		metadata.format == DXGI_FORMAT_BC7_TYPELESS) {
		Logger::Log(*logStream_, "Warning: Texture format " + std::to_string(metadata.format) + " may not support UAV.\n");
	} else {
		// UAVの設定
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.Format = metadata.format;						// フォーマット
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;	// テクスチャ2D

		// UAVの作成
		texture->writeHandle = device_->GetGpuCbvSrvUavDescriptorHeap_()->AllocateDescriptor();
		device_->GetGpuCbvSrvUavDescriptorHeap_()->CreateUnorderedAccessView(texture->resource->GetResource(), uavDesc, texture->writeHandle);
		device_->GetCpuCbvSrvUavDescriptorHeap_()->CreateUnorderedAccessView(texture->resource->GetResource(), uavDesc, texture->writeHandle);
		Logger::Log(*logStream_, "Created UAV for texture: " + std::to_string(texture->writeHandle) + "\n");
	}

	// テクスチャリストに追加
	textures_.insert(std::make_pair(fileName, std::move(texture)));
	Logger::Log(*logStream_, "Loaded texture: " + fileName + "\n");
	return textures_.at(fileName)->readHandle;
}

uint32_t TextureManager::GetTextureReadHandle(const std::string &fileName) const {
	if (textures_.contains(fileName)) {
		return textures_.at(fileName)->readHandle;
	}
	return textures_.at("white8x8.png")->readHandle;
}

uint32_t TextureManager::GetTextureWriteHandle(const std::string &fileName) const {
	if (textures_.contains(fileName)) {
		return textures_.at(fileName)->writeHandle;
	}
	return textures_.at("white8x8.png")->writeHandle;
}

uint32_t TextureManager::EditTexture([[maybe_unused]] const std::string &label, [[maybe_unused]] std::string &fileName) {
#ifdef USE_IMGUI
	if (ImGui::BeginCombo(label.c_str(), fileName.c_str())) {
		for (auto &texture : textures_) {
			bool selected = (texture.first == fileName);
			if (ImGui::Selectable(texture.first.c_str(), selected)) {
				fileName = texture.first;
			}
			if (selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
#endif // USE_IMGUI
	return GetTextureReadHandle(fileName);
}

D3D12_RESOURCE_DESC TextureManager::GetResourceDesc(const std::string &fileName) const {
	assert(textures_.contains(fileName));
	return textures_.at(fileName)->resource->GetResource()->GetDesc();
}

Resource *TextureManager::GetTextureResource(const std::string &fileName) const {
	assert(textures_.contains(fileName));
	return textures_.at(fileName)->resource.get();
}

DirectX::ScratchImage TextureManager::LoadTexture(const std::string &directoryPath, const std::string &fileName) {
	// テクスチャの読み込み
	DirectX::ScratchImage image;
	std::wstring filePathW = StringConverter::ConvertString(directoryPath + "/" + fileName);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	DirectX::ScratchImage mipImages;
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// ミップマップ付きのデータを返す
	return mipImages;
}