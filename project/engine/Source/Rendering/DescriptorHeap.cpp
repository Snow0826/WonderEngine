#include "DescriptorHeap.h"
#include "Device.h"
#include <cassert>

DescriptorHeap::DescriptorHeap(Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible)
	: device_(device->GetDevice())
	, commandList_(device->GetCommandList()) {
	descriptorSize_ = device_->GetDescriptorHandleIncrementSize(type);	// デスクリプタヒープの大きさを取得

	// ヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = type;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	// ヒープの生成
	HRESULT hr = device_->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap_));
	assert(SUCCEEDED(hr));
}

uint32_t DescriptorHeap::AllocateDescriptor() {
	assert(descriptorIndex_ < descriptorHeap_->GetDesc().NumDescriptors && "Exceeded the number of descriptors in the heap.");
	return descriptorIndex_++;
}

void DescriptorHeap::BindToGraphics(UINT rootParameterIndex, uint32_t descriptorIndex) const {
	commandList_->SetGraphicsRootDescriptorTable(rootParameterIndex, GetGPUDescriptorHandle(descriptorIndex));
}

void DescriptorHeap::BindToCompute(UINT rootParameterIndex, uint32_t descriptorIndex) const {
	commandList_->SetComputeRootDescriptorTable(rootParameterIndex, GetGPUDescriptorHandle(descriptorIndex));
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUDescriptorHandle(uint32_t index) const {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize_ * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUDescriptorHandle(uint32_t index) const {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize_ * index);
	return handleGPU;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::CreateRenderTargetView(const Microsoft::WRL::ComPtr<ID3D12Resource> &resource, D3D12_RENDER_TARGET_VIEW_DESC rtvDesc, uint32_t index) const {
	D3D12_CPU_DESCRIPTOR_HANDLE handle = GetCPUDescriptorHandle(index);
	device_->CreateRenderTargetView(resource.Get(), &rtvDesc, handle);
	return handle;
}

void DescriptorHeap::CreateShaderResourceView(const Microsoft::WRL::ComPtr<ID3D12Resource> &resource, D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc, uint32_t index) const {
	D3D12_CPU_DESCRIPTOR_HANDLE handle = GetCPUDescriptorHandle(index);
	device_->CreateShaderResourceView(resource.Get(), &srvDesc, handle);
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::CreateDepthStencilView(const Microsoft::WRL::ComPtr<ID3D12Resource> &resource, D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc, uint32_t index) const {
	D3D12_CPU_DESCRIPTOR_HANDLE handle = GetCPUDescriptorHandle(index);
	device_->CreateDepthStencilView(resource.Get(), &dsvDesc, handle);
	return handle;
}

void DescriptorHeap::CreateUnorderedAccessView(const Microsoft::WRL::ComPtr<ID3D12Resource> &resource, D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc, uint32_t index) const {
	D3D12_CPU_DESCRIPTOR_HANDLE handle = GetCPUDescriptorHandle(index);
	device_->CreateUnorderedAccessView(resource.Get(), nullptr, &uavDesc, handle);
}

void DescriptorHeap::CreateUnorderedAccessView(const Microsoft::WRL::ComPtr<ID3D12Resource> &resource, const Microsoft::WRL::ComPtr<ID3D12Resource> &counterResource, D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc, uint32_t index) const {
	D3D12_CPU_DESCRIPTOR_HANDLE handle = GetCPUDescriptorHandle(index);
	device_->CreateUnorderedAccessView(resource.Get(), counterResource.Get(), &uavDesc, handle);
}