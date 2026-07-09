#pragma once
#include "Matrix4x4.h"
#include "Resource.h"
#include "VertexData.h"
#include <vector>
#include <array>
#include <span>

constexpr uint32_t kNumMaxInfluence = 4;	// 最大影響数

/// @brief 頂点影響データ
struct VertexInfluence final {
	std::array<float, kNumMaxInfluence> weights;	// ウェイトリスト
	std::array<uint32_t, kNumMaxInfluence> jointIndices;	// ジョイントインデックスリスト
};

/// @brief GPU用ウェルデータ
struct WellForGPU final {
	Matrix4x4 skeletonSpaceMatrix;	// スケルトンスペース行列（位置用）
	Matrix4x4 skeletonSpaceInverseTransposeMatrix;	// スケルトンスペース逆転置行列（法線用）
};

struct Skeleton;
struct ModelData;

/// @brief スキンクラスター
struct SkinCluster final {
	std::vector<Matrix4x4> inverseBindPoseMatrices;		// 逆バインドポーズ行列リスト
	std::unique_ptr<Resource> influenceResource;		// 頂点影響リソース
	std::span<VertexInfluence> mappedInfluence;			// マッピングされた頂点影響データ
	std::unique_ptr<Resource> paletteResource;			// パレットリソース
	std::span<WellForGPU> mappedPalette;				// マッピングされたGPU用ウェルデータ
	std::unique_ptr<Resource> inputVertexResource;		// 入力頂点リソース
	std::span<VertexData> mappedVertex;					// マッピングされた頂点データ
	std::unique_ptr<Resource> outputVertexResource;		// 出力頂点リソース
	D3D12_VERTEX_BUFFER_VIEW outputVertexBufferView;	// 出力頂点バッファビュー
	uint32_t paletteSRVHandle;							// パレットSRVハンドル
	uint32_t influenceSRVHandle;						// 頂点影響SRVハンドル
	uint32_t vertexSRVHandle;							// 頂点SRVハンドル
	uint32_t vertexUAVHandle;							// 頂点UAVハンドル
};

class Device;
class Registry;

/// @brief スキンクラスターマネージャー
class SkinClusterManager final {
public:
	/// @brief コンストラクタ
	/// @param device デバイス
	/// @param logStream ログ出力ストリーム
	SkinClusterManager(Device *device, std::ofstream *logStream) : device_(device), logStream_(logStream) {}

	/// @brief スキンクラスターの作成
	/// @param modelData モデルデータ
	/// @return スキンクラスターインデックス
	uint32_t CreateSkinCluster(const ModelData &modelData);

	/// @brief 更新
	void Update();

	/// @brief レジストリの設定
	/// @param registry レジストリ
	void SetRegistry(Registry *registry) { registry_ = registry; }

	/// @brief パレットSRVハンドルの取得
	/// @param skinClusterIndex スキンクラスターインデックス
	/// @return パレットSRVハンドル
	uint32_t GetPaletteSRVHandle(uint32_t skinClusterIndex) const;

	/// @brief 頂点影響SRVハンドルの取得
	/// @param skinClusterIndex スキンクラスターインデックス
	/// @return 頂点影響SRVハンドル
	uint32_t GetInfluenceSRVHandle(uint32_t skinClusterIndex) const;

	/// @brief 頂点SRVハンドルの取得
	/// @param skinClusterIndex スキンクラスターインデックス
	/// @return 頂点SRVハンドル
	uint32_t GetVertexSRVHandle(uint32_t skinClusterIndex) const;

	/// @brief 頂点UAVハンドルの取得
	/// @param skinClusterIndex スキンクラスターインデックス
	/// @return 頂点UAVハンドル
	uint32_t GetVertexUAVHandle(uint32_t skinClusterIndex) const;

	/// @brief 出力頂点バッファビューの取得
	/// @param skinClusterIndex スキンクラスターインデックス
	/// @return 出力頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW GetOutputVertexBufferView(uint32_t skinClusterIndex) const;

private:
	Device *device_ = nullptr;	// デバイス
	std::ofstream *logStream_ = nullptr;	// ログ出力用のストリーム
	Registry *registry_ = nullptr;	// レジストリ
	std::vector<std::unique_ptr<SkinCluster>> skinClusters_;	// スキンクラスターリスト
};