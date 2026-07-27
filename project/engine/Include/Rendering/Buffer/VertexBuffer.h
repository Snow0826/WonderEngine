#pragma once
#include "VertexData.h"
#include <d3d12.h>
#include <memory>

/// @brief 頂点バッファタイプ
enum class VertexBufferType {
	kVBV,	// 頂点バッファビュー
	kSRV,	// シェーダーリソースビュー
};

class Device;
class Resource;

/// @brief 頂点バッファ
class VertexBuffer final {
public:
	/// @brief 初期化
	/// @param device デバイス
	/// @param vertices 頂点数
	/// @param type バッファタイプ
	void Initialize(Device *device, size_t vertices, VertexBufferType type = VertexBufferType::kVBV);

	/// @brief 頂点バッファビューを取得する
	/// @return 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;

	/// @brief SRVハンドルを取得する
	/// @return SRVハンドル
	uint32_t GetSRVHandle() const;

	/// @brief 頂点データを取得する
	/// @return 頂点データ
	VertexData *GetVertexData() const { return vertexData_; }

	/// @brief 頂点数を取得する
	/// @return 頂点数
	size_t GetVertices() const { return vertices_; }

private:
	std::unique_ptr<Resource> resource_ = nullptr;		// リソース
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};		// 頂点バッファビュー
	uint32_t srvHandle_ = 0;							// SRVハンドル
	VertexData *vertexData_ = nullptr;					// 頂点データ
	size_t vertices_ = 0;								// 頂点数
	VertexBufferType type_ = VertexBufferType::kVBV;	// バッファタイプ
};