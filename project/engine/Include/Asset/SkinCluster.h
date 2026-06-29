#pragma once
#include "Matrix4x4.h"
#include "Resource.h"
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
	std::vector<Matrix4x4> inverseBindPoseMatrices;	// 逆バインドポーズ行列リスト
	std::unique_ptr<Resource> influenceResource;	// 頂点影響リソース
	D3D12_VERTEX_BUFFER_VIEW influenceBufferView;	// 頂点影響バッファビュー
	std::span<VertexInfluence> mappedInfluence;		// マッピングされた頂点影響データ
	std::unique_ptr<Resource> paletteResource;		// パレットリソース
	std::span<WellForGPU> mappedPalette;			// マッピングされたGPU用ウェルデータ
	uint32_t paletteSRVHandle;						// パレットSRVハンドル
};

class Device;
class Registry;

/// @brief スキンクラスターマネージャー
class SkinClusterManager final {
public:
	/// @brief コンストラクタ
	/// @param device デバイス
	SkinClusterManager(Device *device) : device_(device) {}

	/// @brief スキンクラスターの作成
	/// @param modelData モデルデータ
	/// @return スキンクラスターインデックス
	uint32_t CreateSkinCluster(const ModelData &modelData);

	/// @brief 更新
	void Update();

	/// @brief レジストリの設定
	/// @param registry レジストリ
	void SetRegistry(Registry *registry) { registry_ = registry; }

	/// @brief 頂点影響バッファビューの取得
	/// @param skinClusterIndex スキンクラスターインデックス
	/// @return 頂点影響バッファビュー
	D3D12_VERTEX_BUFFER_VIEW GetInfluenceBufferView(uint32_t skinClusterIndex) const;

	/// @brief パレットSRVハンドルの取得
	/// @param skinClusterIndex スキンクラスターインデックス
	/// @return パレットSRVハンドル
	uint32_t GetPaletteSRVHandle(uint32_t skinClusterIndex) const;

private:
	Device *device_ = nullptr;	// デバイス
	Registry *registry_ = nullptr;	// レジストリ
	std::vector<std::unique_ptr<SkinCluster>> skinClusters_;	// スキンクラスターリスト
};