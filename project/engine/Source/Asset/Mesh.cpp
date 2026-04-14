#define NOMINMAX
#include "Mesh.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Collision.h"
#include "Resource.h"
#include <algorithm>

MeshManager::MeshManager(Device *device) : device_(device) {}
MeshManager::~MeshManager() = default;

uint32_t MeshManager::CreateMesh(const MeshData &meshData) {
	uint32_t meshHandle = static_cast<uint32_t>(meshes_.size());
	std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

	// 頂点バッファの生成と初期化
	mesh->vertexBuffer = std::make_unique<VertexBuffer>();
	mesh->vertexBuffer->Initialize(device_, meshData.vertices.size());
	std::memcpy(mesh->vertexBuffer->GetVertexData(), meshData.vertices.data(), sizeof(VertexData) * meshData.vertices.size());

	// インデックスバッファの生成と初期化
	mesh->indexBuffer = std::make_unique<IndexBuffer>();
	mesh->indexBuffer->Initialize(device_, meshData.indices.size());
	std::memcpy(mesh->indexBuffer->GetIndexData(), meshData.indices.data(), sizeof(uint32_t) * meshData.indices.size());
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

MeshData MeshManager::ReIndexMeshData(const MeshData &meshData) {
	MeshData reIndexedMeshData;
	std::unordered_map<VertexData, uint32_t, VertexDataHash> vertexMap;
	vertexMap.reserve(meshData.vertices.size());
	for (uint32_t index : meshData.indices) {
		const VertexData &vertex = meshData.vertices[index];
		auto it = vertexMap.find(vertex);
		if (it != vertexMap.end()) {
			// 既に存在する頂点データの場合、既存のインデックスを使用
			reIndexedMeshData.indices.emplace_back(it->second);
		} else {
			// 新しい頂点データの場合、頂点リストに追加し、新しいインデックスを割り当て
			uint32_t newIndex = static_cast<uint32_t>(reIndexedMeshData.vertices.size());
			reIndexedMeshData.vertices.emplace_back(vertex);
			reIndexedMeshData.indices.emplace_back(newIndex);
			vertexMap.emplace(vertex, newIndex);
		}
	}
	return reIndexedMeshData;
}

Collision::Sphere MeshManager::CreateLocalSphere(const MeshData &meshData) {
	// 中心点の計算
	Vector3 center = { 0.0f, 0.0f, 0.0f };
	for (const VertexData &vertexData : meshData.vertices) {
		center.x += vertexData.position.x;
		center.y += vertexData.position.y;
		center.z += vertexData.position.z;
	}
	center /= static_cast<float>(meshData.vertices.size());

	// 半径の計算
	float radius = 0.0f;
	for (const VertexData &vertexData : meshData.vertices) {
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

Collision::AABB MeshManager::CreateLocalAABB(const MeshData &meshData) {
	Collision::AABB aabb = {
		.min = { meshData.vertices[0].position.x, meshData.vertices[0].position.y, meshData.vertices[0].position.z },
		.max = aabb.min
	};

	for (const VertexData &vertexData : meshData.vertices) {
		aabb.min.x = std::min(aabb.min.x, vertexData.position.x);
		aabb.min.y = std::min(aabb.min.y, vertexData.position.y);
		aabb.min.z = std::min(aabb.min.z, vertexData.position.z);
		aabb.max.x = std::max(aabb.max.x, vertexData.position.x);
		aabb.max.y = std::max(aabb.max.y, vertexData.position.y);
		aabb.max.z = std::max(aabb.max.z, vertexData.position.z);
	}
	return aabb;
}

Collision::OBB MeshManager::CreateLocalOBB(const MeshData &meshData) {
	// 中心点の計算
	Vector3 center = { 0.0f, 0.0f, 0.0f };
	for (const VertexData &vertexData : meshData.vertices) {
		center.x += vertexData.position.x;
		center.y += vertexData.position.y;
		center.z += vertexData.position.z;
	}
	center /= static_cast<float>(meshData.vertices.size());

	// 各軸方向の単位ベクトルの計算（仮にX軸、Y軸、Z軸とする）
	Vector3 orientations[3] = {
		{ 1.0f, 0.0f, 0.0f },	// X軸
		{ 0.0f, 1.0f, 0.0f },	// Y軸
		{ 0.0f, 0.0f, 1.0f }	// Z軸
	};

	// 各軸方向の半分の長さの計算
	Vector3 size = { 0.0f, 0.0f, 0.0f };
	for (const VertexData &vertexData : meshData.vertices) {
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