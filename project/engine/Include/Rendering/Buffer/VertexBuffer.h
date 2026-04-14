#pragma once
#include "VertexData.h"
#include <d3d12.h>
#include <memory>

class Device;
class Resource;

/// @brief 頂点バッファ
class VertexBuffer final {
public:
	/// @brief 初期化
	/// @param device デバイス
	/// @param vertices 頂点数
	void Initialize(Device *device, size_t vertices);

	/// @brief VBVの設定
	void IASetVertexBuffers() const;

	/// @brief 描画
	/// @param instanceCount インスタンス数
	void DrawInstanced(UINT instanceCount) const;

	/// @brief 頂点バッファビューを取得する
	/// @return 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const { return vertexBufferView_; }

	/// @brief 頂点データを取得する
	/// @return 頂点データ
	VertexData *GetVertexData() const { return vertexData_; }

private:
	ID3D12GraphicsCommandList *commandList_ = nullptr;	// コマンドリスト
	std::unique_ptr<Resource> resource_ = nullptr;		// リソース
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};		// 頂点バッファビュー
	VertexData *vertexData_ = nullptr;					// 頂点データ
	size_t vertices_ = 0;								// 頂点数
};