#define NOMINMAX
#include "Mesh.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Collision.h"
#include "Resource.h"
#include <algorithm>

MeshManager::MeshManager(Device *device) : device_(device) {}
MeshManager::~MeshManager() = default;

uint32_t MeshManager::CreateMesh(const MeshLODData &meshLODData) {
	uint32_t meshHandle = static_cast<uint32_t>(meshes_.size());
	std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

	// 頂点バッファの生成と初期化
	mesh->vertexBuffer = std::make_unique<VertexBuffer>();
	mesh->vertexBuffer->Initialize(device_, meshLODData.vertices.size());
	std::memcpy(mesh->vertexBuffer->GetVertexData(), meshLODData.vertices.data(), sizeof(VertexData) * meshLODData.vertices.size());

	// インデックスバッファの生成と初期化
	mesh->indexBuffer = std::make_unique<IndexBuffer>();
	mesh->indexBuffer->Initialize(device_, meshLODData.indices.size());
	std::memcpy(mesh->indexBuffer->GetIndexData(), meshLODData.indices.data(), sizeof(uint32_t) * meshLODData.indices.size());
	meshes_.emplace_back(std::move(mesh));
	return meshHandle;
}

uint32_t MeshManager::CreateSprite() {
	uint32_t meshHandle = static_cast<uint32_t>(meshes_.size());
	std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

	// 頂点バッファの生成と初期化
	mesh->vertexBuffer = std::make_unique<VertexBuffer>();
	mesh->vertexBuffer->Initialize(device_, 4);
	VertexData *vertexData = mesh->vertexBuffer->GetVertexData();

	// 左下
	vertexData[0].position = { 0.0f, 1.0f, 0.0f, 1.0f };	// 頂点座標
	vertexData[0].texcoord = { 0.0f, 1.0f };				// テクスチャ座標
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };			// 法線

	// 左上
	vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };	// 頂点座標
	vertexData[1].texcoord = { 0.0f, 0.0f };				// テクスチャ座標
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };			// 法線

	// 右下
	vertexData[2].position = { 1.0f, 1.0f, 0.0f, 1.0f };	// 頂点座標
	vertexData[2].texcoord = { 1.0f, 1.0f };				// テクスチャ座標
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };			// 法線

	// 右上
	vertexData[3].position = { 1.0f, 0.0f, 0.0f, 1.0f };	// 頂点座標
	vertexData[3].texcoord = { 1.0f, 0.0f };				// テクスチャ座標
	vertexData[3].normal = { 0.0f, 0.0f, -1.0f };			// 法線

	// インデックスバッファの生成と初期化
	mesh->indexBuffer = std::make_unique<IndexBuffer>();
	mesh->indexBuffer->Initialize(device_, 6);
	uint32_t *indexData = mesh->indexBuffer->GetIndexData();
	indexData[0] = 0;	// 左下
	indexData[1] = 1;	// 左上
	indexData[2] = 2;	// 右下
	indexData[3] = 1;	// 左上
	indexData[4] = 3;	// 右上
	indexData[5] = 2;	// 右下
	meshes_.emplace_back(std::move(mesh));
	return meshHandle;
}

uint32_t MeshManager::CreatePlane() {
	uint32_t meshHandle = static_cast<uint32_t>(meshes_.size());
	std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

	// 頂点バッファの生成と初期化
	mesh->vertexBuffer = std::make_unique<VertexBuffer>();
	mesh->vertexBuffer->Initialize(device_, 4);
	VertexData *vertexData = mesh->vertexBuffer->GetVertexData();

	// 左下
	vertexData[0].position = { -1.0f, -1.0f, 0.0f, 1.0f };	// 頂点座標
	vertexData[0].texcoord = { 0.0f, 1.0f };				// テクスチャ座標
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };			// 法線

	// 左上
	vertexData[1].position = { -1.0f, 1.0f, 0.0f, 1.0f };	// 頂点座標
	vertexData[1].texcoord = { 0.0f, 0.0f };				// テクスチャ座標
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };			// 法線

	// 右下
	vertexData[2].position = { 1.0f, -1.0f, 0.0f, 1.0f };	// 頂点座標
	vertexData[2].texcoord = { 1.0f, 1.0f };				// テクスチャ座標
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };			// 法線

	// 右上
	vertexData[3].position = { 1.0f, 1.0f, 0.0f, 1.0f };	// 頂点座標
	vertexData[3].texcoord = { 1.0f, 0.0f };				// テクスチャ座標
	vertexData[3].normal = { 0.0f, 0.0f, -1.0f };			// 法線

	// インデックスバッファの生成と初期化
	mesh->indexBuffer = std::make_unique<IndexBuffer>();
	mesh->indexBuffer->Initialize(device_, 6);
	uint32_t *indexData = mesh->indexBuffer->GetIndexData();
	indexData[0] = 0;	// 左下
	indexData[1] = 1;	// 左上
	indexData[2] = 2;	// 右下
	indexData[3] = 1;	// 左上
	indexData[4] = 3;	// 右上
	indexData[5] = 2;	// 右下
	meshes_.emplace_back(std::move(mesh));
	return meshHandle;
}

uint32_t MeshManager::CreateBox() {
	uint32_t meshHandle = static_cast<uint32_t>(meshes_.size());
	std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

	// 頂点バッファの生成と初期化
	mesh->vertexBuffer = std::make_unique<VertexBuffer>();
	mesh->vertexBuffer->Initialize(device_, 24);
	VertexData *vertexData = mesh->vertexBuffer->GetVertexData();
	// 右面
	vertexData[0].position = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[1].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[2].position = { 1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[3].position = { 1.0f, -1.0f, -1.0f, 1.0f };

	// 左面
	vertexData[4].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[5].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[6].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	vertexData[7].position = { -1.0f, -1.0f, 1.0f, 1.0f };

	// 前面
	vertexData[8].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[9].position = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[10].position = { -1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[11].position = { 1.0f, -1.0f, 1.0f, 1.0f };

	// 後面
	vertexData[12].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[13].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[14].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	vertexData[15].position = { 1.0f, -1.0f, -1.0f, 1.0f };

	// 上面
	vertexData[16].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[17].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[18].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[19].position = { 1.0f, 1.0f, 1.0f, 1.0f };

	// 下面
	vertexData[20].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	vertexData[21].position = { -1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[22].position = { 1.0f, -1.0f, -1.0f, 1.0f };
	vertexData[23].position = { 1.0f, -1.0f, 1.0f, 1.0f };

	// インデックスバッファの生成と初期化
	mesh->indexBuffer = std::make_unique<IndexBuffer>();
	mesh->indexBuffer->Initialize(device_, 36);
	uint32_t *indexData = mesh->indexBuffer->GetIndexData();
	// 右面
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 2;
	indexData[4] = 1;
	indexData[5] = 3;

	// 左面
	indexData[6] = 4;
	indexData[7] = 5;
	indexData[8] = 6;
	indexData[9] = 6;
	indexData[10] = 5;
	indexData[11] = 7;

	// 前面
	indexData[12] = 8;
	indexData[13] = 9;
	indexData[14] = 10;
	indexData[15] = 10;
	indexData[16] = 9;
	indexData[17] = 11;

	// 後面
	indexData[18] = 12;
	indexData[19] = 13;
	indexData[20] = 14;
	indexData[21] = 14;
	indexData[22] = 13;
	indexData[23] = 15;

	// 上面
	indexData[24] = 16;
	indexData[25] = 17;
	indexData[26] = 18;
	indexData[27] = 18;
	indexData[28] = 17;
	indexData[29] = 19;

	// 下面
	indexData[30] = 20;
	indexData[31] = 21;
	indexData[32] = 22;
	indexData[33] = 22;
	indexData[34] = 21;
	indexData[35] = 23;
	meshes_.emplace_back(std::move(mesh));
	return meshHandle;
}

void MeshManager::Draw(uint32_t meshHandle, uint32_t instanceCount) const {
	meshes_[meshHandle]->vertexBuffer->IASetVertexBuffers();				// VBVの設定
	meshes_[meshHandle]->indexBuffer->IASetIndexBuffer();					// IBVの設定
	meshes_[meshHandle]->indexBuffer->DrawIndexedInstanced(instanceCount);	// 描画
}

VertexData *MeshManager::GetVertexData(uint32_t meshHandle) const {
	return meshes_[meshHandle]->vertexBuffer->GetVertexData();
}

D3D12_VERTEX_BUFFER_VIEW MeshManager::GetVertexBufferView(uint32_t meshHandle) const {
	return meshes_[meshHandle]->vertexBuffer->GetVertexBufferView();
}

D3D12_INDEX_BUFFER_VIEW MeshManager::GetIndexBufferView(uint32_t meshHandle) const {
	return meshes_[meshHandle]->indexBuffer->GetIndexBufferView();
}

UINT MeshManager::GetIndexCount(uint32_t meshHandle) const {
	return meshes_[meshHandle]->indexBuffer->GetIndices();
}

MeshLODData MeshManager::ReIndexMeshLODData(const MeshLODData &meshLODData) {
	MeshLODData reIndexedMeshLODData;
	std::unordered_map<VertexData, uint32_t, VertexDataHash> vertexMap;
	vertexMap.reserve(meshLODData.vertices.size());
	for (uint32_t index : meshLODData.indices) {
		const VertexData &vertex = meshLODData.vertices[index];
		auto it = vertexMap.find(vertex);
		if (it != vertexMap.end()) {
			// 既に存在する頂点データの場合、既存のインデックスを使用
			reIndexedMeshLODData.indices.emplace_back(it->second);
		} else {
			// 新しい頂点データの場合、頂点リストに追加し、新しいインデックスを割り当て
			uint32_t newIndex = static_cast<uint32_t>(reIndexedMeshLODData.vertices.size());
			reIndexedMeshLODData.vertices.emplace_back(vertex);
			reIndexedMeshLODData.indices.emplace_back(newIndex);
			vertexMap.emplace(vertex, newIndex);
		}
	}
	return reIndexedMeshLODData;
}

Collision::Sphere MeshManager::CreateLocalSphere(const std::vector<VertexData> &vertices) {
	// 中心点の計算
	Vector3 center = { 0.0f, 0.0f, 0.0f };
	for (const VertexData &vertexData : vertices) {
		center.x += vertexData.position.x;
		center.y += vertexData.position.y;
		center.z += vertexData.position.z;
	}
	center /= static_cast<float>(vertices.size());

	// 半径の計算
	float radius = 0.0f;
	for (const VertexData &vertexData : vertices) {
		float distance = center.distanceFrom({ vertexData.position.x, vertexData.position.y, vertexData.position.z });
		radius = std::max(radius, distance);
	}

	// バウンディングスフィアの作成
	Collision::Sphere sphere = {
		.center = center,
		.radius = radius
	};
	return sphere;
}

Collision::AABB MeshManager::CreateLocalAABB(const std::vector<VertexData> &vertices) {
	Collision::AABB aabb = {
		.min = { vertices[0].position.x, vertices[0].position.y, vertices[0].position.z },
		.max = aabb.min
	};

	for (const VertexData &vertexData : vertices) {
		aabb.min.x = std::min(aabb.min.x, vertexData.position.x);
		aabb.min.y = std::min(aabb.min.y, vertexData.position.y);
		aabb.min.z = std::min(aabb.min.z, vertexData.position.z);
		aabb.max.x = std::max(aabb.max.x, vertexData.position.x);
		aabb.max.y = std::max(aabb.max.y, vertexData.position.y);
		aabb.max.z = std::max(aabb.max.z, vertexData.position.z);
	}
	return aabb;
}

Collision::OBB MeshManager::CreateLocalOBB(const std::vector<VertexData> &vertices) {
	// 中心点の計算
	Vector3 center = { 0.0f, 0.0f, 0.0f };
	for (const VertexData &vertexData : vertices) {
		center.x += vertexData.position.x;
		center.y += vertexData.position.y;
		center.z += vertexData.position.z;
	}
	center /= static_cast<float>(vertices.size());
	// 各軸方向の単位ベクトルの計算（仮にX軸、Y軸、Z軸とする）
	Vector3 orientations[3] = {
		{ 1.0f, 0.0f, 0.0f },	// X軸
		{ 0.0f, 1.0f, 0.0f },	// Y軸
		{ 0.0f, 0.0f, 1.0f }	// Z軸
	};

	// 各軸方向の半分の長さの計算
	Vector3 size = { 0.0f, 0.0f, 0.0f };
	for (const VertexData &vertexData : vertices) {
		Vector3 relativePos = {
			vertexData.position.x - center.x,
			vertexData.position.y - center.y,
			vertexData.position.z - center.z
		};
		size.x = std::max(size.x, std::abs(relativePos.dot(orientations[0])));
		size.y = std::max(size.y, std::abs(relativePos.dot(orientations[1])));
		size.z = std::max(size.z, std::abs(relativePos.dot(orientations[2])));
	}

	// バウンディングOBBの作成
	Collision::OBB obb = {
		.center = center,
		.orientations = { orientations[0], orientations[1], orientations[2] },
		.size = size
	};
	return obb;
}