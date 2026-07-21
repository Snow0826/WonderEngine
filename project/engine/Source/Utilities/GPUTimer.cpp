#include "GPUTimer.h"
#include "Device.h"
#include "Resource.h"
#include <cassert>

GPUTimer::GPUTimer(Device *device) : device_(device) {
	// クエリヒープの作成
	D3D12_QUERY_HEAP_DESC queryHeapDesc{ .Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP, .Count = 2 };
	HRESULT hr = device_->GetDevice()->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&queryHeap_));
	assert(SUCCEEDED(hr));

	// 読み取りバッファの作成
	readbackBuffer_ = Resource::CreateReadbackBuffer(device_, sizeof(uint64_t) * 2);
	readbackBuffer_->SetName("GPUTimerReadbackBuffer");
	readbackBuffer_->Map(reinterpret_cast<void **>(&timestamps));
}

GPUTimer::~GPUTimer() = default;

void GPUTimer::Begin() {
	device_->GetCommandList()->EndQuery(queryHeap_.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0);
}

void GPUTimer::End() {
	device_->GetCommandList()->EndQuery(queryHeap_.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 1);
}

void GPUTimer::Resolve() {
	device_->GetCommandList()->ResolveQueryData(queryHeap_.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, 2, readbackBuffer_->GetResource(), 0);
}

double GPUTimer::GetMs() const {
	uint64_t frequency = 0;
	HRESULT hr = device_->GetCommandQueue()->GetTimestampFrequency(&frequency);
	assert(SUCCEEDED(hr));
	return static_cast<double>(timestamps[1] - timestamps[0]) * 1000.0 / static_cast<double>(frequency);
}