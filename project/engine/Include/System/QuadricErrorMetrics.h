#pragma once
#include <VertexData.h>
#include <Matrix3x3.h>
#include <Matrix4x4.h>
#include <Collision.h>
#include <vector>
#include <queue>
#include <unordered_set>

/// @brief QEM三角形
struct QEMTriangle {
	uint32_t v[3];
	bool valid = false;
};

/// @brief QEM頂点
struct QEMVertex {
	VertexData data;
	Matrix4x4 quadric;
	std::unordered_set<uint32_t> neighbors;
	uint32_t version = 0;
	bool valid = true;
};

/// @brief QEMエッジ
struct QEMEdge {
	uint32_t v1, v2;
	uint32_t version1, version2;
	float cost;
	Vector3 optimalPos;

	bool operator<(const QEMEdge &other) const {
		return cost > other.cost; // min-heap
	}
};

/// @brief QEM簡略化クラス
class QEMSimplifier {
public:
	struct ResultLOD {
		std::vector<VertexData> vertices;
		std::vector<uint32_t> indices;
		float error = 0.0f;
	};

	std::vector<ResultLOD> Simplify(
		const std::vector<VertexData> &vertices,
		const std::vector<uint32_t> &indices,
		float errorScale = 2.0f,
		uint32_t minTriangles = 32);

private:
	std::vector<QEMVertex> vertices_;
	std::vector<QEMTriangle> triangles_;
	std::priority_queue<QEMEdge> queue_;

private:

	void Build(const std::vector<VertexData> &vertices, const std::vector<uint32_t> &indices);

	void BuildTriangles(const std::vector<uint32_t> &indices);

	void BuildQuadrics();

	void BuildAdjacency();

	void BuildEdges();

	float ComputeEdgeCost(QEMEdge &e);

	void CollapseEdge(const QEMEdge &e);

	void UpdateEdges(uint32_t v);

	bool IsValidEdge(const QEMEdge &e);

	ResultLOD BuildLOD(float error);

	size_t CountValidTriangles();

	Matrix4x4 CreatePlaneQuadric(const Collision::Triangle &triangle);

	bool SolveOptimalPosition(const Matrix4x4 &quadric, Vector3 &optimalPosition);
};