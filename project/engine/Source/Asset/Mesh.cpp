#define NOMINMAX
#include "Mesh.h"
#include "Device.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "TreeGenerator.h"
#include "Collision.h"
#include "World.h"
#include "Resource.h"
#include "Interpolation.h"
#include "Logger.h"
#include <algorithm>
#include <numbers>

MeshManager::MeshManager(Device *device, std::ofstream *logStream) : device_(device), logStream_(logStream) {}
MeshManager::~MeshManager() = default;

void MeshManager::CreateMesh(const std::string &meshName, const Mesh &mesh) {
	// すでに読み込まれている場合は何もしない
	auto it = meshBuffers_.find(meshName);
	if (it != meshBuffers_.end()) {
		Logger::Log(*logStream_, "Mesh already created: " + meshName + "\n");
		return;
	}

	std::unique_ptr<MeshBuffer> meshBuffer = std::make_unique<MeshBuffer>();

	// 頂点バッファの生成と初期化
	meshBuffer->vertexBuffer = std::make_unique<VertexBuffer>();
	meshBuffer->vertexBuffer->Initialize(device_, mesh.vertices.size());
	std::memcpy(meshBuffer->vertexBuffer->GetVertexData(), mesh.vertices.data(), sizeof(VertexData) * mesh.vertices.size());

	// インデックスバッファの生成と初期化
	meshBuffer->indexBuffer = std::make_unique<IndexBuffer>();
	meshBuffer->indexBuffer->Initialize(device_, mesh.indices.size());
	std::memcpy(meshBuffer->indexBuffer->GetIndexData(), mesh.indices.data(), sizeof(uint32_t) * mesh.indices.size());

	meshBuffers_.emplace(meshName, std::move(meshBuffer));
}

void MeshManager::CreateSprite(const std::string &meshName) {
	// すでに読み込まれている場合は何もしない
	auto it = meshBuffers_.find(meshName);
	if (it != meshBuffers_.end()) {
		Logger::Log(*logStream_, "Mesh already created: " + meshName + "\n");
		return;
	}

	std::unique_ptr<MeshBuffer> mesh = std::make_unique<MeshBuffer>();

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
	meshBuffers_.emplace(meshName, std::move(mesh));
}

void MeshManager::CreatePlane(const std::string &meshName) {
	// すでに読み込まれている場合は何もしない
	auto it = meshBuffers_.find(meshName);
	if (it != meshBuffers_.end()) {
		Logger::Log(*logStream_, "Mesh already created: " + meshName + "\n");
		return;
	}

	std::unique_ptr<MeshBuffer> mesh = std::make_unique<MeshBuffer>();

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
	meshBuffers_.emplace(meshName, std::move(mesh));
}

void MeshManager::CreateBox(const std::string &meshName) {
	// すでに読み込まれている場合は何もしない
	auto it = meshBuffers_.find(meshName);
	if (it != meshBuffers_.end()) {
		Logger::Log(*logStream_, "Mesh already created: " + meshName + "\n");
		return;
	}

	std::unique_ptr<MeshBuffer> mesh = std::make_unique<MeshBuffer>();

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
	meshBuffers_.emplace(meshName, std::move(mesh));
}

void MeshManager::CreateRing(const std::string &meshName, uint32_t divide, float outerRadius, float innerRadius) {
	// すでに読み込まれている場合は何もしない
	auto it = meshBuffers_.find(meshName);
	if (it != meshBuffers_.end()) {
		Logger::Log(*logStream_, "Mesh already created: " + meshName + "\n");
		return;
	}

	std::unique_ptr<MeshBuffer> mesh = std::make_unique<MeshBuffer>();

	// 頂点バッファの生成と初期化
	mesh->vertexBuffer = std::make_unique<VertexBuffer>();
	mesh->vertexBuffer->Initialize(device_, (divide + 1) * 2);
	VertexData *vertexData = mesh->vertexBuffer->GetVertexData();

	const float radianPerDivide = std::numbers::pi_v<float> *2.0f / static_cast<float>(divide);
	for (uint32_t index = 0; index <= divide; ++index) {
		float sin = std::sin(radianPerDivide * index);
		float cos = std::cos(radianPerDivide * index);
		float u = static_cast<float>(index) / static_cast<float>(divide);
		uint32_t baseVertex = index * 2;
		// 外周の頂点データの生成
		vertexData[baseVertex].position = { -sin * outerRadius, cos * outerRadius, 0.0f, 1.0f };	// 頂点座標
		vertexData[baseVertex].texcoord = { u, 0.0f };												// テクスチャ座標
		vertexData[baseVertex].normal = { 0.0f, 0.0f, 1.0f };										// 法線
		// 内周の頂点データの生成
		vertexData[baseVertex + 1].position = { -sin * innerRadius, cos * innerRadius, 0.0f, 1.0f };	// 頂点座標
		vertexData[baseVertex + 1].texcoord = { u, 1.0f };												// テクスチャ座標
		vertexData[baseVertex + 1].normal = { 0.0f, 0.0f, 1.0f };										// 法線
	}

	// インデックスバッファの生成と初期化
	mesh->indexBuffer = std::make_unique<IndexBuffer>();
	mesh->indexBuffer->Initialize(device_, divide * 6);
	uint32_t *indexData = mesh->indexBuffer->GetIndexData();
	for (uint32_t i = 0; i < divide; i++) {
		uint32_t baseVertex = i * 2;
		uint32_t baseIndex = i * 6;
		// 三角形1
		indexData[baseIndex] = baseVertex;			// 外周の頂点
		indexData[baseIndex + 1] = baseVertex + 1;	// 内周の頂点
		indexData[baseIndex + 2] = baseVertex + 2;	// 次の外周の頂点
		// 三角形2
		indexData[baseIndex + 3] = baseVertex + 2;	// 次の外周の頂点
		indexData[baseIndex + 4] = baseVertex + 1;	// 内周の頂点
		indexData[baseIndex + 5] = baseVertex + 3;	// 次の内周の頂点
	}

	meshBuffers_.emplace(meshName, std::move(mesh));
}

void MeshManager::CreateCylinder(const std::string &meshName, uint32_t divide, float topRadius, float bottomRadius, float height, bool cap) {
	// すでに読み込まれている場合は何もしない
	auto it = meshBuffers_.find(meshName);
	if (it != meshBuffers_.end()) {
		Logger::Log(*logStream_, "Mesh already created: " + meshName + "\n");
		return;
	}

	// 頂点データの生成
	std::vector<VertexData> vertices;
	const float radianPerDivide = std::numbers::pi_v<float> *2.0f / static_cast<float>(divide);

	// 円柱の側面の頂点データの生成
	for (uint32_t index = 0; index <= divide; ++index) {
		float sin = std::sin(radianPerDivide * index);
		float cos = std::cos(radianPerDivide * index);
		float u = static_cast<float>(index) / static_cast<float>(divide);
		float slope = (bottomRadius - topRadius) / height;
		vertices.emplace_back(VertexData{
			.position = { -sin * topRadius, height, cos * topRadius, 1.0f },
			.texcoord = { u, 0.0f },
			.normal = Vector3{ -sin, slope, cos }.normalized()
			});
		vertices.emplace_back(VertexData{
			.position = { -sin * bottomRadius, 0.0f, cos * bottomRadius, 1.0f },
			.texcoord = { u, 1.0f },
			.normal = Vector3{ -sin, slope, cos }.normalized()
			});
	}

	uint32_t topCenterIndex = 0;
	uint32_t topBaseIndex = 0;
	uint32_t bottomCenterIndex = 0;
	uint32_t bottomBaseIndex = 0;
	if (cap) {
		// 円柱の上面の頂点データの生成
		topCenterIndex = static_cast<uint32_t>(vertices.size());
		vertices.emplace_back(VertexData{
			.position = { 0.0f, height, 0.0f, 1.0f },
			.texcoord = { 0.5f, 0.5f },
			.normal = { 0.0f, 1.0f, 0.0f }
			});

		topBaseIndex = static_cast<uint32_t>(vertices.size());
		for (uint32_t index = 0; index <= divide; ++index) {
			float sin = std::sin(radianPerDivide * index);
			float cos = std::cos(radianPerDivide * index);
			vertices.emplace_back(VertexData{
				.position = { -sin * topRadius, height, cos * topRadius, 1.0f },
				.texcoord = { -sin * 0.5f + 0.5f, -cos * 0.5f + 0.5f },
				.normal = { 0.0f, 1.0f, 0.0f }
				});
		}

		// 円柱の下面の頂点データの生成
		bottomCenterIndex = static_cast<uint32_t>(vertices.size());
		vertices.emplace_back(VertexData{
			.position = { 0.0f, 0.0f, 0.0f, 1.0f },
			.texcoord = { 0.5f, 0.5f },
			.normal = { 0.0f, -1.0f, 0.0f }
			});

		bottomBaseIndex = static_cast<uint32_t>(vertices.size());
		for (uint32_t index = 0; index <= divide; ++index) {
			float sin = std::sin(radianPerDivide * index);
			float cos = std::cos(radianPerDivide * index);
			vertices.emplace_back(VertexData{
				.position = { -sin * bottomRadius, 0.0f, cos * bottomRadius, 1.0f },
				.texcoord = { -sin * 0.5f + 0.5f, cos * 0.5f + 0.5f },
				.normal = { 0.0f, -1.0f, 0.0f }
				});
		}
	}

	// インデックスデータの生成
	std::vector<uint32_t> indices;

	// 円柱の側面のインデックスデータの生成
	for (uint32_t i = 0; i < divide; ++i) {
		uint32_t baseVertex = i * 2;
		// 三角形1
		indices.emplace_back(baseVertex);		// 上面の頂点
		indices.emplace_back(baseVertex + 1);	// 下面の頂点
		indices.emplace_back(baseVertex + 2);	// 次の上面の頂点
		// 三角形2
		indices.emplace_back(baseVertex + 2);	// 次の上面の頂点
		indices.emplace_back(baseVertex + 1);	// 下面の頂点
		indices.emplace_back(baseVertex + 3);	// 次の下面の頂点
	}

	if (cap) {
		// 円柱の上面のインデックスデータの生成
		for (uint32_t i = 0; i < divide; i++) {
			indices.emplace_back(topCenterIndex);		// 上面の中心の頂点
			indices.emplace_back(topBaseIndex + i);		// 上面の周囲の頂点
			indices.emplace_back(topBaseIndex + i + 1);	// 上面の周囲の頂点
		}

		// 円柱の下面のインデックスデータの生成
		for (uint32_t i = 0; i < divide; i++) {
			indices.emplace_back(bottomCenterIndex);		// 下面の中心の頂点
			indices.emplace_back(bottomBaseIndex + i + 1);	// 下面の周囲の頂点
			indices.emplace_back(bottomBaseIndex + i);		// 下面の周囲の頂点
		}
	}

	// メッシュの生成
	std::unique_ptr<MeshBuffer> mesh = std::make_unique<MeshBuffer>();

	// 頂点バッファの生成と初期化
	mesh->vertexBuffer = std::make_unique<VertexBuffer>();
	mesh->vertexBuffer->Initialize(device_, vertices.size());
	std::memcpy(mesh->vertexBuffer->GetVertexData(), vertices.data(), sizeof(VertexData) * vertices.size());

	// インデックスバッファの生成と初期化
	mesh->indexBuffer = std::make_unique<IndexBuffer>();
	mesh->indexBuffer->Initialize(device_, indices.size());
	std::memcpy(mesh->indexBuffer->GetIndexData(), indices.data(), sizeof(uint32_t) * indices.size());
	meshBuffers_.emplace(meshName, std::move(mesh));
}

void MeshManager::CreateTree(const std::string &meshName, const BranchMesh &branchMesh, uint32_t lengthSegments, uint32_t radialSegments, float uvScale) {
	Mesh treeMesh;
	for (const BranchPath &branchPath : branchMesh.paths) {
		if (branchPath.points.size() < 2) {
			continue;
		}

		const Vector3 reference = (std::abs(branchPath.points[0].direction.y) < 0.9f) ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
		Vector3 frameNormal = (frameNormal - branchPath.points[0].direction * frameNormal.dot(branchPath.points[0].direction)).normalized();
		for (size_t i = 0; i + 1 < branchPath.points.size(); ++i) {
			Mesh segmentMesh = CreateHermiteTubeSegment(branchPath.points[i], branchPath.points[i + 1], lengthSegments, radialSegments, frameNormal, uvScale);
			AppendMesh(treeMesh, segmentMesh);
		}
	}

	CreateMesh(meshName, treeMesh);
}

void MeshManager::Draw(const std::string &meshName, uint32_t instanceCount) const {
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = meshBuffers_.at(meshName)->vertexBuffer->GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW indexBufferView = meshBuffers_.at(meshName)->indexBuffer->GetIndexBufferView();
	UINT indexCount = meshBuffers_.at(meshName)->indexBuffer->GetIndices();
	device_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);	// VBVの設定
	device_->GetCommandList()->IASetIndexBuffer(&indexBufferView);	// IBVの設定
	device_->GetCommandList()->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);	// 描画
}

D3D12_VERTEX_BUFFER_VIEW MeshManager::GetVertexBufferView(const std::string &meshName) const {
	return meshBuffers_.at(meshName)->vertexBuffer->GetVertexBufferView();
}

D3D12_INDEX_BUFFER_VIEW MeshManager::GetIndexBufferView(const std::string &meshName) const {
	return meshBuffers_.at(meshName)->indexBuffer->GetIndexBufferView();
}

VertexData *MeshManager::GetVertexData(const std::string &meshName) const {
	return meshBuffers_.at(meshName)->vertexBuffer->GetVertexData();
}

size_t MeshManager::GetVertexCount(const std::string &meshName) const {
	return meshBuffers_.at(meshName)->vertexBuffer->GetVertices();
}

UINT MeshManager::GetIndexCount(const std::string &meshName) const {
	return meshBuffers_.at(meshName)->indexBuffer->GetIndices();
}

Mesh MeshManager::ReIndexMesh(const Mesh &mesh) {
	Mesh reIndexedMesh;
	std::unordered_map<VertexData, uint32_t, VertexDataHash> vertexMap;
	vertexMap.reserve(mesh.vertices.size());
	for (uint32_t index : mesh.indices) {
		const VertexData &vertex = mesh.vertices[index];
		auto it = vertexMap.find(vertex);
		if (it != vertexMap.end()) {
			// 既に存在する頂点データの場合、既存のインデックスを使用
			reIndexedMesh.indices.emplace_back(it->second);
		} else {
			// 新しい頂点データの場合、頂点リストに追加し、新しいインデックスを割り当て
			uint32_t newIndex = static_cast<uint32_t>(reIndexedMesh.vertices.size());
			reIndexedMesh.vertices.emplace_back(vertex);
			reIndexedMesh.indices.emplace_back(newIndex);
			vertexMap.emplace(vertex, newIndex);
		}
	}
	return reIndexedMesh;
}

Collision::Sphere MeshManager::CreateLocalSphere(const std::string &meshName) {
	VertexData *vertexData = meshBuffers_.at(meshName)->vertexBuffer->GetVertexData();
	size_t vertices = meshBuffers_.at(meshName)->vertexBuffer->GetVertices();

	// 中心点の計算
	Vector3 center = { 0.0f, 0.0f, 0.0f };
	for (size_t i = 0; i < vertices; ++i) {
		center.x += vertexData[i].position.x;
		center.y += vertexData[i].position.y;
		center.z += vertexData[i].position.z;
	}
	center /= static_cast<float>(vertices);

	// 半径の計算
	float radius = 0.0f;
	for (size_t i = 0; i < vertices; ++i) {
		float distance = center.distanceFrom({ vertexData[i].position.x, vertexData[i].position.y, vertexData[i].position.z });
		radius = std::max(radius, distance);
	}

	// バウンディングスフィアの作成
	Collision::Sphere sphere = {
		.center = center,
		.radius = radius
	};
	return sphere;
}

Collision::AABB MeshManager::CreateLocalAABB(const std::string &meshName) {
	VertexData *vertexData = meshBuffers_.at(meshName)->vertexBuffer->GetVertexData();
	size_t vertices = meshBuffers_.at(meshName)->vertexBuffer->GetVertices();

	Collision::AABB aabb = {
		.min = { vertexData[0].position.x, vertexData[0].position.y, vertexData[0].position.z },
		.max = aabb.min
	};

	for (size_t i = 0; i < vertices; ++i) {
		aabb.min.x = std::min(aabb.min.x, vertexData[i].position.x);
		aabb.min.y = std::min(aabb.min.y, vertexData[i].position.y);
		aabb.min.z = std::min(aabb.min.z, vertexData[i].position.z);
		aabb.max.x = std::max(aabb.max.x, vertexData[i].position.x);
		aabb.max.y = std::max(aabb.max.y, vertexData[i].position.y);
		aabb.max.z = std::max(aabb.max.z, vertexData[i].position.z);
	}
	return aabb;
}

Collision::OBB MeshManager::CreateLocalOBB(const std::string &meshName) {
	VertexData *vertexData = meshBuffers_.at(meshName)->vertexBuffer->GetVertexData();
	size_t vertices = meshBuffers_.at(meshName)->vertexBuffer->GetVertices();

	// 中心点の計算
	Vector3 center = { 0.0f, 0.0f, 0.0f };
	for (size_t i = 0; i < vertices; ++i) {
		center.x += vertexData[i].position.x;
		center.y += vertexData[i].position.y;
		center.z += vertexData[i].position.z;
	}
	center /= static_cast<float>(vertices);
	// 各軸方向の単位ベクトルの計算（仮にX軸、Y軸、Z軸とする）
	Vector3 orientations[3] = {
		{ 1.0f, 0.0f, 0.0f },	// X軸
		{ 0.0f, 1.0f, 0.0f },	// Y軸
		{ 0.0f, 0.0f, 1.0f }	// Z軸
	};

	// 各軸方向の半分の長さの計算
	Vector3 size = { 0.0f, 0.0f, 0.0f };
	for (size_t i = 0; i < vertices; ++i) {
		Vector3 relativePos = {
			vertexData[i].position.x - center.x,
			vertexData[i].position.y - center.y,
			vertexData[i].position.z - center.z
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

Mesh MeshManager::CreateHermiteTubeSegment(const BranchPathPoint &start, const BranchPathPoint &end, uint32_t lengthSegments, uint32_t radialSegments, Vector3 &frameNormal, float uvScale) {
	Mesh mesh;
	if (lengthSegments == 0 || radialSegments < 3) {
		return mesh;
	}

	const uint32_t ringSize = radialSegments + 1;
	const float segmentDistance = end.distance - start.distance;
	const float radiusSlope = segmentDistance > 0.0001f ? (end.radius - start.radius) / segmentDistance : 0.0f;
	const float segmentLength = (end.position - start.position).length();
	const Vector3 tangent0 = start.direction.normalized() * segmentLength;
	const Vector3 tangent1 = end.direction.normalized() * segmentLength;

	// 最初の断面の基準方向を作る
	for (uint32_t i = 0; i <= lengthSegments; ++i) {
		const float t = static_cast<float>(i) / static_cast<float>(lengthSegments);
		const Vector3 center = Interpolation<Vector3>::Hermite(start.position, end.position, tangent0, tangent1, t);
		const Vector3 tangent = Interpolation<Vector3>::HermiteDerivative(start.position, end.position, tangent0, tangent1, t).normalized();

		// 前の断面の向きを現在の接線に直交化し、断面の回転を抑える
		frameNormal = (frameNormal - tangent * frameNormal.dot(tangent)).normalized();

		// 接線がframeNormalとほぼ平行だと正規化できないので補正する
		if (frameNormal.isZero()) {
			const Vector3 reference = (std::abs(tangent.y) < 0.9f) ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
			frameNormal = tangent.cross(reference).normalized();
		}

		const Vector3 frameBinormal = tangent.cross(frameNormal).normalized();
		const float radius = start.radius + (end.radius - start.radius) * t;
		const float distance = start.distance + segmentDistance * t;

		for (uint32_t j = 0; j <= radialSegments; ++j) {
			const float u = static_cast<float>(j) / static_cast<float>(radialSegments);
			const float angle = u * std::numbers::pi_v<float> *2.0f;
			const Vector3 radial = frameNormal * std::cos(angle) + frameBinormal * std::sin(angle);
			const Vector3 position = center + radial * radius;
			const Vector3 normal = (radial - tangent * radiusSlope).normalized();
			mesh.vertices.emplace_back(VertexData{
				.position = { position.x, position.y, position.z, 1.0f },
				.texcoord = { u, distance * uvScale },
				.normal = normal,
				});
		}
	}

	for (uint32_t i = 0; i < lengthSegments; ++i) {
		for (uint32_t j = 0; j < radialSegments; ++j) {
			const uint32_t a = i * ringSize + j;
			const uint32_t b = a + ringSize;
			mesh.indices.insert(mesh.indices.end(), { a, b, a + 1, a + 1, b, b + 1 });
		}
	}
	return mesh;
}

void MeshManager::AppendMesh(Mesh &destination, const Mesh &source) {
	const uint32_t offset = static_cast<uint32_t>(destination.vertices.size());
	destination.vertices.insert(destination.vertices.end(), source.vertices.begin(), source.vertices.end());
	for (uint32_t index : source.indices) {
		destination.indices.emplace_back(index + offset);
	}
}