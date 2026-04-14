#include "IndexBuffer.h"
#include "Resource.h"
#include "Device.h"

void IndexBuffer::Initialize(Device *device, size_t indices) {
	// デバイスのnullチェック
	assert(device);

	// コマンドリストの取得
	commandList_ = device->GetCommandList();

	// インデックス数の保存
	indices_ = indices;

	// リソースを作成して、マッピング
	resource_ = Resource::CreateUploadBuffer(device, sizeof(uint32_t) * indices_);
	resource_->Map(reinterpret_cast<void **>(&indexData_));

	// IBVの設定
	indexBufferView_.BufferLocation = resource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices_);
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}

void IndexBuffer::IASetIndexBuffer() const {
	commandList_->IASetIndexBuffer(&indexBufferView_);
}

void IndexBuffer::DrawIndexedInstanced(UINT instanceCount) const {
	if (instanceCount) {
		commandList_->DrawIndexedInstanced(static_cast<UINT>(indices_), instanceCount, 0, 0, 0);
	}
}