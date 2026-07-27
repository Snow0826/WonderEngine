#include "VertexBuffer.h"
#include "Resource.h"
#include "Device.h"

void VertexBuffer::Initialize(Device *device, size_t vertices, VertexBufferType type) {
	// デバイスのnullチェック
	assert(device);

	// 頂点数の保存
	vertices_ = vertices;

	// バッファタイプの保存
	type_ = type;

	// リソースを作成して、マッピング
	resource_ = Resource::CreateUploadBuffer(device, sizeof(VertexData) * vertices_);
	resource_->Map(reinterpret_cast<void **>(&vertexData_));

	// Viewの設定
	switch (type) {
		case VertexBufferType::kVBV:
			vertexBufferView_.BufferLocation = resource_->GetGPUVirtualAddress();
			vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_);
			vertexBufferView_.StrideInBytes = sizeof(VertexData);
			break;
		case VertexBufferType::kSRV:
		{
			DescriptorHeap *descriptorHeap = device->GetGpuCbvSrvUavDescriptorHeap();
			srvHandle_ = descriptorHeap->AllocateDescriptor();
			D3D12_SHADER_RESOURCE_VIEW_DESC srvBufferDesc{};
			srvBufferDesc.Format = DXGI_FORMAT_UNKNOWN;											// バッファなのでフォーマットなし
			srvBufferDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;							// バッファビュー
			srvBufferDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;	// 標準設定
			srvBufferDesc.Buffer.FirstElement = 0;												// 先頭の要素
			srvBufferDesc.Buffer.NumElements = static_cast<UINT>(vertices_);					// 要素数
			srvBufferDesc.Buffer.StructureByteStride = sizeof(VertexData);						// 構造体のサイズ
			srvBufferDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;							// 特になし
			descriptorHeap->CreateShaderResourceView(resource_->GetResource(), srvBufferDesc, srvHandle_);
		}
		break;
		default:
			break;
	}
}

D3D12_VERTEX_BUFFER_VIEW VertexBuffer::GetVertexBufferView() const {
	assert(type_ == VertexBufferType::kVBV);
	return vertexBufferView_;
}

uint32_t VertexBuffer::GetSRVHandle() const {
	assert(type_ == VertexBufferType::kSRV);
	return srvHandle_;
}