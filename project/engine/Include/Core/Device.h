#pragma once
#include "DescriptorHeap.h"
#include <dxgi1_6.h>
#include <fstream>
#include <vector>
#include <chrono>

class Window;
class Resource;

/// @brief デバイス
class Device final {
public:
	/// @brief デフォルトコンストラクタ
	Device();

	/// @brief デフォルトデストラクタ
	~Device();

	/// @brief 初期化
	/// @param logStream ログストリーム
	/// @param window ウィンドウ
	void Initialize(std::ofstream &logStream, const Window &window);

	/// @brief フレームの開始処理
	void NewFrame();

	/// @brief フレームの終了処理
	void EndFrame();

	/// @brief 解放
	void Finalize() const;

	/// @brief デバイスを取得
	/// @return デバイス
	ID3D12Device *GetDevice() const { return device_.Get(); }

	/// @brief コマンドリストを取得
	/// @return コマンドリスト
	ID3D12GraphicsCommandList *GetCommandList() const { return commandList_.Get(); }

	/// @brief GPU用のCBV,SRV,UAVのディスクリプタヒープを取得
	/// @return GPU用のCBV,SRV,UAVのディスクリプタヒープ
	DescriptorHeap *GetGpuCbvSrvUavDescriptorHeap_() { return &gpuCbvSrvUavDescriptorHeap_; }

	/// @brief CPU用のCBV,SRV,UAVのディスクリプタヒープを取得
	/// @return CPU用のCBV,SRV,UAVのディスクリプタヒープ
	DescriptorHeap *GetCpuCbvSrvUavDescriptorHeap_() { return &cpuCbvSrvUavDescriptorHeap_; }

	/// @brief ビューポートを取得
	/// @return ビューポート
	D3D12_VIEWPORT GetViewport() const { return viewport_; }

	/// @brief 前フレームの深度ステンシルテクスチャを取得
	/// @return 前フレームの深度ステンシルテクスチャ
	Resource *GetPreviousDepthStencilTexture() const { return previousDepthStencilTexture_.get(); }

	/// @brief DSVハンドルを取得
	/// @return DSVハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const { return dsvHandle_; }

	/// @brief Object3d用ルートシグネチャを取得
	/// @return Object3d用ルートシグネチャ
	ID3D12RootSignature *GetObject3dRootSignature() const { return object3dRootSignature_.Get(); }

	/// @brief Instance3d用ルートシグネチャを取得
	/// @return Instance3d用ルートシグネチャ
	ID3D12RootSignature *GetInstance3dRootSignature() const { return instance3dRootSignature_.Get(); }

	/// @brief Line用ルートシグネチャを取得
	/// @return Line用ルートシグネチャ
	ID3D12RootSignature *GetLineRootSignature() const { return lineRootSignature_.Get(); }

	// @brief 深度ステンシルテクスチャコピー用ルートシグネチャを取得
	/// @return 深度ステンシルテクスチャコピー用ルートシグネチャ
	ID3D12RootSignature *GetDepthStencilCopyRootSignature() const { return depthStencilCopyRootSignature_.Get(); }

	/// @brief HiZミップマップ生成用ルートシグネチャを取得
	/// @return HiZミップマップ生成用ルートシグネチャ
	ID3D12RootSignature *GetGenerateHiZMipMapRootSignature() const { return generateHiZMipMapRootSignature_.Get(); }

	/// @brief オクルージョンカリング用ルートシグネチャを取得
	/// @return オクルージョンカリング用ルートシグネチャ
	ID3D12RootSignature *GetOcclusionCullingRootSignature() const { return occlusionCullingRootSignature_.Get(); }

	/// @brief フットプリント用ルートシグネチャを取得
	/// @return フットプリント用ルートシグネチャ
	ID3D12RootSignature *GetFootprintRootSignature() const { return footprintRootSignature_.Get(); }
	
	/// @brief フットプリントマップ用ルートシグネチャを取得
	/// @return フットプリントマップ用ルートシグネチャ
	ID3D12RootSignature *GetFootprintMapRootSignature() const { return footprintMapRootSignature_.Get(); }

private:
	std::chrono::steady_clock::time_point reference_;										// 基準時間(FPS固定用)
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;							// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter_ = nullptr;								// アダプタ
	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;									// デバイス
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue_ = nullptr;							// デバッグ用の情報キュー
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;						// コマンドキュー
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;				// コマンドアロケータ
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;				// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;									// フェンス
	uint32_t fenceValue_ = 0;																// フェンスの値
	HANDLE fenceEvent_{};																	// フェンスを待つイベント
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;							// スワップチェーン
	DescriptorHeap rtvDescriptorHeap_;														// RTV用のディスクリプタヒープ
	DescriptorHeap gpuCbvSrvUavDescriptorHeap_;												// GPU用のCBV,SRV,UAV用のディスクリプタヒープ
	DescriptorHeap cpuCbvSrvUavDescriptorHeap_;												// CPU用のCBV,SRV,UAV用のディスクリプタヒープ
	DescriptorHeap dsvDescriptorHeap_;														// DSV用のディスクリプタヒープ
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles_;									// RTVハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_{};												// DSVハンドル
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> swapChainResources_;				// スワップチェーンリソース
	std::unique_ptr<Resource> previousDepthStencilTexture_ = nullptr;						// 前フレームの深度ステンシルテクスチャ
	std::unique_ptr<Resource> depthStencilTexture_ = nullptr;								// 深度ステンシルテクスチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> object3dRootSignature_ = nullptr;			// Object3d用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> instance3dRootSignature_ = nullptr;			// Instance3d用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> lineRootSignature_ = nullptr;				// Line用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> depthStencilCopyRootSignature_ = nullptr;	// 深度ステンシルテクスチャコピー用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> generateHiZMipMapRootSignature_ = nullptr;	// HiZミップマップ生成用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> occlusionCullingRootSignature_ = nullptr;	// オクルージョンカリング用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> footprintRootSignature_ = nullptr;			// フットプリント用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> footprintMapRootSignature_ = nullptr;		// フットプリントマップ用ルートシグネチャ
	D3D12_VIEWPORT viewport_{};																// ビューポート
	D3D12_RECT scissorRect_{};																// シザー矩形

	/// @brief FPS固定初期化
	void InitializeFixFPS();

	/// @brief FPS固定更新
	void UpdateFixFPS();

	/// @brief デバッグレイヤーを有効にする
	void EnableDebugLayer() const;
};