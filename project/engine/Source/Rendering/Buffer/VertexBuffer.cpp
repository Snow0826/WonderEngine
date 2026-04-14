#include "VertexBuffer.h"
#include "Resource.h"
#include "Device.h"

void VertexBuffer::Initialize(Device *device, size_t vertices) {
	// デバイスのnullチェック
	assert(device);

	// コマンドリストの取得
	commandList_ = device->GetCommandList();

	// 頂点数の保存
	vertices_ = vertices;

	// リソースを作成して、マッピング
	resource_ = Resource::CreateUploadBuffer(device, sizeof(VertexData) * vertices_);
	resource_->Map(reinterpret_cast<void **>(&vertexData_));

	// VBVの設定
	vertexBufferView_.BufferLocation = resource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_);
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void VertexBuffer::IASetVertexBuffers() const {
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
}

void VertexBuffer::DrawInstanced(UINT instanceCount) const {
	if (instanceCount) {
		commandList_->DrawInstanced(static_cast<UINT>(vertices_), instanceCount, 0, 0);
	}
}