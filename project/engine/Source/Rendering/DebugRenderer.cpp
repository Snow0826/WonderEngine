#include "DebugRenderer.h"
#include "World.h"
#include "Collision.h"
#include <numbers>

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

namespace {
	// AABBの12個のエッジを表すインデックスペア
	std::vector<std::vector<uint32_t>> edgesAABB = {
		{0, 1}, {1, 2}, {2, 3}, {3, 0}, // 下の面
		{4, 5}, {5, 6}, {6, 7}, {7, 4}, // 上の面
		{0, 4}, {1, 5}, {2, 6}, {3, 7}  // 垂直の辺
	};

	constexpr uint32_t kSubdivision = 8;	// 球の分割数

	// 平面の4個の辺を表すインデックスペア
	std::vector<std::vector<uint32_t>> edgesPlane = {
		{0, 2}, {0, 3}, {1, 2}, {1, 3}
	};

	// OBBの12個のエッジを表すインデックスペア
	std::vector<std::vector<uint32_t>> edgesOBB = {
		{0, 1}, {1, 3}, {3, 2}, {2, 0}, // 前面の辺
		{4, 5}, {5, 7}, {7, 6}, {6, 4}, // 背面の辺
		{0, 4}, {1, 5}, {2, 6}, {3, 7}  // 側面の辺
	};
}

void DebugRenderer::EndFrame() {
	world_->CopyLineData(lines_.data(), sizeof(Rendering::Line) * lines_.size());
}

void DebugRenderer::AddAABB(const Collision::AABB &aabb) {
	Vector3 vertices[8] = {
		{aabb.min.x, aabb.min.y, aabb.min.z},
		{aabb.max.x, aabb.min.y, aabb.min.z},
		{aabb.max.x, aabb.max.y, aabb.min.z},
		{aabb.min.x, aabb.max.y, aabb.min.z},
		{aabb.min.x, aabb.min.y, aabb.max.z},
		{aabb.max.x, aabb.min.y, aabb.max.z},
		{aabb.max.x, aabb.max.y, aabb.max.z},
		{aabb.min.x, aabb.max.y, aabb.max.z}
	};

	for (size_t i = 0; i < edgesAABB.size(); i++) {
		AddLine({ .start = vertices[edgesAABB[i][0]], .end = vertices[edgesAABB[i][1]] });
	}
}

void DebugRenderer::AddSphere(const Collision::Sphere &sphere) {
	float kLatEvery = std::numbers::pi_v<float> / static_cast<float>(kSubdivision);			// 緯度分割1つ分の角度
	float kLonEvery = std::numbers::pi_v<float> *2.0f / static_cast<float>(kSubdivision);	// 経度分割1つ分の角度

	// 緯度の方向に分割 -π/2 ~ π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex; // 現在の緯度

		// 経度の方向に分割 0 ~ 2π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery; // 現在の経度
			float nextLon = lon + kLonEvery;
			float nextLat = lat + kLatEvery;

			// a,b,cのworld座標系を求める
			Vector3 a, b, c;
			a = { std::cos(lat) * std::cos(lon), std::sin(lat), std::cos(lat) * std::sin(lon) };
			b = { std::cos(nextLat) * std::cos(lon), std::sin(nextLat), std::cos(nextLat) * std::sin(lon) };
			c = { std::cos(lat) * std::cos(nextLon), std::sin(lat), std::cos(lat) * std::sin(nextLon) };
			a = sphere.center + sphere.radius * a;
			b = sphere.center + sphere.radius * b;
			c = sphere.center + sphere.radius * c;

			// ラインの生成
			AddLine({ .start = a, .end = b });
			AddLine({ .start = a, .end = c });
		}
	}
}

void DebugRenderer::AddPlane(const Collision::Plane &plane) {
	Vector3 center = plane.normal * plane.distance;	// 平面の中心
	Vector3 perpendiculars[4];
	perpendiculars[0] = plane.normal.perpendicular().normalized();	// 平面の法線に垂直なベクトル
	perpendiculars[1] = -perpendiculars[0];							// perpendiculars[0]の逆ベクトル
	perpendiculars[2] = perpendiculars[0].cross(plane.normal);		// 法線とperpendiculars[0]の外積
	perpendiculars[3] = -perpendiculars[2];							// perpendiculars[2]の逆ベクトル
	Vector3 points[4];
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = perpendiculars[index] * 16.0f;
		points[index] = center + extend;
	}

	for (size_t i = 0; i < edgesPlane.size(); i++) {
		AddLine({ .start = points[edgesPlane[i][0]], .end = points[edgesPlane[i][1]] });
	}
}

void DebugRenderer::AddOBB(const Collision::OBB &obb) {
	// OBBの8頂点を計算
	Vector3 size = obb.size;
	Vector3 orientationX = obb.orientations[0] * size.x;
	Vector3 orientationY = obb.orientations[1] * size.y;
	Vector3 orientationZ = obb.orientations[2] * size.z;
	Vector3 vertices[8] = {
		obb.center + orientationX + orientationY + orientationZ,	// 0: +X, +Y, +Z
		obb.center - orientationX + orientationY + orientationZ,	// 1: -X, +Y, +Z
		obb.center + orientationX - orientationY + orientationZ,	// 2: +X, -Y, +Z
		obb.center - orientationX - orientationY + orientationZ,	// 3: -X, -Y, +Z
		obb.center + orientationX + orientationY - orientationZ,	// 4: +X, +Y, -Z
		obb.center - orientationX + orientationY - orientationZ,	// 5: -X, +Y, -Z
		obb.center + orientationX - orientationY - orientationZ,	// 6: +X, -Y, -Z
		obb.center - orientationX - orientationY - orientationZ		// 7: -X, -Y, -Z
	};

	for (size_t i = 0; i < edgesOBB.size(); i++) {
		AddLine({ .start = vertices[edgesOBB[i][0]], .end = vertices[edgesOBB[i][1]] });
	}
}

void DebugRenderer::AddCapsule(const Collision::Capsule &capsule) {
	Vector3 axis = capsule.segment.diff;
	Vector3 start = capsule.segment.origin;
	Vector3 end = capsule.segment.origin + capsule.segment.diff;

	Vector3 right, up;
	MakeBasis(axis, right, up);

	constexpr int div = 16;
	float step = 2.0f * std::numbers::pi_v<float> / div;

	// 1. 中心線
	AddLine({ .start = start, .end = end });

	// 2. 両端の球
	AddSphere({ .center = start, .radius = capsule.radius });
	AddSphere({ .center = end, .radius = capsule.radius });

	// 3. 円筒の側面線
	for (int i = 0; i < div; i += div / 4) {
		float a = step * i;
		Vector3 offset = capsule.radius * (cosf(a) * right + sinf(a) * up);
		AddLine({ .start = (start + offset), .end = (end + offset) });
	}
}

void DebugRenderer::Debug() const {
#ifdef USE_IMGUI
	ImGui::Text("Line Count: %u", lines_.size());
#endif // USE_IMGUI
}