#pragma once
#include "MeshType.h"
#include "BlendMode.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "CPUTimer.h"
#include <array>
#include <vector>
#include <memory>
#include <d3d12.h>

/// @brief uint型4要素ベクトル
struct Uint4 final {
	uint32_t x = 0;
	uint32_t y = 0;
	uint32_t z = 0;
	uint32_t w = 0;
};

/// @brief キューオフセットリスト
using QueueOffsets = std::array<Uint4, (static_cast<uint32_t>(MeshType::kCountOfMeshType) *static_cast<uint32_t>(BlendMode::kCountOfBlendMode) + 3) / 4>;

/// @brief カリング定数データ
struct CullingConstantsData final {
	uint32_t meshCount = 0;		// メッシュの数
	uint32_t padding[3] = {};	// パディング
	QueueOffsets queueOffsets;	// キューオフセットリスト
};

/// @brief ライトデータ
struct LightData final {
	uint32_t pointLightCount = 0;	// 点光源の数
	uint32_t spotLightCount = 0;	// スポットライトの数
	uint32_t pointLightHandle = 0;	// 点光源のSRVハンドル
	uint32_t spotLightHandle = 0;	// スポットライトのSRVハンドル
};

/// @brief グレースケールカラー
struct GrayscaleColor final {
	float r = 1.0f;	// 赤
	float g = 1.0f;	// 緑
	float b = 1.0f;	// 青
};

/// @brief ビネットパラメータ
struct VignetteParam final {
	float scale = 16.0f;	// 倍率
	float intensity = 0.8f;	// 強さ
};

/// @brief ボックスフィルターパラメータ
struct BoxFilterParam final {
	int32_t kernelRadius = 1;	// カーネル半径
	Vector2 texelSize;			// テクセルサイズ
};

/// @brief ガウシアンフィルターパラメータ
struct GaussianFilterParam final {
	int32_t kernelRadius = 1;	// カーネル半径
	Vector2 texelSize;			// テクセルサイズ
	float sigma = 2.0f;			// 標準偏差
};

/// @brief プレウィットフィルターパラメータ
struct PrewittFilterParam final {
	Vector2 texelSize;	// テクセルサイズ
	float scale = 1.0f;	// スケール
};

/// @brief 深度マテリアル
struct DepthMaterial final {
	Matrix4x4 projectionInverse;	// プロジェクション逆行列
};

/// @brief ラジアルブラーのパラメータ
struct RadialBlurParam final {
	Vector2 center = { 0.5f, 0.5f };	// 中心座標
	float blurWidth = 0.01f;			// ブラー幅
	uint32_t sampleCount = 10;			// サンプル数
};

/// @brief ディゾルブのパラメータ
struct DissolveParam final {
	float threshold = 0.5f;		// 閾値
	float edgeWidth = 0.03f;	// エッジ幅
	Vector3 edgeColor = { 1.0f, 1.0f, 1.0f };	// エッジカラー
};

/// @brief ノイズのパラメータ
struct NoiseParam final {
	float time = 0.0f;	// 時間
};

/// @brief int型4要素ベクトル
struct Int4 final {
	int32_t x = 0;
	int32_t y = 0;
	int32_t z = 0;
	int32_t w = 0;
};

/// @brief フットプリント(GPU)
struct FootprintForGPU final {
	Vector3 worldPos;		// ワールド座標
	float radius = 0.0f;	// 半径
	Vector4 color;			// 色
};

/// @brief 定数バッファの種類
enum class ConstantBufferType {
	kTransform,					// ワールド変換データ
	kViewProjection,			// ビュープロジェクション
	kMaterial,					// マテリアル
	kCameraPosition,			// カメラ座標
	kDirectionalLight,			// 平行光源
	kFrustum,					// 視錐台
	kGrayscaleColor,			// グレースケールカラー
	kVignetteParam,				// ビネットパラメータ
	kBoxFilterParam,			// ボックスフィルターパラメータ
	kGaussianFilterParam,		// ガウシアンフィルターパラメータ
	kPrewittFilterParam,		// プレウィットフィルターパラメータ
	kDepthMaterial,				// 深度マテリアル
	kRadialBlurParam,			// ラジアルブラーのパラメータ
	kDissolveParam,				// ディゾルブのパラメータ
	kNoiseParam,				// ノイズのパラメータ
	kFootprintMap,				// フットプリントマップ
	kCountOfConstantBufferType	// 定数バッファの種類の数
};

/// @brief 構造化バッファの種類
enum class StructuredBufferType {
	kLine,							// ライン
	kPointLight,					// 点光源
	kSpotLight,						// スポットライト
	kObject,						// オブジェクト
	kMesh,							// メッシュ
	kMeshLOD,						// メッシュLOD
	kFootprint,						// フットプリント
	kCountOfStructuredBufferType	// 構造化バッファの種類の数
};

/// @brief ポストエフェクト
enum class PostEffect {
	kNone,					// なし
	kGrayscale,				// グレースケール
	kVignette,				// ビネット
	kBoxFilter,				// ボックスフィルター
	kGaussianFilter,		// ガウシアンフィルター
	kLuminanceBasedOutline,	// 輝度ベースの輪郭抽出
	kDepthBasedOutline,		// 深度ベースの輪郭抽出
	kRadialBlur,			// ラジアルブラー
	kDissolve,				// ディゾルブ
	kNoise,					// ノイズ
	kCountOfPostEffect		// ポストエフェクトの数
};

class Device;
class Registry;
class ConstantBuffer;
class Resource;
namespace Rendering {
	struct Line;
}
struct PointLight;
struct SpotLight;

/// @brief ワールド
class World final {
public:
	/// @brief コンストラクタ
	/// @param device デバイス
	/// @param logStream ログストリーム
	World(Device *device, std::ofstream &logStream);

	/// @brief デストラクタ
	~World();

	/// @brief 更新
	void Update();

	/// @brief 編集
	void Edit();

	/// @brief レジストリを設定
	/// @param registry レジストリ
	void SetRegistry(Registry *registry) { registry_ = registry; }

	/// @brief フットプリントマップバッファをコピー
	void CopyFootprintMapBuffer();

	/// @brief ラインデータをコピー
	/// @param data コピーするデータ
	/// @param size データの大きさ
	void CopyLineData(const Rendering::Line *data, size_t size);

	/// @brief 最大AABB数を取得
	/// @return 最大AABB数
	constexpr uint32_t GetMaxAABB() const { return kMaxAABB; }

	/// @brief キューあたりの最大コマンド数を取得
	/// @return キューあたりの最大コマンド数
	constexpr uint32_t GetMaxCommandPerQueue() const { return kMaxCommandPerQueue; }

	/// @brief 定数バッファを取得
	/// @param type 定数バッファの種類
	ConstantBuffer *GetConstantBuffer(ConstantBufferType type) { return constantBuffers_[static_cast<size_t>(type)].get(); }

	/// @brief 構造化バッファを取得
	/// @param type 構造化バッファの種類
	/// @return 構造化バッファ
	Resource *GetStructuredBuffer(StructuredBufferType type) { return structuredBuffers_[static_cast<size_t>(type)].get(); }

	/// @brief シーンのレンダーテクスチャを取得
	/// @return シーンのレンダーテクスチャ
	Resource *GetSceneRenderTexture() { return sceneRenderTexture_.get(); }

	/// @brief ゲームのレンダーテクスチャを取得
	/// @return ゲームのレンダーテクスチャ
	Resource *GetGameRenderTexture() { return gameRenderTexture_.get(); }

	/// @brief ポストエフェクトのレンダーテクスチャを取得
	/// @return ポストエフェクトのレンダーテクスチャ
	Resource *GetPostEffectRenderTexture() { return postEffectRenderTexture_.get(); }

	/// @brief Hi-Zテクスチャを取得
	/// @return Hi-Zテクスチャ
	Resource *GetHiZTexture() { return hiZTexture_.get(); }

	/// @brief コマンドバッファアップロードを取得
	/// @return コマンドバッファアップロード
	Resource *GetCommandBufferUpload() { return commandBufferUpload_.get(); }

	/// @brief カリング済みコマンドバッファを取得
	/// @return カリング済みコマンドバッファ
	Resource *GetProcessedCommandBuffer() { return processedCommandBuffer_.get(); }

	/// @brief コマンドカウンターバッファを取得
	/// @return コマンドカウンターバッファ
	Resource *GetCommandCounterBuffer() { return commandCounterBuffer_.get(); }

	/// @brief フットプリントマップバッファを取得
	/// @return フットプリントマップバッファ
	Resource *GetFootprintMapBuffer() { return footprintMapBuffer_.get(); }

	/// @brief キューオフセットリストを取得
	/// @return キューオフセットリスト
	QueueOffsets GetQueueOffsets() const { return queueOffsets_; }

	/// @brief フットプリントマップデータを取得
	/// @return フットプリントマップデータ
	Int4 GetColor() const { return colorData_[0]; }

	/// @brief ミップレベル数を取得
	/// @return ミップレベル数
	uint32_t GetMipLevels() const { return mipLevels_; }

	/// @brief シーンのレンダーテクスチャのRTVハンドルを取得
	/// @return シーンのレンダーテクスチャのRTVハンドル
	uint32_t GetSceneRenderTextureRTVHandle() const { return sceneRenderTextureRTVHandle_; }

	/// @brief シーンのレンダーテクスチャのSRVハンドルを取得
	/// @return シーンのレンダーテクスチャのSRVハンドル
	uint32_t GetSceneRenderTextureSRVHandle() const { return sceneRenderTextureSRVHandle_; }

	/// @brief ゲームのレンダーテクスチャのRTVハンドルを取得
	/// @return ゲームのレンダーテクスチャのRTVハンドル
	uint32_t GetGameRenderTextureRTVHandle() const { return gameRenderTextureRTVHandle_; }

	/// @brief ゲームのレンダーテクスチャのSRVハンドルを取得
	/// @return ゲームのレンダーテクスチャのSRVハンドル
	uint32_t GetGameRenderTextureSRVHandle() const { return gameRenderTextureSRVHandle_; }

	/// @brief ポストエフェクトのレンダーテクスチャのRTVハンドルを取得
	/// @return ポストエフェクトのレンダーテクスチャのRTVハンドル
	uint32_t GetPostEffectRenderTextureRTVHandle() const { return postEffectRenderTextureRTVHandle_; }

	/// @brief ポストエフェクトのレンダーテクスチャのSRVハンドルを取得
	/// @return ポストエフェクトのレンダーテクスチャのSRVハンドル
	uint32_t GetPostEffectRenderTextureSRVHandle() const { return postEffectRenderTextureSRVHandle_; }

	/// @brief 前フレームのメインカメラの深度ステンシルテクスチャのSRVハンドルを取得
	/// @return 前フレームのメインカメラの深度ステンシルテクスチャのSRVハンドル
	uint32_t GetPreviousMainCameraDepthStencilTextureSRVHandle() const { return previousMainCameraDepthStencilTextureSRVHandle_; }

	/// @brief メインカメラの深度ステンシルテクスチャのSRVハンドルを取得
	/// @return メインカメラの深度ステンシルテクスチャのSRVハンドル
	uint32_t GetMainCameraDepthStencilTextureSRVHandle() const { return mainCameraDepthStencilTextureSRVHandle_; }

	/// @brief Hi-ZテクスチャのSRVハンドルを取得
	/// @return Hi-ZテクスチャのSRVハンドル
	uint32_t GetHiZTextureSRVHandle() const { return hiZTextureSRVHandle_; }

	/// @brief Hi-ZテクスチャのUAVハンドルを取得
	/// @return Hi-ZテクスチャのUAVハンドル
	uint32_t GetHiZTextureUAVHandle() const { return hiZTextureUAVHandle_; }

	/// @brief Hi-Zミップマップ読み取りハンドルを取得
	/// @param index インデックス
	/// @return Hi-Zミップマップ読み取りハンドル
	uint32_t GetHiZMipMapReadHandle(uint32_t index) const { return hiZMipMapReadHandles_[index]; }

	/// @brief Hi-Zミップマップ書き込みハンドルを取得
	/// @param index インデックス
	/// @return Hi-Zミップマップ書き込みハンドル
	uint32_t GetHiZMipMapWriteHandle(uint32_t index) const { return hiZMipMapWriteHandles_[index]; }

	/// @brief フットプリントハンドルを取得
	/// @return フットプリントハンドル
	uint32_t GetFootprintHandle() const { return footprintHandle_; }

	/// @brief フットプリントマップハンドルを取得
	/// @return フットプリントマップハンドル
	uint32_t GetFootprintMapHandle() const { return footprintMapHandle_; }

	/// @brief ラインハンドルを取得
	/// @return ラインハンドル
	uint32_t GetLineHandle() const { return lineHandle_; }

	/// @brief 点光源ハンドルを取得
	/// @return 点光源ハンドル
	uint32_t GetPointLightHandle() const { return pointLightHandle_; }

	/// @brief スポットライトハンドルを取得
	/// @return スポットライトハンドル
	uint32_t GetSpotLightHandle() const { return spotLightHandle_; }

	/// @brief カリングオブジェクトハンドルを取得
	/// @return カリングオブジェクトハンドル
	uint32_t GetCullingObjectHandle() const { return cullingObjectHandle_; }

	/// @brief カリングメッシュハンドルを取得
	/// @return カリングメッシュハンドル
	uint32_t GetCullingMeshHandle() const { return cullingMeshHandle_; }

	/// @brief メッシュLODハンドルを取得
	/// @return メッシュLODハンドル
	uint32_t GetMeshLODHandle() const { return meshLODHandle_; }

	/// @brief カリング済みコマンドハンドルを取得
	/// @return カリング済みコマンドハンドル
	uint32_t GetProcessedCommandHandle() const { return processedCommandHandle_; }

	/// @brief コマンドカウンターハンドルを取得
	/// @return コマンドカウンターハンドル
	uint32_t GetCommandCounterHandle() const { return commandCounterHandle_; }

	/// @brief 結果表示フラグを取得
	/// @return 結果表示フラグ
	bool IsResult() const { return isResult_; }

	/// @brief 結果表示フラグを設定
	/// @param isResult 結果表示フラグ
	void SetResult(bool isResult) { isResult_ = isResult; }

	/// @brief ポストエフェクトを取得
	/// @return ポストエフェクト
	PostEffect GetPostEffect() const { return postEffect_; }

private:
	using ConstantBuffers = std::array<std::unique_ptr<ConstantBuffer>, static_cast<size_t>(ConstantBufferType::kCountOfConstantBufferType)>;
	using StructuredBuffers = std::array<std::unique_ptr<Resource>, static_cast<size_t>(StructuredBufferType::kCountOfStructuredBufferType)>;
	static inline constexpr uint32_t kMaxObject = 1048576;				// 最大オブジェクト数
	static inline constexpr uint32_t kMaxLine = 65536;					// 最大ライン数
	static inline constexpr uint32_t kMaxPointLight = 32;				// 最大点光源数
	static inline constexpr uint32_t kMaxSpotLight = 32;				// 最大スポットライト数
	static inline constexpr uint32_t kMaxAABB = 1048576;				// 最大AABB数
	static inline constexpr uint32_t kMaxFootprint = 64;				// 最大フットプリント数
	static inline constexpr uint32_t kMaxCommandPerQueue = 1024;		// コマンドキューあたりの最大コマンド数
	Registry *registry_ = nullptr;										// レジストリ
	ConstantBuffers constantBuffers_;									// 定数バッファリスト
	StructuredBuffers structuredBuffers_;								// 構造化バッファリスト
	std::unique_ptr<Resource> sceneRenderTexture_ = nullptr;			// シーンのレンダーテクスチャ
	std::unique_ptr<Resource> gameRenderTexture_ = nullptr;				// ゲームのレンダーテクスチャ
	std::unique_ptr<Resource> postEffectRenderTexture_ = nullptr;		// ポストエフェクトのレンダーテクスチャ
	std::unique_ptr<Resource> hiZTexture_ = nullptr;					// Hi-Zテクスチャ
	std::unique_ptr<Resource> commandBufferUpload_ = nullptr;			// コマンドバッファアップロード用
	std::unique_ptr<Resource> processedCommandBuffer_ = nullptr;		// カリング済みコマンドバッファ
	std::unique_ptr<Resource> commandCounterBuffer_ = nullptr;			// コマンドカウンターバッファ
	std::unique_ptr<Resource> footprintMapBuffer_ = nullptr;			// フットプリントマップバッファ
	std::unique_ptr<Resource> footprintMapReadbackBuffer_ = nullptr;	// フットプリントマップ読み戻しバッファ
	QueueOffsets queueOffsets_;											// キューオフセットリスト
	GrayscaleColor grayscaleColor_;										// グレースケールカラー
	VignetteParam vignetteParam_;										// ビネットパラメータ
	BoxFilterParam boxFilterParam_;										// ボックスフィルターパラメータ
	GaussianFilterParam gaussianFilterParam_;							// ガウシアンフィルターパラメータ
	PrewittFilterParam luminancePrewittFilterParam_;					// 輝度用プレウィットフィルターパラメータ
	PrewittFilterParam depthPrewittFilterParam_;						// 深度用プレウィットフィルターパラメータ
	DepthMaterial depthMaterial_;										// 深度マテリアル
	RadialBlurParam radialBlurParam_;									// ラジアルブラーのパラメータ
	DissolveParam dissolveParam_;										// ディゾルブのパラメータ
	NoiseParam noiseParam_;												// ノイズのパラメータ
	FootprintForGPU *footprintData_ = nullptr;							// フットプリントデータ
	Int4 *colorData_ = nullptr;											// 色データ
	Rendering::Line *lineData_ = nullptr;								// ラインデータ
	PointLight *pointLightData_ = nullptr;								// 点光源データ
	SpotLight *spotLightData_ = nullptr;								// スポットライトデータ
	uint32_t mipLevels_ = 0;											// ミップレベル数
	uint32_t sceneRenderTextureRTVHandle_ = 0;							// シーンのレンダーテクスチャRTVハンドル
	uint32_t sceneRenderTextureSRVHandle_ = 0;							// シーンのレンダーテクスチャSRVハンドル
	uint32_t gameRenderTextureRTVHandle_ = 0;							// ゲームのレンダーテクスチャRTVハンドル
	uint32_t gameRenderTextureSRVHandle_ = 0;							// ゲームのレンダーテクスチャSRVハンドル
	uint32_t postEffectRenderTextureRTVHandle_ = 0;						// ポストエフェクトのレンダーテクスチャRTVハンドル
	uint32_t postEffectRenderTextureSRVHandle_ = 0;						// ポストエフェクトのレンダーテクスチャSRVハンドル
	uint32_t previousMainCameraDepthStencilTextureSRVHandle_ = 0;		// 前フレームのメインカメラ用深度ステンシルテクスチャSRVハンドル
	uint32_t mainCameraDepthStencilTextureSRVHandle_ = 0;				// メインカメラ用深度ステンシルテクスチャSRVハンドル
	uint32_t hiZTextureSRVHandle_ = 0;									// Hi-ZテクスチャSRVハンドル
	uint32_t hiZTextureUAVHandle_ = 0;									// Hi-ZテクスチャUAVハンドル
	std::vector<uint32_t> hiZMipMapReadHandles_;						// Hi-Zミップマップ読み取りハンドル
	std::vector<uint32_t> hiZMipMapWriteHandles_;						// Hi-Zミップマップ書き込みハンドル
	uint32_t processedCommandHandle_ = 0;								// カリング済みコマンドハンドル
	uint32_t commandCounterHandle_ = 0;									// コマンドカウンターハンドル
	uint32_t footprintHandle_ = 0;										// フットプリントハンドル
	uint32_t footprintMapHandle_ = 0;									// フットプリントマップハンドル
	uint32_t lineHandle_ = 0;											// ラインハンドル
	uint32_t pointLightHandle_ = 0;										// 点光源ハンドル
	uint32_t spotLightHandle_ = 0;										// スポットライトハンドル
	uint32_t cullingObjectHandle_ = 0;									// カリングオブジェクトハンドル
	uint32_t cullingMeshHandle_ = 0;									// カリングメッシュハンドル
	uint32_t meshLODHandle_ = 0;										// メッシュLODハンドル
	PostEffect postEffect_ = PostEffect::kNone;							// ポストエフェクト
	bool isCulling_ = false;											// カリング有効フラグ
	bool isResult_ = false;												// 結果表示フラグ
	CPUTimer cpuTimer_;													// CPUタイマー

	/// @brief 平行光源の転送
	void TransferDirectionalLight();

	/// @brief 点光源の転送
	void TransferPointLight();

	/// @brief スポットライトの転送
	void TransferSpotLight();

	/// @brief モデルカメラの転送
	void TransferCamera();

	/// @brief ワールド変換の転送
	void TransferTransform();

	/// @brief マテリアルの転送
	void TransferMaterial();

	/// @brief グレースケールカラーの転送
	void TransferGrayscaleColor();

	/// @brief ビネットパラメータの転送
	void TransferVignetteParam();

	/// @brief ボックスフィルターパラメータの転送
	void TransferBoxFilterParam();

	/// @brief ガウシアンフィルターパラメータの転送
	void TransferGaussianFilterParam();

	/// @brief 輝度ベースのアウトラインデータの転送
	void TransferLuminanceBasedOutlineData();

	/// @brief 深度ベースのアウトラインデータの転送
	void TransferDepthBasedOutlineData();

	/// @brief ラジアルブラーのパラメータの転送
	void TransferRadialBlurParam();

	/// @brief ディゾルブのパラメータの転送
	void TransferDissolveParam();

	/// @brief ノイズのパラメータの転送
	void TransferNoiseParam();

	/// @brief フットプリントの転送
	void TransferFootprint();

	/// @brief フットプリントマップの転送
	void TransferFootprintMap();
};