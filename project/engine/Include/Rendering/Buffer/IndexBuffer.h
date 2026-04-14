#pragma once
#include <d3d12.h>
#include <memory>

class Device;
class Resource;

/// @brief インデックスバッファ
class IndexBuffer final {
public:
	/// @brief 初期化
	/// @param device デバイス
	/// @param indices インデックス数
	void Initialize(Device *device, size_t indices);

	/// @brief IBVの設定
	void IASetIndexBuffer() const;

	/// @brief 描画
	/// @param instanceCount インスタンス数
	void DrawIndexedInstanced(UINT instanceCount) const;

	/// @brief インデックスバッファビューを取得する
	/// @return インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const { return indexBufferView_; }

	/// @brief インデックスデータを取得する
	/// @return インデックスデータ
	uint32_t *GetIndexData() const { return indexData_; }

	/// @brief インデックス数を取得する
	/// @return インデックス数
	UINT GetIndices() const { return static_cast<UINT>(indices_); }

private:
	ID3D12GraphicsCommandList *commandList_ = nullptr;	// コマンドリスト
	std::unique_ptr<Resource> resource_ = nullptr;		// リソース
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};			// インデックスバッファビュー
	uint32_t *indexData_ = nullptr;						// インデックスデータ
	size_t indices_ = 0;								// インデックス数
};