#pragma once
#include "VertexData.h"
#include "Collision.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <d3d12.h>

/// @brief メッシュLODデータ
struct MeshLODData final {
	std::vector<VertexData> vertices;	// 頂点リスト
	std::vector<uint32_t> indices;		// インデックスリスト
	float error = 0.0f;					// LODエラー
	std::string meshName;				// メッシュ名
};

/// @brief メッシュデータ
struct MeshData final {
	std::vector<MeshLODData> lods;	// LODデータリスト
	uint32_t materialIndex = 0;		// マテリアルインデックス
};

class VertexBuffer;
class IndexBuffer;

/// @brief メッシュ
struct Mesh final {
	std::unique_ptr<VertexBuffer> vertexBuffer = nullptr;	// 頂点バッファ
	std::unique_ptr<IndexBuffer> indexBuffer = nullptr;		// インデックスバッファ
};

class Device;
struct MeshLOD;

/// @brief メッシュマネージャー
class MeshManager final {
public:
	/// @brief コンストラクタ
	/// @param device デバイス
	/// @param logStream ログ出力ストリーム
	MeshManager(Device *device, std::ofstream *logStream);

	/// @brief デストラクタ
	~MeshManager();

	/// @brief メッシュの生成
	/// @param meshName メッシュ名
	/// @param meshLODData メッシュLODデータ
	void CreateMesh(const std::string &meshName, const MeshLODData &meshLODData);

	/// @brief スプライトの生成
	/// @param meshName メッシュ名
	void CreateSprite(const std::string &meshName);

	/// @brief 平面の生成
	/// @param meshName メッシュ名
	void CreatePlane(const std::string &meshName);

	/// @brief 立方体の生成
	/// @param meshName メッシュ名
	void CreateBox(const std::string &meshName);

	/// @brief 円環の生成
	/// @param meshName メッシュ名
	/// @param divide 分割数
	/// @param outerRadius 外半径
	/// @param innerRadius 内半径
	void CreateRing(const std::string &meshName, uint32_t divide, float outerRadius, float innerRadius);

	/// @brief 円柱の生成
	/// @param meshName メッシュ名
	/// @param divide 分割数
	/// @param topRadius 上面の半径
	/// @param bottomRadius 下面の半径
	/// @param height 高さ
	/// @param cap キャップの有無
	void CreateCylinder(const std::string &meshName, uint32_t divide, float topRadius, float bottomRadius, float height, bool cap = true);

	/// @brief 描画
	/// @param meshName メッシュ名
	/// @param instanceCount インスタンス数
	void Draw(const std::string &meshName, uint32_t instanceCount) const;

	/// @brief 頂点バッファビューの取得
	/// @param meshName メッシュ名
	/// @return 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(const std::string &meshName) const;

	/// @brief インデックスバッファビューの取得
	/// @param meshName メッシュ名
	/// @return インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(const std::string &meshName) const;

	/// @brief 頂点データの取得
	/// @param meshName メッシュ名
	/// @return 頂点データ
	VertexData *GetVertexData(const std::string &meshName) const;

	/// @brief インデックス数の取得
	/// @param meshName メッシュ名
	/// @return インデックス数
	UINT GetIndexCount(const std::string &meshName) const;

	/// @brief メッシュLODデータの再インデックス化
	/// @param meshLODData メッシュLODデータ
	/// @return 再インデックス化されたメッシュLODデータ
	static MeshLODData ReIndexMeshLODData(const MeshLODData &meshLODData);

	/// @brief ローカル球の生成
	/// @param meshName メッシュ名
	/// @return ローカル球
	Collision::Sphere CreateLocalSphere(const std::string &meshName);

	/// @brief ローカルAABBの生成
	/// @param meshName メッシュ名
	/// @return ローカルAABB
	Collision::AABB CreateLocalAABB(const std::string &meshName);

	/// @brief ローカルOBBの生成
	/// @param meshName メッシュ名
	/// @return ローカルOBB
	Collision::OBB CreateLocalOBB(const std::string &meshName);

private:
	Device *device_ = nullptr;	// デバイス
	std::ofstream *logStream_ = nullptr;	// ログ出力用のストリーム
	std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes_;	// メッシュリスト
};