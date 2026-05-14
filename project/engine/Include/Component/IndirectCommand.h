#pragma once
#include "BlendMode.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include <d3d12.h>
#include <vector>

/// @brief カリングオブジェクトデータ(GPU)
struct CullingObjectData final {
	Matrix4x4 worldMatrix;	// ワールド行列
	BlendMode blendMode;	// ブレンドモード
};

/// @brief AABB(GPU)
struct AABBForGPU final {
	Vector4 min;	// 最小点
	Vector4 max;	// 最大点
};

/// @brief カリングデータ(GPU)
struct CullingMeshData final {
	AABBForGPU aabb;			// AABB
	uint32_t objectHandle = 0;	// オブジェクトハンドル
	uint32_t lodOffset = 0;		// LODオフセット
	uint32_t lodCount = 0;		// LOD数
	uint32_t useCulling = 0;	// カリングを使用するか
};

/// @brief テクスチャデータ
struct TextureData final {
	uint32_t textureHandle = 0;	// テクスチャハンドル
	bool enableMipMaps = true;	// ミップマップ有効フラグ
};

/// @brief 間接コマンド
#pragma pack(push, 1)
struct IndirectCommand final {
	D3D12_GPU_VIRTUAL_ADDRESS cbv[2];					// CBV
	TextureData textureData;							// テクスチャデータ
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;			// 頂点バッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView;			// インデックスバッファビュー
	D3D12_DRAW_INDEXED_ARGUMENTS drawIndexedArguments;	// 描画コマンド引数
};
#pragma pack(pop)

/// @brief メッシュLOD
#pragma pack(push, 1)
struct MeshLOD final {
	IndirectCommand indirectCommand;	// 間接コマンド
	float error = 0.0f;					// LODエラー
};
#pragma pack(pop)

/// @brief 間接コマンドハンドル
struct IndirectCommandHandle final {
	std::vector<uint32_t> handles;	// ハンドル
};

class Registry;
class World;
class MeshManager;

/// @brief 間接コマンドマネージャ
class IndirectCommandManager {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param world ワールド
	/// @param meshManager メッシュマネージャー
	IndirectCommandManager(Registry *registry, World *world, MeshManager *meshManager);

	/// @brief 間接コマンドの追加
	/// @param entity エンティティ
	IndirectCommandHandle AddIndirectCommand(uint32_t entity);

	/// @brief 間接コマンドの削除
	/// @param entity エンティティ
	void RemoveIndirectCommand(uint32_t entity);

	/// @brief ブレンドモードデータの設定
	/// @param entity エンティティ
	void SetBlendModeData(uint32_t entity);

	/// @brief カリングデータの更新
	void UpdateCullingData();

	/// @brief 間接コマンド数のデバッグ表示
	void Debug() const;

	/// @brief メッシュカウンターの取得
	/// @return メッシュカウンター
	uint32_t GetMeshCounter() const { return meshCounter_; }

private:
	Registry *registry_ = nullptr;						// レジストリ
	World *world_ = nullptr;							// ワールド
	MeshManager *meshManager_ = nullptr;				// メッシュマネージャー
	CullingObjectData *cullingObjectData_ = nullptr;	// カリングオブジェクトデータ
	CullingMeshData *cullingMeshData_ = nullptr;		// カリングメッシュデータ
	MeshLOD *meshLODData_ = nullptr;					// メッシュLODデータ
	std::vector<uint32_t> entities_;					// インデックスからエンティティへのマッピング
	uint32_t meshCounter_ = 0;							// メッシュカウンター
};