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

	/// @brief スワップチェーンの設定
	void SetupSwapChain();

	/// @brief フレームの終了処理
	void EndFrame();

	/// @brief 解放
	void Finalize() const;

	/// @brief デバイスを取得
	/// @return デバイス
	ID3D12Device *GetDevice() const { return device_.Get(); }

	/// @brief コマンドキューを取得
	/// @return コマンドキュー
	ID3D12CommandQueue *GetCommandQueue() const { return commandQueue_.Get(); }

	/// @brief コマンドリストを取得
	/// @return コマンドリスト
	ID3D12GraphicsCommandList *GetCommandList() const { return commandList_.Get(); }

	/// @brief RTVのディスクリタヒープを取得
	/// @return RTVのディスクリタヒープ
	DescriptorHeap *GetRTVDescriptorHeap() { return &rtvDescriptorHeap_; }

	/// @brief DSVのディスクリタヒープを取得
	/// @return DSVのディスクリタヒープ
	DescriptorHeap *GetDSVDescriptorHeap() { return &dsvDescriptorHeap_; }

	/// @brief GPU用のCBV,SRV,UAVのディスクリプタヒープを取得
	/// @return GPU用のCBV,SRV,UAVのディスクリプタヒープ
	DescriptorHeap *GetGpuCbvSrvUavDescriptorHeap() { return &gpuCbvSrvUavDescriptorHeap_; }

	/// @brief CPU用のCBV,SRV,UAVのディスクリプタヒープを取得
	/// @return CPU用のCBV,SRV,UAVのディスクリプタヒープ
	DescriptorHeap *GetCpuCbvSrvUavDescriptorHeap() { return &cpuCbvSrvUavDescriptorHeap_; }

	/// @brief ビューポートを取得
	/// @return ビューポート
	D3D12_VIEWPORT GetViewport() const { return viewport_; }

	/// @brief シザー矩形を取得
	/// @return シザー矩形
	D3D12_RECT GetScissorRect() const { return scissorRect_; }

	/// @brief 前フレームのメインカメラ用深度ステンシルテクスチャを取得
	/// @return 前フレームのメインカメラ用深度ステンシルテクスチャ
	Resource *GetPreviousMainCameraDepthStencilTexture() const { return previousMainCameraDepthStencilTexture_.get(); }

	/// @brief メインカメラ用深度ステンシルテクスチャを取得
	/// @return メインカメラ用深度ステンシルテクスチャ
	Resource *GetMainCameraDepthStencilTexture() const { return mainCameraDepthStencilTexture_.get(); }

	/// @brief メインカメラ用深度ステンシルテクスチャDSVハンドルを取得
	/// @return メインカメラ用深度ステンシルテクスチャDSVハンドル
	uint32_t GetMainCameraDepthStencilTextureDSVHandle() const { return mainCameraDepthStencilTextureDSVHandle_; }

	/// @brief デバッグカメラ用深度ステンシルテクスチャDSVハンドルを取得
	/// @return デバッグカメラ用深度ステンシルテクスチャDSVハンドル
	uint32_t GetDebugCameraDepthStencilTextureDSVHandle() const { return debugCameraDepthStencilTextureDSVHandle_; }

	/// @brief Object3d用ルートシグネチャを取得
	/// @return Object3d用ルートシグネチャ
	ID3D12RootSignature *GetObject3dRootSignature() const { return object3dRootSignature_.Get(); }

	/// @brief RingObject3d用ルートシグネチャを取得
	/// @return RingObject3d用ルートシグネチャ
	ID3D12RootSignature *GetRingObject3dRootSignature() const { return ringObject3dRootSignature_.Get(); }

	/// @brief Particle用ルートシグネチャを取得
	/// @return Particle用ルートシグネチャ
	ID3D12RootSignature *GetParticleRootSignature() const { return particleRootSignature_.Get(); }

	/// @brief RingParticle用ルートシグネチャを取得
	/// @return RingParticle用ルートシグネチャ
	ID3D12RootSignature *GetRingParticleRootSignature() const { return ringParticleRootSignature_.Get(); }

	/// @brief Line用ルートシグネチャを取得
	/// @return Line用ルートシグネチャ
	ID3D12RootSignature *GetLineRootSignature() const { return lineRootSignature_.Get(); }

	/// @brief Skybox用ルートシグネチャを取得
	/// @return Skybox用ルートシグネチャ
	ID3D12RootSignature *GetSkyboxRootSignature() const { return skyboxRootSignature_.Get(); }

	/// @brief Fullscreen用ルートシグネチャを取得
	/// @return Fullscreen用ルートシグネチャ
	ID3D12RootSignature *GetFullscreenRootSignature() const { return fullscreenRootSignature_.Get(); }

	/// @brief Grayscale用ルートシグネチャを取得
	/// @return Grayscale用ルートシグネチャ
	ID3D12RootSignature *GetGrayscaleRootSignature() const { return grayscaleRootSignature_.Get(); }

	/// @brief Vignette用ルートシグネチャを取得
	/// @return Vignette用ルートシグネチャ
	ID3D12RootSignature *GetVignetteRootSignature() const { return vignetteRootSignature_.Get(); }

	/// @brief BoxFilter用ルートシグネチャを取得
	/// @return BoxFilter用ルートシグネチャ
	ID3D12RootSignature *GetBoxFilterRootSignature() const { return boxFilterRootSignature_.Get(); }

	/// @brief GaussianFilter用ルートシグネチャを取得
	/// @return GaussianFilter用ルートシグネチャ
	ID3D12RootSignature *GetGaussianFilterRootSignature() const { return gaussianFilterRootSignature_.Get(); }

	/// @brief LuminanceBasedOutline用ルートシグネチャを取得
	/// @return LuminanceBasedOutline用ルートシグネチャ
	ID3D12RootSignature *GetLuminanceBasedOutlineRootSignature() const { return luminanceBasedOutlineRootSignature_.Get(); }

	/// @brief DepthBasedOutline用ルートシグネチャを取得
	/// @return DepthBasedOutline用ルートシグネチャ
	ID3D12RootSignature *GetDepthBasedOutlineRootSignature() const { return depthBasedOutlineRootSignature_.Get(); }

	/// @brief RadialBlur用ルートシグネチャを取得
	/// @return RadialBlur用ルートシグネチャ
	ID3D12RootSignature *GetRadialBlurRootSignature() const { return radialBlurRootSignature_.Get(); }

	/// @brief Dissolve用ルートシグネチャを取得
	/// @return Dissolve用ルートシグネチャ
	ID3D12RootSignature *GetDissolveRootSignature() const { return dissolveRootSignature_.Get(); }

	/// @brief Noise用ルートシグネチャを取得
	/// @return Noise用ルートシグネチャ
	ID3D12RootSignature *GetNoiseRootSignature() const { return noiseRootSignature_.Get(); }

	/// @brief スキニング用ルートシグネチャを取得
	/// @return スキニング用ルートシグネチャ
	ID3D12RootSignature *GetSkinningRootSignature() const { return skinningRootSignature_.Get(); }

	/// @brief パーティクル初期化用ルートシグネチャを取得
	/// @return パーティクル初期化用ルートシグネチャ
	ID3D12RootSignature *GetInitializeParticleRootSignature() const { return initializeParticleRootSignature_.Get(); }

	/// @brief パーティクル発生用ルートシグネチャを取得
	/// @return パーティクル発生用ルートシグネチャ
	ID3D12RootSignature *GetEmitParticleRootSignature() const { return emitParticleRootSignature_.Get(); }

	/// @brief パーティクル更新用ルートシグネチャを取得
	/// @return パーティクル更新用ルートシグネチャ
	ID3D12RootSignature *GetUpdateParticleRootSignature() const { return updateParticleRootSignature_.Get(); }

	/// @brief 深度ステンシルテクスチャコピー用ルートシグネチャを取得
	/// @return 深度ステンシルテクスチャコピー用ルートシグネチャ
	ID3D12RootSignature *GetDepthStencilCopyRootSignature() const { return depthStencilCopyRootSignature_.Get(); }

	/// @brief HiZミップマップ生成用ルートシグネチャを取得
	/// @return HiZミップマップ生成用ルートシグネチャ
	ID3D12RootSignature *GetGenerateHiZMipMapRootSignature() const { return generateHiZMipMapRootSignature_.Get(); }

	/// @brief オクルージョンカリング用ルートシグネチャを取得
	/// @return オクルージョンカリング用ルートシグネチャ
	ID3D12RootSignature *GetOcclusionCullingRootSignature() const { return occlusionCullingRootSignature_.Get(); }

	/// @brief メッシュコマンドステートのクリア用ルートシグネチャを取得
	/// @return メッシュコマンドステートのクリア用ルートシグネチャ
	ID3D12RootSignature *GetClearMeshCommandStatesRootSignature() const { return clearMeshCommandStatesRootSignature_.Get(); }

	/// @brief インスタンス数の反映用ルートシグネチャを取得
	/// @return インスタンス数の反映用ルートシグネチャ
	ID3D12RootSignature *GetSetInstanceCountRootSignature() const { return setInstanceCountRootSignature_.Get(); }

	/// @brief フットプリント用ルートシグネチャを取得
	/// @return フットプリント用ルートシグネチャ
	ID3D12RootSignature *GetFootprintRootSignature() const { return footprintRootSignature_.Get(); }
	
	/// @brief フットプリントマップ用ルートシグネチャを取得
	/// @return フットプリントマップ用ルートシグネチャ
	ID3D12RootSignature *GetFootprintMapRootSignature() const { return footprintMapRootSignature_.Get(); }

private:
	std::chrono::steady_clock::time_point reference_;											// 基準時間(FPS固定用)
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;								// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter_ = nullptr;									// アダプタ
	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;										// デバイス
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue_ = nullptr;								// デバッグ用の情報キュー
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;							// コマンドキュー
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;					// コマンドアロケータ
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;					// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;										// フェンス
	uint32_t fenceValue_ = 0;																	// フェンスの値
	HANDLE fenceEvent_{};																		// フェンスを待つイベント
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;								// スワップチェーン
	DescriptorHeap rtvDescriptorHeap_;															// RTV用のディスクリプタヒープ
	DescriptorHeap gpuCbvSrvUavDescriptorHeap_;													// GPU用のCBV,SRV,UAV用のディスクリプタヒープ
	DescriptorHeap cpuCbvSrvUavDescriptorHeap_;													// CPU用のCBV,SRV,UAV用のディスクリプタヒープ
	DescriptorHeap dsvDescriptorHeap_;															// DSV用のディスクリプタヒープ
	std::vector<uint32_t> swapChainResourceRTVHandles_;											// スワップチェーンリソースRTVハンドル
	uint32_t mainCameraDepthStencilTextureDSVHandle_ = 0;										// メインカメラ用深度ステンシルテクスチャDSVハンドル
	uint32_t debugCameraDepthStencilTextureDSVHandle_ = 0;										// デバッグカメラ用深度ステンシルテクスチャDSVハンドル
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> swapChainResources_;					// スワップチェーンリソース
	std::unique_ptr<Resource> previousMainCameraDepthStencilTexture_ = nullptr;					// 前フレームのメインカメラ用深度ステンシルテクスチャ
	std::unique_ptr<Resource> mainCameraDepthStencilTexture_ = nullptr;							// メインカメラ用深度ステンシルテクスチャ
	std::unique_ptr<Resource> debugCameraDepthStencilTexture_ = nullptr;						// デバッグカメラ用深度ステンシルテクスチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> object3dRootSignature_ = nullptr;				// Object3d用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> ringObject3dRootSignature_ = nullptr;			// RingObject3d用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> particleRootSignature_ = nullptr;				// Particle用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> ringParticleRootSignature_ = nullptr;			// RingParticle用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> lineRootSignature_ = nullptr;					// Line用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> skyboxRootSignature_ = nullptr;					// Skybox用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> fullscreenRootSignature_ = nullptr;				// Fullscreen用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> grayscaleRootSignature_ = nullptr;				// Grayscale用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> vignetteRootSignature_ = nullptr;				// Vignette用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> boxFilterRootSignature_ = nullptr;				// BoxFilter用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> gaussianFilterRootSignature_ = nullptr;			// GaussianFilter用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> luminanceBasedOutlineRootSignature_ = nullptr;	// LuminanceBasedOutline用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> depthBasedOutlineRootSignature_ = nullptr;		// DepthBasedOutline用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> radialBlurRootSignature_ = nullptr;				// RadialBlur用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> dissolveRootSignature_ = nullptr;				// Dissolve用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> noiseRootSignature_ = nullptr;					// Noise用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> skinningRootSignature_ = nullptr;				// スキニング用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> initializeParticleRootSignature_ = nullptr;		// パーティクル初期化用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> emitParticleRootSignature_ = nullptr;			// パーティクル発生用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> updateParticleRootSignature_ = nullptr;			// パーティクル更新用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> depthStencilCopyRootSignature_ = nullptr;		// 深度ステンシルテクスチャコピー用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> generateHiZMipMapRootSignature_ = nullptr;		// HiZミップマップ生成用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> clearMeshCommandStatesRootSignature_ = nullptr;	// メッシュコマンドステートのクリア用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> occlusionCullingRootSignature_ = nullptr;		// オクルージョンカリング用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> setInstanceCountRootSignature_ = nullptr;		// インスタンス数の反映用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> footprintRootSignature_ = nullptr;				// フットプリント用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> footprintMapRootSignature_ = nullptr;			// フットプリントマップ用ルートシグネチャ
	D3D12_VIEWPORT viewport_{};																	// ビューポート
	D3D12_RECT scissorRect_{};																	// シザー矩形

	/// @brief FPS固定初期化
	void InitializeFixFPS();

	/// @brief FPS固定更新
	void UpdateFixFPS();

	/// @brief デバッグレイヤーを有効にする
	void EnableDebugLayer() const;
};