#pragma once
#include <wrl/client.h>
#include <d3d12.h>
#include "DirectXTex.h"
#include <string>
#include <memory>

class Device;

/// @brief リソース
class Resource final {
public:
	/// @brief デフォルトコンストラクタ
	Resource() = default;

	/// @brief コンストラクタ
	/// @param device デバイス
	/// @param HeapType ヒープの種類
	/// @param resourceDesc リソースの設定
	/// @param resourceState リソースの初期状態
	/// @param clearValue クリア値
	Resource(Device *device, D3D12_HEAP_TYPE HeapType, const D3D12_RESOURCE_DESC &resourceDesc, D3D12_RESOURCE_STATES resourceState, D3D12_CLEAR_VALUE *clearValue = nullptr);

	/// @brief バッファを作成する
	/// @param device デバイス
	/// @param HeapType ヒープの種類
	/// @param size バッファの大きさ
	/// @param resourceState リソースの初期状態
	/// @param resourceFlag リソースのフラグ
	/// @return バッファ
	static std::unique_ptr<Resource> CreateBuffer(Device *device, D3D12_HEAP_TYPE HeapType, size_t size, D3D12_RESOURCE_STATES resourceState, D3D12_RESOURCE_FLAGS resourceFlag = D3D12_RESOURCE_FLAG_NONE);

	/// @brief テクスチャを作成する
	/// @param device デバイス
	/// @param metadata メタデータ
	/// @return テクスチャ
	static std::unique_ptr<Resource> CreateTexture(Device *device, const DirectX::TexMetadata &metadata);

	/// @brief 2Dテクスチャを作成する
	/// @param device デバイス
	/// @param width テクスチャの幅
	/// @param height テクスチャの高さ
	/// @param mipLevels ミップレベル数
	/// @param resourceState リソースの初期状態
	/// @param format テクスチャのフォーマット
	/// @param resourceFlag リソースのフラグ
	/// @param clearValue クリア値
	/// @return テクスチャ
	static std::unique_ptr<Resource> CreateTexture2D(Device *device, size_t width, size_t height, uint16_t mipLevels, D3D12_RESOURCE_STATES resourceState, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS resourceFlag = D3D12_RESOURCE_FLAG_NONE, D3D12_CLEAR_VALUE *clearValue = nullptr);

	/// @brief アップロードバッファを作成する
	/// @param device デバイス
	/// @param size アップロードバッファの大きさ
	/// @return アップロードバッファ
	static std::unique_ptr<Resource> CreateUploadBuffer(Device *device, size_t size);

	/// @brief リードバックバッファを作成する
	/// @param device デバイス
	/// @param size リードバックバッファの大きさ
	/// @return リードバックバッファ
	static std::unique_ptr<Resource> CreateReadbackBuffer(Device *device, size_t size);

	/// @brief 読み書き可能バッファを作成する
	/// @param device デバイス
	/// @param size 読み書き可能バッファの大きさ
	/// @return 読み書き可能バッファ
	static std::unique_ptr<Resource> CreateRWBuffer(Device *device, size_t size);

	/// @brief UAVに対応しているか
	/// @param device デバイス
	/// @param format フォーマット
	/// @return UAVに対応しているか
	static bool IsUAVCompatible(ID3D12Device *device, DXGI_FORMAT format);

	/// @brief リソースの名前を設定する
	/// @param name 名前
	void SetName(const std::string &name);

	/// @brief リソースのマッピング
	/// @param ppData リソースのデータ
	void Map(void **ppData);

	/// @brief リソースのアンマッピング
	void Unmap();

	/// @brief リソースのコピー
	/// @param srcResource コピー元のリソース
	void CopyFrom(const Microsoft::WRL::ComPtr<ID3D12Resource> &srcResource);

	/// @brief リソースのコピー
	/// @param srcResource コピー元のリソース
	/// @param dstOffset コピー先のオフセット
	/// @param srcOffset コピー元のオフセット
	/// @param numBytes コピーするバイト数
	void CopyFrom(const Microsoft::WRL::ComPtr<ID3D12Resource> &srcResource, UINT64 dstOffset, UINT64 srcOffset, UINT64 numBytes);

	/// @brief UAVバリアの設定
	void UAVBarrier();

	/// @brief リソースバリアの遷移
	/// @param afterState 遷移後の状態
	/// @param subresource サブリソース
	void TransitionBarrier(D3D12_RESOURCE_STATES afterState, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

	/// @brief リソースバリアの遷移
	/// @param beforeState 遷移前の状態
	/// @param afterState 遷移後の状態
	/// @param subresource サブリソース
	void TransitionBarrier(D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

	/// @brief リソースバリアの遷移
	/// @param commandList コマンドリスト
	/// @param resource リソース
	/// @param beforeState 遷移前の状態
	/// @param afterState 遷移後の状態
	/// @param subresource サブリソース
	static void TransitionBarrier(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> &commandList, const Microsoft::WRL::ComPtr<ID3D12Resource> &resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

	/// @brief リソースを取得する
	/// @return リソース
	ID3D12Resource *GetResource() const { return resource_.Get(); }

	/// @brief GPUの仮想アドレスを取得する
	/// @return GPUの仮想アドレス
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return resource_->GetGPUVirtualAddress(); }

private:
	ID3D12GraphicsCommandList *commandList_ = nullptr;			// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_ = nullptr;	// リソース
	D3D12_RESOURCE_STATES resourceState_;						// リソースの状態
};