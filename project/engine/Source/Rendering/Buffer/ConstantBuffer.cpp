#include "ConstantBuffer.h"
#include "Resource.h"
#include "Device.h"

ConstantBuffer::ConstantBuffer() = default;
ConstantBuffer::~ConstantBuffer() = default;

void ConstantBuffer::Initialize(Device *device, size_t size, uint32_t count) {
	// デバイスのnullチェック
	assert(device);

	// 256バイトアライメントにサイズを調整
	alignedSize_ = (size + 255) & ~255;
	bufferSize_ = alignedSize_ * count;

	// コマンドリストを取得
	commandList_ = device->GetCommandList();

	// リソースを作成して、マッピング
	resource_ = Resource::CreateUploadBuffer(device, bufferSize_);
	resource_->Map(reinterpret_cast<void **>(&bufferGPUAddress_));
}

void ConstantBuffer::CopyData(const void *data, size_t size, uint32_t index) {
	assert(alignedSize_ * index + size <= bufferSize_);
	memcpy(bufferGPUAddress_ + (alignedSize_ * index), data, size);
}

void ConstantBuffer::BindToGraphics(UINT rootParameterIndex, uint32_t bufferLocationIndex) const {
	commandList_->SetGraphicsRootConstantBufferView(rootParameterIndex, resource_->GetGPUVirtualAddress() + (alignedSize_ * bufferLocationIndex));
}

void ConstantBuffer::BindToCompute(UINT rootParameterIndex, uint32_t bufferLocationIndex) const {
	commandList_->SetComputeRootConstantBufferView(rootParameterIndex, resource_->GetGPUVirtualAddress() + (alignedSize_ * bufferLocationIndex));
}

void ConstantBuffer::SetName(const std::string &name) {
	resource_->SetName(name);
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGPUVirtualAddress(uint32_t index) const {
	return resource_->GetGPUVirtualAddress() + (alignedSize_ * index);
}