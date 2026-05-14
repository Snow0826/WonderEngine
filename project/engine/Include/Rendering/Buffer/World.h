#pragma once
#include "BlendMode.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <array>
#include <vector>
#include <memory>
#include <d3d12.h>

/// @brief ライトの数
struct LightCount final {
	uint32_t pointLightCount = 0;	// 点光源の数
	uint32_t spotLightCount = 0;	// スポットライトの数
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
	kCamera,					// カメラ
	kDirectionalLight,			// 平行光源
	kFrustum,					// 視錐台
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
	static const UINT kCommandSizePerFrame;			// フレームあたりのコマンドサイズ
	static const UINT kCommandBufferCounterOffset;	// コマンドバッファカウンターオフセット

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

	/// @brief 定数バッファを取得
	/// @param type 定数バッファの種類
	ConstantBuffer *GetConstantBuffer(ConstantBufferType type) {
		return constantBuffers_[static_cast<size_t>(type)].get();
	}

	/// @brief 構造化バッファを取得
	/// @param type 構造化バッファの種類
	/// @return 構造化バッファ
	Resource *GetStructuredBuffer(StructuredBufferType type) {
		return structuredBuffers_[static_cast<size_t>(type)].get();
	}

	/// @brief Hi-Zテクスチャを取得
	/// @return Hi-Zテクスチャ
	Resource *GetHiZTexture() {
		return hiZTexture_.get();
	}

	/// @brief ブレンド別処理済みコマンドバッファを取得
	/// @param blendMode ブレンドモード
	/// @return ブレンド別処理済みコマンドバッファ
	Resource *GetBlendProcessedCommandBuffer(BlendMode blendMode) {
		return blendProcessedCommandBuffers_[static_cast<size_t>(blendMode)].get();
	}

	/// @brief コマンドバッファアップロードを取得
	/// @return コマンドバッファアップロード
	Resource *GetCommandBufferUpload() {
		return commandBufferUpload_.get();
	}

	/// @brief 処理済みコマンドバッファカウンターリセットを取得
	/// @return 処理済みコマンドバッファカウンターリセット
	Resource *GetProcessedCommandBufferCounterReset() {
		return processedCommandBufferCounterReset_.get();
	}

	/// @brief フットプリントマップバッファを取得
	/// @return フットプリントマップバッファ
	Resource *GetFootprintMapBuffer() {
		return footprintMapBuffer_.get();
	}

	/// @brief フットプリントマップデータを取得
	/// @return フットプリントマップデータ
	Int4 GetColor() const { return colorData_[0]; }

	/// @brief ミップレベル数を取得
	/// @return ミップレベル数
	uint32_t GetMipLevels() const { return mipLevels_; }

	/// @brief 深度ステンシルコピー元ハンドルを取得
	/// @return 深度ステンシルコピー元ハンドル
	uint32_t GetDepthStencilCopySourceHandle() const { return depthStencilCopySourceHandle_; }

	/// @brief 深度ステンシルコピー先ハンドルを取得
	/// @return 深度ステンシルコピー先ハンドル
	uint32_t GetDepthStencilCopyDestHandle() const { return depthStencilCopyDestHandle_; }

	/// @brief Hi-Zミップマップ読み取りハンドルを取得
	/// @param index インデックス
	/// @return Hi-Zミップマップ読み取りハンドル
	uint32_t GetHiZMipMapReadHandle(uint32_t index) const { return hiZMipMapReadHandles_[index]; }

	/// @brief Hi-Zミップマップ書き込みハンドルを取得
	/// @param index インデックス
	/// @return Hi-Zミップマップ書き込みハンドル
	uint32_t GetHiZMipMapWriteHandle(uint32_t index) const { return hiZMipMapWriteHandles_[index]; }

	/// @brief Hi-Zテクスチャハンドルを取得
	/// @return Hi-Zテクスチャハンドル
	uint32_t GetHiZTextureHandle() const { return hiZTextureHandle_; }

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

	/// @brief ブレンド別処理済み間接コマンドハンドルを取得
	/// @param blendMode ブレンドモード
	/// @return ブレンド別処理済み間接コマンドハンドル
	uint32_t GetBlendProcessedIndirectCommandHandle(BlendMode blendMode) const {
		return blendProcessedIndirectCommandHandle_[static_cast<size_t>(blendMode)];
	}

	/// @brief 結果表示フラグを取得
	/// @return 結果表示フラグ
	bool IsResult() const { return isResult_; }

	/// @brief 結果表示フラグを設定
	/// @param isResult 結果表示フラグ
	void SetResult(bool isResult) { isResult_ = isResult; }

private:
	using ConstantBuffers = std::array<std::unique_ptr<ConstantBuffer>, static_cast<size_t>(ConstantBufferType::kCountOfConstantBufferType)>;
	using StructuredBuffers = std::array<std::unique_ptr<Resource>, static_cast<size_t>(StructuredBufferType::kCountOfStructuredBufferType)>;
	using BlendBuffers = std::array<std::unique_ptr<Resource>, static_cast<size_t>(BlendMode::kCountOfBlendMode)>;
	using BlendHandle = std::array<uint32_t, static_cast<size_t>(BlendMode::kCountOfBlendMode)>;
	static inline constexpr uint32_t kMaxObject = 1048576;						// 最大オブジェクト数
	static inline constexpr uint32_t kMaxLine = 65536;							// 最大ライン数
	static inline constexpr uint32_t kMaxPointLight = 32;						// 最大点光源数
	static inline constexpr uint32_t kMaxSpotLight = 32;						// 最大スポットライト数
	static inline constexpr uint32_t kMaxAABB = 1048576;						// 最大AABB数
	static inline constexpr uint32_t kMaxFootprint = 64;						// 最大フットプリント数
	Registry *registry_ = nullptr;												// レジストリ
	ConstantBuffers constantBuffers_;											// 定数バッファリスト
	StructuredBuffers structuredBuffers_;										// 構造化バッファリスト
	BlendBuffers blendProcessedCommandBuffers_;									// ブレンド別処理済みコマンドバッファリスト
	std::unique_ptr<Resource> hiZTexture_ = nullptr;							// Hi-Zテクスチャ
	std::unique_ptr<Resource> commandBufferUpload_ = nullptr;					// コマンドバッファアップロード用
	std::unique_ptr<Resource> processedCommandBufferCounterReset_ = nullptr;	// 処理済みコマンドバッファカウンターリセット用
	std::unique_ptr<Resource> footprintMapBuffer_ = nullptr;					// フットプリントマップバッファ
	std::unique_ptr<Resource> footprintMapReadbackBuffer_ = nullptr;			// フットプリントマップ読み戻しバッファ
	FootprintForGPU *footprintData_ = nullptr;									// フットプリントデータ
	Int4 *colorData_ = nullptr;													// 色データ
	Rendering::Line *lineData_ = nullptr;										// ラインデータ
	PointLight *pointLightData_ = nullptr;										// 点光源データ
	SpotLight *spotLightData_ = nullptr;										// スポットライトデータ
	uint32_t mipLevels_ = 0;													// ミップレベル数
	std::vector<uint32_t> hiZMipMapReadHandles_;								// Hi-Zミップマップ読み取りハンドル
	std::vector<uint32_t> hiZMipMapWriteHandles_;								// Hi-Zミップマップ書き込みハンドル
	uint32_t hiZTextureHandle_ = 0;												// Hi-Zテクスチャハンドル
	uint32_t depthStencilCopySourceHandle_ = 0;									// 深度ステンシルコピー元ハンドル
	uint32_t depthStencilCopyDestHandle_ = 0;									// 深度ステンシルコピー先ハンドル
	uint32_t footprintHandle_ = 0;												// フットプリントハンドル
	uint32_t footprintMapHandle_ = 0;											// フットプリントマップハンドル
	uint32_t lineHandle_ = 0;													// ラインハンドル
	uint32_t pointLightHandle_ = 0;												// 点光源ハンドル
	uint32_t spotLightHandle_ = 0;												// スポットライトハンドル
	uint32_t cullingObjectHandle_ = 0;											// カリングオブジェクトハンドル
	uint32_t cullingMeshHandle_ = 0;											// カリングメッシュハンドル
	uint32_t meshLODHandle_ = 0;												// メッシュLODハンドル
	BlendHandle blendProcessedIndirectCommandHandle_;							// ブレンド別処理済み間接コマンドハンドル
	bool isCulling_ = false;													// カリング有効フラグ
	bool isResult_ = false;														// 結果表示フラグ

	/// @brief UAVカウンター用にアライメントを調整する
	/// @param bufferSize バッファサイズ
	/// @return アライメント調整後のバッファサイズ
	static inline UINT AlignForUavCounter(UINT bufferSize) {
		const UINT alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
		return (bufferSize + (alignment - 1)) & ~(alignment - 1);
	}

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

	/// @brief フットプリントの転送
	void TransferFootprint();

	/// @brief フットプリントマップの転送
	void TransferFootprintMap();
};