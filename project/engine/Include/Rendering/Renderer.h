#pragma once
#include "MeshType.h"
#include "BlendMode.h"
#include <wrl/client.h>
#include <d3d12.h>
#include <fstream>
#include <array>
#include <vector>

class Device;
class DescriptorHeap;
class Registry;
class World;
class DebugRenderer;
class MeshManager;
class TextureManager;
class SkinClusterManager;
class ParticleManager;
class FootprintManager;

/// @brief レンダラー
class Renderer final {
public:
	/// @brief コンストラクタ
	/// @param device デバイス
	Renderer(Device *device);

	/// @brief 初期化
	/// @param logStream ログストリーム
	void Initialize(std::ofstream &logStream);

	/// @brief パーティクルの初期化
	void InitializeParticle();

	/// @brief 描画
	void Render();

	/// @brief レジストリをセットする
	/// @return レジストリ
	void SetRegistry(Registry *registry);

	/// @brief ワールドをセットする
	/// @param world ワールド
	void SetWorld(World *world);

	/// @brief デバッグレンダラーをセットする
	/// @param debugRenderer デバッグレンダラー
	void SetDebugRenderer(DebugRenderer *debugRenderer);

	/// @brief メッシュマネージャーをセットする
	/// @param meshManager メッシュマネージャー
	void SetMeshManager(MeshManager *meshManager);

	/// @brief テクスチャマネージャーをセットする
	/// @param textureManager テクスチャマネージャー
	void SetTextureManager(TextureManager *textureManager);

	/// @brief スキンクラスターマネージャーをセットする
	/// @param skinClusterManager スキンクラスターマネージャー
	void SetSkinClusterManager(SkinClusterManager *skinClusterManager);

	/// @brief パーティクルマネージャーをセットする
	/// @param particleManager パーティクルマネージャー
	void SetParticleManager(ParticleManager *particleManager);

	/// @brief フットプリントマネージャーをセットする
	/// @param footprintManager フットプリントマネージャー
	void SetFootprintManager(FootprintManager *footprintManager);
	
	/// @brief ゲーム終了フラグをセットする
	void SetGameFinished() { isGameFinished_ = true; }

	/// @brief シーンビューの表示フラグをセットする
	/// @param visible 表示フラグ
	void SetSceneViewVisible(bool visible) { isSceneViewVisible_ = visible; }

	/// @brief ゲームビューの表示フラグをセットする
	/// @param visible 表示フラグ
	void SetGameViewVisible(bool visible) { isGameViewVisible_ = visible; }

private:
	using BlendPipelineState = std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<uint32_t>(BlendMode::kCountOfBlendMode)>;
	using MeshPipelineState = std::array<BlendPipelineState, static_cast<uint32_t>(MeshType::kCountOfMeshType)>;
	using MeshCommandSignature = std::array<Microsoft::WRL::ComPtr<ID3D12CommandSignature>, static_cast<uint32_t>(MeshType::kCountOfMeshType)>;
	Device *device_ = nullptr;																	// デバイス
	DescriptorHeap *gpuCbvSrvUavDescriptorHeap_ = nullptr;										// GPU用のCBV,SRV,UAVのディスクリプタヒープ
	DescriptorHeap *cpuCbvSrvUavDescriptorHeap_ = nullptr;										// CPU用のCBV,SRV,UAVのディスクリプタヒープ
	ID3D12GraphicsCommandList *commandList_ = nullptr;											// コマンドリスト
	Registry *registry_ = nullptr;																// レジストリ
	World *world_ = nullptr;																	// ワールド
	DebugRenderer *debugRenderer_ = nullptr;													// デバッグレンダラー
	MeshManager *meshManager_ = nullptr;														// メッシュマネージャー
	TextureManager *textureManager_ = nullptr;													// テクスチャマネージャー
	SkinClusterManager *skinClusterManager_ = nullptr;											// スキンクラスターマネージャー
	ParticleManager *particleManager_ = nullptr;												// パーティクルマネージャー
	FootprintManager *footprintManager_ = nullptr;												// フットプリントマネージャー
	ID3D12RootSignature *object3dRootSignature_ = nullptr;										// Object3d用ルートシグネチャ
	ID3D12RootSignature *ringObject3dRootSignature_ = nullptr;									// RingObject3d用ルートシグネチャ
	ID3D12RootSignature *particleRootSignature_ = nullptr;										// Particle用ルートシグネチャ
	ID3D12RootSignature *ringParticleRootSignature_ = nullptr;									// RingParticle用ルートシグネチャ
	ID3D12RootSignature *lineRootSignature_ = nullptr;											// Line用ルートシグネチャ
	ID3D12RootSignature *skyboxRootSignature_ = nullptr;										// Skybox用ルートシグネチャ
	ID3D12RootSignature *fullscreenRootSignature_ = nullptr;									// Fullscreen用ルートシグネチャ
	ID3D12RootSignature *grayscaleRootSignature_ = nullptr;										// Grayscale用ルートシグネチャ
	ID3D12RootSignature *vignetteRootSignature_ = nullptr;										// Vignette用ルートシグネチャ
	ID3D12RootSignature *boxFilterRootSignature_ = nullptr;										// BoxFilter用ルートシグネチャ
	ID3D12RootSignature *gaussianFilterRootSignature_ = nullptr;								// GaussianFilter用ルートシグネチャ
	ID3D12RootSignature *luminanceBasedOutlineRootSignature_ = nullptr;							// LuminanceBasedOutline用ルートシグネチャ
	ID3D12RootSignature *depthBasedOutlineRootSignature_ = nullptr;								// DepthBasedOutline用ルートシグネチャ
	ID3D12RootSignature *radialBlurRootSignature_ = nullptr;									// RadialBlur用ルートシグネチャ
	ID3D12RootSignature *dissolveRootSignature_ = nullptr;										// Dissolve用ルートシグネチャ
	ID3D12RootSignature *noiseRootSignature_ = nullptr;											// Noise用ルートシグネチャ
	ID3D12RootSignature *skinningRootSignature_ = nullptr;										// スキニング用ルートシグネチャ
	ID3D12RootSignature *initializeParticleRootSignature_ = nullptr;							// パーティクル初期化用ルートシグネチャ
	ID3D12RootSignature *emitParticleRootSignature_ = nullptr;									// パーティクル発生用ルートシグネチャ
	ID3D12RootSignature *updateParticleRootSignature_ = nullptr;								// パーティクル更新用ルートシグネチャ
	ID3D12RootSignature *depthStencilCopyRootSignature_ = nullptr;								// 深度ステンシルテクスチャコピー用ルートシグネチャ
	ID3D12RootSignature *generateHiZMipMapRootSignature_ = nullptr;								// HiZミップマップ生成用ルートシグネチャ
	ID3D12RootSignature *clearMeshCommandStatesRootSignature_ = nullptr;						// メッシュコマンドステートのクリア用ルートシグネチャ
	ID3D12RootSignature *occlusionCullingRootSignature_ = nullptr;								// オクルージョンカリング用ルートシグネチャ
	ID3D12RootSignature *setInstanceCountRootSignature_ = nullptr;								// インスタンス数の反映用ルートシグネチャ
	ID3D12RootSignature *footprintRootSignature_ = nullptr;										// フットプリント用ルートシグネチャ
	ID3D12RootSignature *footprintMapRootSignature_ = nullptr;									// フットプリントマップ用ルートシグネチャ
	MeshPipelineState meshPipelineState_;														// Mesh用パイプラインステート
	MeshPipelineState meshParticlePipelineState_;												// MeshParticle用パイプラインステート
	BlendPipelineState spritePipelineState_;													// Sprite用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> linePipelineState_ = nullptr;					// Line用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> skyboxPipelineState_ = nullptr;					// Skybox用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> fullscreenPipelineState_ = nullptr;				// Fullscreen用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> grayscalePipelineState_ = nullptr;				// Grayscale用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> vignettePipelineState_ = nullptr;				// Vignette用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> boxFilterPipelineState_ = nullptr;				// BoxFilter用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> gaussianFilterPipelineState_ = nullptr;			// GaussianFilter用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> luminanceBasedOutlinePipelineState_ = nullptr;	// LuminanceBasedOutline用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> depthBasedOutlinePipelineState_ = nullptr;		// DepthBasedOutline用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> radialBlurPipelineState_ = nullptr;				// RadialBlur用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> dissolvePipelineState_ = nullptr;				// Dissolve用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> noisePipelineState_ = nullptr;					// Noise用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> skinningPipelineState_ = nullptr;				// スキニング用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> initializeParticlePipelineState_ = nullptr;		// パーティクル初期化用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> emitParticlePipelineState_ = nullptr;			// パーティクル発生用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> updateParticlePipelineState_ = nullptr;			// パーティクル更新用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> depthStencilCopyPipelineState_ = nullptr;		// 深度ステンシルテクスチャコピー用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> generateHiZMipMapPipelineState_ = nullptr;		// HiZミップマップ生成用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> clearMeshCommandStatesPipelineState_ = nullptr;	// メッシュコマンドステートのクリア用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> occlusionCullingPipelineState_ = nullptr;		// オクルージョンカリング用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState>	setInstanceCountPipelineState_ = nullptr;		// インスタンス数の反映用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> footprintPipelineState_ = nullptr;				// フットプリント用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> footprintMapPipelineState_ = nullptr;			// フットプリントマップ用パイプラインステート
	MeshCommandSignature meshCommandSignature_;													// メッシュコマンドシグネチャ
	bool isSceneViewVisible_ = true;															// シーンビューの表示フラグ
	bool isGameViewVisible_ = true;																// ゲームビューの表示フラグ
	bool isGameFinished_ = false;																// ゲーム終了フラグ

	/// @brief パーティクルの発生
	void EmitParticle();

	/// @brief パーティクルの更新
	void UpdateParticle();

	/// @brief スキニング
	void Skinning();

	/// @brief 深度テクスチャをHiZテクスチャにコピー
	void CopyDepthToHiZ();

	/// @brief HiZミップマップ生成
	void GenerateHiZMipMap();

	/// @brief メッシュコマンドステートのクリア
	void ClearMeshCommandStates();

	/// @brief オクルージョンカリング
	void OcclusionCulling();

	/// @brief インスタンス数の反映
	void SetInstanceCount();

	/// @brief フットプリント
	void Footprint();

	/// @brief 結果マップの読み込み
	void LoadResultMap();

	/// @brief シーンビューの描画
	void RenderSceneView();

	/// @brief ゲームビューの描画
	void RenderGameView();

	/// @brief リリース構成の描画
	void RenderRelease();

	/// @brief レンダーターゲットのセットアップ
	/// @param rtvHandle RTVハンドル
	/// @param dsvHandle DSVハンドル
	/// @param clearDepth 深度をクリアするかどうか
	void SetupRenderTarget(uint32_t rtvHandle, uint32_t dsvHandle, bool clearDepth = true);

	/// @brief ワールドの描画
	/// @param cameraBufferLocationIndex カメラのバッファ位置
	void RenderWorld(uint32_t cameraBufferLocationIndex);

	/// @brief メッシュの描画
	/// @param cameraBufferLocationIndex カメラのバッファ位置
	void DrawMesh(uint32_t cameraBufferLocationIndex);

	/// @brief パーティクルの描画
	/// @param cameraBufferLocationIndex カメラのバッファ位置
	void DrawParticle(uint32_t cameraBufferLocationIndex);

	/// @brief スプライトの描画
	void DrawSprite();

	/// @brief スカイボックスの描画
	/// @param cameraBufferLocationIndex カメラのバッファ位置
	void DrawSkybox(uint32_t cameraBufferLocationIndex);

	/// @brief 画像のコピー
	void CopyImage();

	/// @brief ラインの描画
	void DrawLine(uint32_t cameraBufferLocationIndex);
};