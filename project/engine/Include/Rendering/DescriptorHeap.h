#pragma once
#include <wrl/client.h>
#include <d3d12.h>
#include <cstdint>

class Device;

/// @brief ディスクリプタヒープ
class DescriptorHeap final {
public:
	static inline constexpr uint32_t kMaxSRVCount = 512;	// SRVの最大数

	/// @brief コンストラクタ
	DescriptorHeap() = default;

	/// @brief ディスクリプタを作成する
	/// @param device デバイス
	/// @param type ヒープの種類
	/// @param numDescriptors ディスクリプタの数
	/// @param shaderVisible シェーダー内で使用するかどうか
	/// @return ディスクリプタヒープ
	static DescriptorHeap Create(Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible) {
		return DescriptorHeap(device, type, numDescriptors, shaderVisible);
	}

	/// @brief ディスクリプタを割り当てる
	/// @return ディスクリプタのインデックス
	uint32_t AllocateDescriptor();

	/// @brief 描画用のディスクリプタテーブルを設定する
	/// @param rootParameterIndex ルートパラメータのインデックス
	/// @param descriptorIndex ディスクリプタのインデックス
	void BindToGraphics(UINT rootParameterIndex, uint32_t descriptorIndex) const;

	/// @brief Compute用のディスクリプタテーブルを設定する
	/// @param rootParameterIndex ルートパラメータのインデックス
	/// @param descriptorIndex ディスクリプタのインデックス
	void BindToCompute(UINT rootParameterIndex, uint32_t descriptorIndex) const;

	/// @brief CPUディスクリプタハンドルを取得
	/// @param index インデックス
	/// @return CPUディスクリプタハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index) const;

	/// @brief GPUディスクリプタハンドルを取得
	/// @param index インデックス
	/// @return GPUディスクリプタハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index) const;

	/// @brief RTVを作成
	/// @param resource リソース
	/// @param rtvDesc RTVの設定
	/// @param index インデックス
	/// @return CPUディスクリプタハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE CreateRenderTargetView(const Microsoft::WRL::ComPtr<ID3D12Resource> &resource, D3D12_RENDER_TARGET_VIEW_DESC rtvDesc, uint32_t index) const;

	/// @brief SRVを作成
	/// @param resource リソース
	/// @param srvDesc SRVの設定
	/// @param index インデックス
	void CreateShaderResourceView(const Microsoft::WRL::ComPtr<ID3D12Resource> &resource, D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc, uint32_t index) const;

	/// @brief DSVを作成
	/// @param resource リソース
	/// @param dsvDesc DSVの設定
	/// @param index インデックス
	/// @return CPUディスクリプタハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE CreateDepthStencilView(const Microsoft::WRL::ComPtr<ID3D12Resource> &resource, D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc, uint32_t index) const;

	/// @brief UAVを作成
	/// @param resource リソース
	/// @param uavDesc UAVの設定
	/// @param index インデックス
	void CreateUnorderedAccessView(const Microsoft::WRL::ComPtr<ID3D12Resource> &resource, D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc, uint32_t index) const;

	/// @brief UAVを作成
	/// @param resource リソース
	/// @param counterResource カウンターリソース
	/// @param uavDesc UAVの設定
	/// @param index インデックス
	void CreateUnorderedAccessView(const Microsoft::WRL::ComPtr<ID3D12Resource> &resource, const Microsoft::WRL::ComPtr<ID3D12Resource> &counterResource, D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc, uint32_t index) const;

	/// @brief ディスクリプタヒープを取得
	/// @return ディスクリプタヒープ
	ID3D12DescriptorHeap *GetDescriptorHeap() const { return descriptorHeap_.Get(); }

private:
	ID3D12Device *device_ = nullptr;										// デバイス
	ID3D12GraphicsCommandList *commandList_ = nullptr;						// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_ = nullptr;	// ディスクリプタヒープ
	uint32_t descriptorSize_ = 0;											// ディスクリプタサイズ
	uint32_t descriptorIndex_ = 0;											// ディスクリプタのインデックス

	/// @brief コンストラクタ
	/// @param device デバイス
	/// @param type ヒープの種類
	/// @param numDescriptors ディスクリプタの数
	/// @param shaderVisible シェーダー内で使用するかどうか
	DescriptorHeap(Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible);
};