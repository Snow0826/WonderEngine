#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <vector>

/// @brief レンダリング関連名前空間
namespace Rendering {
	/// @brief ライン
	struct Line final {
		Vector3 start;	// 始点
		Vector3 end;	// 終点
		Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };	// 色
	};
}

class World;
namespace Collision {
	struct AABB;
	struct Sphere;
	struct Plane;
	struct OBB;
	struct Capsule;
}

/// @brief デバッグレンダラー
class DebugRenderer final {
public:
	/// @brief コンストラクタ
	/// @param world ワールド
	DebugRenderer(World *world) : world_(world) {}

	/// @brief フレーム開始
	void BeginFrame() { lines_.clear(); }

	/// @brief フレーム終了
	void EndFrame();

	/// @brief ラインの追加
	/// @param line ライン
	void AddLine(const Rendering::Line &line) { lines_.emplace_back(line); }

	/// @brief AABBの追加
	/// @param aabb AABB
	void AddAABB(const Collision::AABB &aabb);

	/// @brief 球の追加
	/// @param sphere 球
	void AddSphere(const Collision::Sphere &sphere);

	/// @brief 平面の追加
	/// @param plane 平面
	void AddPlane(const Collision::Plane &plane);

	/// @brief OBBの追加
	/// @param obb OBB
	void AddOBB(const Collision::OBB &obb);

	/// @brief カプセルの追加
	/// @param capsule
	void AddCapsule(const Collision::Capsule &capsule);

	/// @brief ライン数のデバッグ表示
	void Debug() const;

	/// @brief ライン数の取得
	/// @return ライン数
	uint32_t GetLineCount() const { return static_cast<uint32_t>(lines_.size()); }

private:
	World *world_ = nullptr;				// ワールド
	std::vector<Rendering::Line> lines_;	// ラインリスト
};