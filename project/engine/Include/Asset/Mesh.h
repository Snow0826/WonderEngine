#pragma once
#include "VertexData.h"
#include "Collision.h"
#include <memory>
#include <vector>
#include <string>
#include <d3d12.h>

class Device;
class VertexBuffer;
class IndexBuffer;

/// @brief メッシュLODデータ
struct MeshLODData final {
	std::vector<VertexData> vertices;	// 頂点リスト
	std::vector<uint32_t> indices;		// インデックスリスト
	float error = 0.0f;					// LODエラー
	uint32_t handle = 0;				// メッシュハンドル
};

/// @brief メッシュデータ
struct MeshData final {
	std::vector<MeshLODData> lods;	// LODデータリスト
	uint32_t materialIndex = 0;		// マテリアルインデックス
	Collision::Sphere sphere;		// 球
	Collision::AABB aabb;			// AABB
	Collision::OBB obb;				// OBB
};

/// @brief メッシュ
struct Mesh final {
	std::unique_ptr<VertexBuffer> vertexBuffer = nullptr;	// 頂点バッファ
	std::unique_ptr<IndexBuffer> indexBuffer = nullptr;		// インデックスバッファ
};

/// @brief メッシュマネージャー
class MeshManager final {
public:
	/// @brief コンストラクタ
	/// @param device デバイス
	MeshManager(Device *device);

	/// @brief デストラクタ
	~MeshManager();

	/// @brief メッシュの生成
	/// @param meshLODData メッシュLODデータ
	/// @return メッシュハンドル
	uint32_t CreateMesh(const MeshLODData &meshLODData);

	/// @brief スプライトの生成
	/// @return メッシュハンドル
	uint32_t CreateSprite();

	/// @brief 平面の生成
	/// @return メッシュハンドル
	uint32_t CreatePlane();

	/// @brief 立方体の生成
	/// @return メッシュハンドル
	uint32_t CreateBox();
	
	/// @brief 円環の生成
	/// @param divide 分割数
	/// @param outerRadius 外半径
	/// @param innerRadius 内半径
	/// @return メッシュハンドル
	uint32_t CreateRing(uint32_t divide, float outerRadius, float innerRadius);

	/// @brief 円柱の生成
	/// @param divide 分割数
	/// @param topRadius 上面の半径
	/// @param bottomRadius 下面の半径
	/// @param height 高さ
	/// @return メッシュハンドル
	uint32_t CreateCylinder(uint32_t divide, float topRadius, float bottomRadius, float height);

	/// @brief 描画
	/// @param meshHandle メッシュハンドル
	/// @param instanceCount インスタンス数
	void Draw(uint32_t meshHandle, uint32_t instanceCount = 1) const;

	/// @brief 頂点バッファビューの取得
	/// @param meshHandle メッシュハンドル
	/// @return 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(uint32_t meshHandle) const;

	/// @brief インデックスバッファビューの取得
	/// @param meshHandle メッシュハンドル
	/// @return インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(uint32_t meshHandle) const;

	/// @brief 頂点データの取得
	/// @param meshHandle メッシュハンドル
	/// @return 頂点データ
	VertexData *GetVertexData(uint32_t meshHandle) const;

	/// @brief インデックス数の取得
	/// @param meshHandle メッシュハンドル
	/// @return インデックス数
	UINT GetIndexCount(uint32_t meshHandle) const;

	/// @brief メッシュLODデータの再インデックス化
	/// @param meshLODData メッシュLODデータ
	/// @return 再インデックス化されたメッシュLODデータ
	static MeshLODData ReIndexMeshLODData(const MeshLODData &meshLODData);

	/// @brief ローカル球の生成
	/// @param meshHandle メッシュハンドル
	/// @return ローカル球
	Collision::Sphere CreateLocalSphere(uint32_t meshHandle);

	/// @brief ローカルAABBの生成
	/// @param meshHandle メッシュハンドル
	/// @return ローカルAABB
	Collision::AABB CreateLocalAABB(uint32_t meshHandle);

	/// @brief ローカルOBBの生成
	/// @param meshHandle メッシュハンドル
	/// @return ローカルOBB
	Collision::OBB CreateLocalOBB(uint32_t meshHandle);

private:
	Device *device_ = nullptr;					// デバイス
	std::vector<std::unique_ptr<Mesh>> meshes_;	// メッシュリスト
};