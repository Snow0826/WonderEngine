#include "Resource.h"
#include "Device.h"
#include "StringConverter.h"

Resource::Resource(Device *device, D3D12_HEAP_TYPE heapType, const D3D12_RESOURCE_DESC &resourceDesc, D3D12_RESOURCE_STATES resourceState, D3D12_CLEAR_VALUE *clearValue)
	: commandList_(device->GetCommandList())
	, resourceState_(resourceState) {
	// ヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = heapType;

	// リソースの生成
	HRESULT hr = device->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		resourceState_,
		clearValue,
		IID_PPV_ARGS(&resource_)
	);
	assert(SUCCEEDED(hr));
}

std::unique_ptr<Resource> Resource::CreateBuffer(Device *device, D3D12_HEAP_TYPE heapType, size_t size, D3D12_RESOURCE_STATES resourceState, D3D12_RESOURCE_FLAGS resourceFlag) {
	// リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = size;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = resourceFlag;

	std::unique_ptr<Resource> resource = std::make_unique<Resource>(device, heapType, resourceDesc, resourceState);
	return std::move(resource);
}

std::unique_ptr<Resource> Resource::CreateTexture(Device *device, const DirectX::TexMetadata &metadata) {
	// metadataを基にリソースを設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = static_cast<UINT>(metadata.width);
	resourceDesc.Height = static_cast<UINT>(metadata.height);
	resourceDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
	resourceDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	if (IsUAVCompatible(device->GetDevice(), metadata.format)) {
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	std::unique_ptr<Resource> resource = std::make_unique<Resource>(device, D3D12_HEAP_TYPE_DEFAULT, resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST);
	return std::move(resource);
}

std::unique_ptr<Resource> Resource::CreateTexture2D(Device *device, size_t width, size_t height, uint16_t mipLevels, D3D12_RESOURCE_STATES resourceState, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS resourceFlag, D3D12_CLEAR_VALUE *clearValue) {
	// リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width = static_cast<UINT>(width);
	resourceDesc.Height = static_cast<UINT>(height);
	resourceDesc.MipLevels = static_cast<UINT16>(mipLevels);
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Flags = resourceFlag;

	std::unique_ptr<Resource> resource = std::make_unique<Resource>(device, D3D12_HEAP_TYPE_DEFAULT, resourceDesc, resourceState, clearValue);
	return std::move(resource);
}

std::unique_ptr<Resource> Resource::CreateUploadBuffer(Device *device, size_t size) {
	return CreateBuffer(device, D3D12_HEAP_TYPE_UPLOAD, size, D3D12_RESOURCE_STATE_GENERIC_READ);
}

std::unique_ptr<Resource> Resource::CreateReadbackBuffer(Device *device, size_t size) {
	return CreateBuffer(device, D3D12_HEAP_TYPE_READBACK, size, D3D12_RESOURCE_STATE_COPY_DEST);
}

std::unique_ptr<Resource> Resource::CreateRWBuffer(Device *device, size_t size) {
	return CreateBuffer(device, D3D12_HEAP_TYPE_DEFAULT, size, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
}

bool Resource::IsUAVCompatible(ID3D12Device *device, DXGI_FORMAT format) {
	D3D12_FEATURE_DATA_FORMAT_SUPPORT support = {};
	support.Format = format;
	HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support));
	assert(SUCCEEDED(hr));
	return (support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE) != 0;
}

void Resource::SetName(const std::string &name) {
	HRESULT hr = resource_->SetName(StringConverter::ConvertString(name).c_str());
	assert(SUCCEEDED(hr));
}

void Resource::Map(void **ppData) {
	HRESULT hr = resource_->Map(0, nullptr, ppData);
	assert(SUCCEEDED(hr));
}

void Resource::Unmap() {
	resource_->Unmap(0, nullptr);
}

void Resource::CopyFrom(const Microsoft::WRL::ComPtr<ID3D12Resource> &srcResource) {
	commandList_->CopyResource(resource_.Get(), srcResource.Get());
}

void Resource::CopyFrom(const Microsoft::WRL::ComPtr<ID3D12Resource> &srcResource, UINT64 dstOffset, UINT64 srcOffset, UINT64 numBytes) {
	commandList_->CopyBufferRegion(resource_.Get(), dstOffset, srcResource.Get(), srcOffset, numBytes);
}

void Resource::UAVBarrier() {
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.UAV.pResource = resource_.Get();
	commandList_->ResourceBarrier(1, &barrier);
}

void Resource::TransitionBarrier(D3D12_RESOURCE_STATES afterState, UINT subresource) {
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource_.Get();
	barrier.Transition.Subresource = subresource;
	barrier.Transition.StateBefore = resourceState_;
	barrier.Transition.StateAfter = afterState;
	commandList_->ResourceBarrier(1, &barrier);
	resourceState_ = afterState;
}

void Resource::TransitionBarrier(D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState, UINT subresource) {
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource_.Get();
	barrier.Transition.Subresource = subresource;
	barrier.Transition.StateBefore = beforeState;
	barrier.Transition.StateAfter = afterState;
	commandList_->ResourceBarrier(1, &barrier);
	resourceState_ = afterState;
}

void Resource::TransitionBarrier(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> &commandList, const Microsoft::WRL::ComPtr<ID3D12Resource> &resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState, UINT subresource) {
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource.Get();
	barrier.Transition.Subresource = subresource;
	barrier.Transition.StateBefore = beforeState;
	barrier.Transition.StateAfter = afterState;
	commandList->ResourceBarrier(1, &barrier);
}