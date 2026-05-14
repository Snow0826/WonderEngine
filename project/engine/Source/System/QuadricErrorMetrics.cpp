#include "QuadricErrorMetrics.h"

std::vector<QEMSimplifier::ResultLOD> QEMSimplifier::Simplify(const std::vector<VertexData> &vertices, const std::vector<uint32_t> &indices, float errorScale, uint32_t minTriangles) {
	Build(vertices, indices);

	std::vector<ResultLOD> lods;
	lods.emplace_back(BuildLOD(0.0f));

	float lastError = 0.0f;

	while (!queue_.empty()) {
		QEMEdge e = queue_.top();
		queue_.pop();

		if (!IsValidEdge(e)) continue;

		CollapseEdge(e);

		UpdateEdges(e.v1);

		float error = e.cost;

		if (error > lastError * errorScale) {
			lods.emplace_back(BuildLOD(error));
			lastError = error;
		}

		if (CountValidTriangles() < minTriangles)
			break;
	}

	return lods;
}

void QEMSimplifier::Build(const std::vector<VertexData> &vertices, const std::vector<uint32_t> &indices) {
	vertices_.resize(vertices.size());
	for (size_t i = 0; i < vertices.size(); i++)
		vertices_[i].data = vertices[i];

	BuildTriangles(indices);
	BuildQuadrics();
	BuildAdjacency();
	BuildEdges();
}

void QEMSimplifier::BuildTriangles(const std::vector<uint32_t> &indices) {
	// インデックス数チェック
	if (indices.size() % 3 != 0) {
		return;
	}

	triangles_.reserve(indices.size() / 3);
	for (size_t i = 0; i < indices.size(); i += 3) {
		QEMTriangle t;
		for (size_t j = 0; j < 3; j++) {
			t.v[j] = indices[i + j];
		}
		t.valid = true;

		// ------------------------
		// ① 範囲チェック（必須）
		// ------------------------
		if (t.v[0] >= vertices_.size() ||
			t.v[1] >= vertices_.size() ||
			t.v[2] >= vertices_.size()) {
			continue;
		}

		// ------------------------
		// ② 退化三角形チェック（必須）
		// ------------------------
		if (t.v[0] == t.v[1] || t.v[1] == t.v[2] || t.v[2] == t.v[0]) {
			continue;
		}

		// ------------------------
		// ③ 面積チェック（超重要）
		// ------------------------
		Vector4 e0 = vertices_[t.v[1]].data.position - vertices_[t.v[0]].data.position;
		Vector4 e1 = vertices_[t.v[2]].data.position - vertices_[t.v[0]].data.position;
		float area = ToVector3(e0).cross(ToVector3(e1)).lengthSquare();
		if (area < 1e-12f) {
			continue;
		}

		triangles_.emplace_back(t);
	}
}

void QEMSimplifier::BuildQuadrics() {
	for (QEMVertex &v : vertices_)
		v.quadric = Matrix4x4{};

	for (QEMTriangle &tri : triangles_) {
		if (!tri.valid) continue;

		Vector3 p0 = ToVector3(vertices_[tri.v[0]].data.position);
		Vector3 p1 = ToVector3(vertices_[tri.v[1]].data.position);
		Vector3 p2 = ToVector3(vertices_[tri.v[2]].data.position);

		Matrix4x4 quadric = CreatePlaneQuadric({ p0, p1, p2 });

		for (size_t i = 0; i < 3; i++)
			vertices_[tri.v[i]].quadric += quadric;
	}
}

void QEMSimplifier::BuildAdjacency() {
	for (QEMTriangle &tri : triangles_) {
		for (uint32_t i = 0; i < 3; i++) {
			uint32_t a = tri.v[i];
			uint32_t b = tri.v[(i + 1) % 3];

			vertices_[a].neighbors.insert(b);
			vertices_[b].neighbors.insert(a);
		}
	}
}

void QEMSimplifier::BuildEdges() {
	for (uint32_t i = 0; i < vertices_.size(); i++) {
		for (uint32_t n : vertices_[i].neighbors) {
			if (i < n) {
				QEMEdge e;
				e.v1 = i;
				e.v2 = n;
				e.version1 = vertices_[i].version;
				e.version2 = vertices_[n].version;
				e.cost = ComputeEdgeCost(e);
				queue_.push(e);
			}
		}
	}
}

float QEMSimplifier::ComputeEdgeCost(QEMEdge &e) {
	Matrix4x4 quadric = vertices_[e.v1].quadric + vertices_[e.v2].quadric;

	Vector3 pos;
	if (!SolveOptimalPosition(quadric, pos)) {
		pos = (ToVector3(vertices_[e.v1].data.position) + ToVector3(vertices_[e.v2].data.position)) * 0.5f;
	}

	e.optimalPos = pos;

	Vector4 v = { pos.x, pos.y, pos.z, 1.0f };
	return v.dot(quadric * v);
}

void QEMSimplifier::CollapseEdge(const QEMEdge &e) {
	QEMVertex &vk = vertices_[e.v1];
	QEMVertex &vr = vertices_[e.v2];

	vk.data.position = { e.optimalPos.x, e.optimalPos.y, e.optimalPos.z, 1.0f };
	vk.quadric += vr.quadric;
	vk.version++;
	vr.valid = false;

	for (QEMTriangle &tri : triangles_) {
		if (!tri.valid) continue;

		for (size_t i = 0; i < 3; i++) {
			if (tri.v[i] == e.v2)
				tri.v[i] = e.v1;
		}

		if (tri.v[0] == tri.v[1] ||
			tri.v[1] == tri.v[2] ||
			tri.v[2] == tri.v[0])
			tri.valid = false;
	}

	for (uint32_t n : vr.neighbors) {
		vertices_[n].neighbors.erase(e.v2);

		if (n != e.v1) {
			vertices_[n].neighbors.insert(e.v1);
			vk.neighbors.insert(n);
		}
	}
}

void QEMSimplifier::UpdateEdges(uint32_t v) {
	for (auto n : vertices_[v].neighbors) {
		if (!vertices_[n].valid) continue;

		QEMEdge e;
		e.v1 = std::min(v, n);
		e.v2 = std::max(v, n);
		e.cost = ComputeEdgeCost(e);

		queue_.push(e);
	}
}

bool QEMSimplifier::IsValidEdge(const QEMEdge &e) {
	if (!vertices_[e.v1].valid ||
		!vertices_[e.v2].valid ||
		vertices_[e.v1].version != e.version1 ||
		vertices_[e.v2].version != e.version2)
		return false;
	return true;
}

QEMSimplifier::ResultLOD QEMSimplifier::BuildLOD(float error) {
	ResultLOD lod;
	lod.error = error;

	std::vector<uint32_t> remap(vertices_.size(), static_cast<uint32_t>(-1));

	for (size_t i = 0; i < vertices_.size(); i++) {
		if (!vertices_[i].valid) continue;
		remap[i] = static_cast<uint32_t>(lod.vertices.size());
		lod.vertices.emplace_back(vertices_[i].data);
	}

	for (auto &tri : triangles_) {
		if (!tri.valid) continue;

		lod.indices.emplace_back(remap[tri.v[0]]);
		lod.indices.emplace_back(remap[tri.v[1]]);
		lod.indices.emplace_back(remap[tri.v[2]]);
	}

	return lod;
}

size_t QEMSimplifier::CountValidTriangles() {
	size_t c = 0;
	for (auto &t : triangles_)
		if (t.valid) c++;
	return c;
}

Matrix4x4 QEMSimplifier::CreatePlaneQuadric(const Collision::Triangle &triangle) {
	Vector3 v0 = triangle.vertices[0];
	Vector3 v1 = triangle.vertices[1];
	Vector3 v2 = triangle.vertices[2];
	// 法線ベクトルの計算
	Vector3 normal = (v1 - v0).cross(v2 - v0).normalized();
	// 平面の方程式の係数の計算
	float a = normal.x;
	float b = normal.y;
	float c = normal.z;
	float d = -normal.dot(v0);
	// クアドラティック行列の作成
	Matrix4x4 p;
	p.m[0][0] = a;
	p.m[0][1] = b;
	p.m[0][2] = c;
	p.m[0][3] = d;
	return p * p.transpose();
}

bool QEMSimplifier::SolveOptimalPosition(const Matrix4x4 &quadric, Vector3 &optimalPosition) {
	Matrix3x3 a = Matrix3x3{
		quadric.m[0][0], quadric.m[0][1], quadric.m[0][2],
		quadric.m[1][0], quadric.m[1][1], quadric.m[1][2],
		quadric.m[2][0], quadric.m[2][1], quadric.m[2][2]
	};
	if (!a.inverse(a)) {
		return false; // 逆行列が存在しない場合は最適位置を解決できない
	}
	Vector3 b = { quadric.m[0][3], quadric.m[1][3], quadric.m[2][3] };
	optimalPosition = -b * a;
	return true;
}
