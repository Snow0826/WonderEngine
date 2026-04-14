#pragma once
#include "BlendMode.h"
#include <wrl/client.h>
#include <d3d12.h>
#include <fstream>
#include <array>

class Device;
class DescriptorHeap;
class Registry;
class World;
class DebugRenderer;
class IndirectCommandManager;
class MeshManager;
class TextureManager;
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

	/// @brief 間接コマンドマネージャーをセットする
	/// @param indirectCommandManager 間接コマンドマネージャー
	void SetIndirectCommandManager(IndirectCommandManager *indirectCommandManager);

	/// @brief メッシュマネージャーをセットする
	/// @param meshManager メッシュマネージャー
	void SetMeshManager(MeshManager *meshManager);

	/// @brief テクスチャマネージャーをセットする
	/// @param textureManager テクスチャマネージャー
	void SetTextureManager(TextureManager *textureManager);

	/// @brief フットプリントマネージャーをセットする
	/// @param footprintManager フットプリントマネージャー
	void SetFootprintManager(FootprintManager *footprintManager);

	/// @brief ゲーム終了フラグをセットする
	void SetGameFinished() { isGameFinished_ = true; }

private:
	using BlendPipelineState = std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<uint32_t>(BlendMode::kCountOfBlendMode)>;
	Device *device_ = nullptr;																// デバイス
	DescriptorHeap *gpuCbvSrvUavDescriptorHeap_ = nullptr;									// GPU用のCBV,SRV,UAVのディスクリプタヒープ
	DescriptorHeap *cpuCbvSrvUavDescriptorHeap_ = nullptr;									// CPU用のCBV,SRV,UAVのディスクリプタヒープ
	ID3D12GraphicsCommandList *commandList_ = nullptr;										// コマンドリスト
	Registry *registry_ = nullptr;															// レジストリ
	World *world_ = nullptr;																// ワールド
	DebugRenderer *debugRenderer_ = nullptr;												// デバッグレンダラー
	IndirectCommandManager *indirectCommandManager_ = nullptr;								// 間接コマンドマネージャー
	MeshManager *meshManager_ = nullptr;													// メッシュマネージャー
	TextureManager *textureManager_ = nullptr;												// テクスチャマネージャー
	FootprintManager *footprintManager_ = nullptr;											// フットプリントマネージャー
	ID3D12RootSignature *object3dRootSignature_ = nullptr;									// Object3d用ルートシグネチャ
	ID3D12RootSignature *instance3dRootSignature_ = nullptr;								// Instance3d用ルートシグネチャ
	ID3D12RootSignature *lineRootSignature_ = nullptr;										// Line用ルートシグネチャ
	ID3D12RootSignature *depthStencilCopyRootSignature_ = nullptr;							// 深度ステンシルテクスチャコピー用ルートシグネチャ
	ID3D12RootSignature *generateHiZMipMapRootSignature_ = nullptr;							// HiZミップマップ生成用ルートシグネチャ
	ID3D12RootSignature *occlusionCullingRootSignature_ = nullptr;							// オクルージョンカリング用ルートシグネチャ
	ID3D12RootSignature *footprintRootSignature_ = nullptr;									// フットプリント用ルートシグネチャ
	ID3D12RootSignature *footprintMapRootSignature_ = nullptr;								// フットプリントマップ用ルートシグネチャ
	BlendPipelineState modelPipelineState_;													// Model用パイプラインステート
	BlendPipelineState spritePipelineState_;												// Sprite用パイプラインステート
	BlendPipelineState particlePipelineState_;												// Particle用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> linePipelineState_ = nullptr;				// Line用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> depthStencilCopyPipelineState_ = nullptr;	// 深度ステンシルテクスチャコピー用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> generateHiZMipMapPipelineState_ = nullptr;	// HiZミップマップ生成用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> occlusionCullingPipelineState_ = nullptr;	// オクルージョンカリング用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> footprintPipelineState_ = nullptr;			// フットプリント用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> footprintMapPipelineState_ = nullptr;		// フットプリントマップ用パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12CommandSignature> commandSignature_ = nullptr;				// コマンドシグネチャ
	bool isGameFinished_ = false;															// ゲーム終了フラグ

	/// @brief デバッグカメラかどうか取得
	/// @return デバッグカメラならtrue
	bool IsDebugCamera();

	/// @brief 深度テクスチャをHiZテクスチャにコピー
	void CopyDepthToHiZ();

	/// @brief HiZミップマップ生成
	void GenerateHiZMipMap();

	/// @brief オクルージョンカリング
	void OcclusionCulling();

	/// @brief フットプリント
	void Footprint();

	/// @brief 結果マップの読み込み
	void LoadResultMap();

	/// @brief モデルの描画前処理
	void PreDrawModel();

	/// @brief カリングモデルの描画
	void DrawModel();

	/// @brief パーティクルの描画前処理
	void PreDrawParticle();

	/// @brief パーティクルの描画
	void DrawParticle();

	/// @brief スプライトの描画前処理
	void PreDrawSprite();

	/// @brief スプライトの描画
	void DrawSprite();

	/// @brief ラインの描画前処理
	void PreDrawLine();

	/// @brief ラインの描画
	void DrawLine();
};