#pragma once
#include "Vector3.h"

struct Matrix4x4;

namespace Collision {
	/// @brief 直線
	struct Line final {
		Vector3 origin;	// 始点
		Vector3 diff;	// 終点への差分ベクトル
	};

	/// @brief 半直線
	struct Ray final {
		Vector3 origin;	// 始点
		Vector3 diff;	// 終点への差分ベクトル
	};

	/// @brief 線分
	struct Segment final {
		Vector3 origin;	// 始点
		Vector3 diff;	// 終点への差分ベクトル
	};

	/// @brief カプセル
	struct Capsule final {
		Segment segment;		// 中心線
		float radius = 0.0f;	// 半径
	};

	/// @brief 球
	struct Sphere final {
		Vector3 center;			// 中心点
		float radius = 0.0f;	// 半径
	};

	/// @brief 平面
	struct Plane final {
		Vector3 normal;			// 法線ベクトル
		float distance = 0.0f;	// 原点からの距離
	};

	/// @brief 三角形
	struct Triangle final {
		Vector3 vertices[3];	// 頂点
	};

	/// @brief 軸並行境界箱
	struct AABB final {
		Vector3 min;	// 最小点
		Vector3 max;	// 最大点
	};

	/// @brief 有向境界箱
	struct OBB final {
		Vector3 center;				// 中心点
		Vector3 orientations[3];	// 各軸の単位ベクトル
		Vector3 size;				// 各軸方向の半分の長さ
	};
}

/// @brief AABBの中心点を求める関数
/// @param aabb AABB
/// @return 中心点
Vector3 GetCenter(const Collision::AABB &aabb);

/// @brief AABBの各軸方向の半分の長さを求める関数
/// @param aabb AABB
/// @return 各軸方向の半分の長さ
Vector3 GetExtent(const Collision::AABB &aabb);

/// @brief OBB行列の作成
/// @param obb OBB
/// @return OBB行列
Matrix4x4 MakeOBBMatrix(const Collision::OBB &obb);

/// @brief 反射ベクトルを求める関数
/// @param input 入射ベクトル
/// @param normal 法線
/// @return 反射ベクトル
Vector3 Reflect(const Vector3 &input, const Vector3 &normal);

/// @brief 射影関数
/// @param v1 ベクトル1
/// @param v2 ベクトル2
/// @return 正射影ベクトル
Vector3 Project(const Vector3 &v1, const Vector3 &v2);

/// @brief 点と直線の最近接点を求める関数
/// @param point 点
/// @param line 直線
/// @return 最近接点
Vector3 ClosestPoint(const Vector3 &point, const Collision::Line &line);

/// @brief 点と半直線の最近接点を求める関数
/// @param point 点
/// @param ray 半直線
/// @return 最近接点
Vector3 ClosestPoint(const Vector3 &point, const Collision::Ray &ray);

/// @brief 点と線分の最近接点を求める関数
/// @param point 点
/// @param segment 線分
/// @return 最近接点
Vector3 ClosestPoint(const Vector3 &point, const Collision::Segment &segment);

/// @brief 点と平面の最近接点を求める
/// @param point 点
/// @param plane 平面
/// @return 最近接点
Vector3 ClosestPoint(const Vector3 &point, const Collision::Plane &plane);

/// @brief 点とAABBの最近接点を求める
/// @param point 点
/// @param aabb AABB
/// @return 最近接点
Vector3 ClosestPoint(const Vector3 &point, const Collision::AABB &aabb);

/// @brief 点とOBBの最近接点を求める
/// @param point 点
/// @param obb OBB
/// @return 最近接点
Vector3 ClosestPoint(const Vector3 &point, const Collision::OBB &obb);

/// @brief 線分と平面の最近接点を求める関数
/// @param segment 線分Collision::
/// @param plane 平面
/// @return 最近接点
Vector3 ClosestPoint(const Collision::Segment &segment, const Collision::Plane &plane);

/// @brief 線分とAABBの最近接点を求める
/// @param segment 線分
/// @param aabb AABB
/// @return 最近接点
Vector3 ClosestPoint(const Collision::Segment &segment, const Collision::AABB &aabb);

/// @brief 線分とOBBの最近接点を求める
/// @param segment 線分
/// @param obb OBB
/// @return 最近接点
Vector3 ClosestPoint(const Collision::Segment &segment, const Collision::OBB &obb);

/// @brief 線分と線分の最近接点を求める
/// @param segment1 線分1
/// @param segment2 線分2
/// @return 最近接点
Vector3 ClosestPoint(const Collision::Segment &segment1, const Collision::Segment &segment2);

/// @brief 球と球の最近接点を求める
/// @param sphere1 球1
/// @param sphere2 球2
/// @return 最近接点
Vector3 ClosestPoint(const Collision::Sphere &sphere1, const Collision::Sphere &sphere2);

/// @brief 点とAABBの衝突面法線を求める
/// @param point 点
/// @param aabb AABB
/// @return 衝突面法線
Vector3 Normal(const Vector3 &point, const Collision::AABB &aabb);

/// @brief 点とOBBの衝突面法線を求める
/// @param point 点
/// @param obb OBB
/// @return 衝突面法線
Vector3 Normal(const Vector3 &point, const Collision::OBB &obb);

/// @brief 線分とAABBの衝突面法線を求める関数
/// @param segment 線分
/// @param aabb AABB
/// @return 衝突面法線
Vector3 Normal(const Collision::Segment &segment, const Collision::AABB &aabb);

/// @brief 線分とOBBの衝突面法線を求める関数
/// @param segment 線分
/// @param obb OBB
/// @return 衝突面法線
Vector3 Normal(const Collision::Segment &segment, const Collision::OBB &obb);

/// @brief 点と平面の距離を求める
/// @param point 点
/// @param plane 平面
/// @return 距離
float Distance(const Vector3 &point, const Collision::Plane &plane);

/// @brief 点とAABBの距離を求める
/// @param point 点
/// @param aabb AABB
/// @return 距離
float Distance(const Vector3 &point, const Collision::AABB &aabb);

/// @brief 点とOBBの距離を求める
/// @param point 点
/// @param obb OBB
/// @return 距離
float Distance(const Vector3 &point, const Collision::OBB &obb);

/// @brief 線分と平面の距離を求める関数
/// @param segment 線分
/// @param plane 平面
/// @return 距離
float Distance(const Collision::Segment &segment, const Collision::Plane &plane);

/// @brief 線分とAABBの距離を求める関数
/// @param segment 線分
/// @param aabb AABB
/// @return 距離
float Distance(const Collision::Segment &segment, const Collision::AABB &aabb);

/// @brief 線分とOBBの距離を求める関数
/// @param segment 線分
/// @param obb OBB
/// @return 距離
float Distance(const Collision::Segment &segment, const Collision::OBB &obb);

/// @brief 線分と線分の距離を求める
/// @param segment1 線分1
/// @param segment2 線分2
/// @return 距離
float Distance(const Collision::Segment &segment1, const Collision::Segment &segment2);

/// @brief 球と平面の貫入量を求める関数
/// @param sphere 球
/// @param plane 平面
/// @return 貫入量
float PenetrationDepth(const Collision::Sphere &sphere, const Collision::Plane &plane);

/// @brief 球とAABBの貫入量を求める関数
/// @param sphere 球
/// @param aabb AABB
/// @return 貫入量
float PenetrationDepth(const Collision::Sphere &sphere, const Collision::AABB &aabb);

/// @brief 球とOBBの貫入量を求める関数
/// @param sphere 球
/// @param obb OBB
/// @return 貫入量
float PenetrationDepth(const Collision::Sphere &sphere, const Collision::OBB &obb);

/// @brief カプセルと平面の貫入量を求める関数
/// @param capsule カプセル
/// @param plane 平面
/// @return 貫入量
float PenetrationDepth(const Collision::Capsule &capsule, const Collision::Plane &plane);

/// @brief カプセルとAABBの貫入量を求める関数
/// @param capsule カプセル
/// @param aabb AABB
/// @return 貫入量
float PenetrationDepth(const Collision::Capsule &capsule, const Collision::AABB &aabb);

/// @brief カプセルとOBBの貫入量を求める関数
/// @param capsule カプセル
/// @param obb OBB
/// @return 貫入量
float PenetrationDepth(const Collision::Capsule &capsule, const Collision::OBB &obb);

/// @brief 直線と平面の衝突判定
/// @param line 直線
/// @param plane 平面
/// @return 判定結果
bool IsCollision(const Collision::Line &line, const Collision::Plane &plane);

/// @brief 半直線と平面の衝突判定
/// @param ray 半直線
/// @param plane 平面
/// @return 判定結果
bool IsCollision(const Collision::Ray &ray, const Collision::Plane &plane);

/// @brief 線分と平面の衝突判定
/// @param segment 線分
/// @param plane 平面
/// @return 判定結果
bool IsCollision(const Collision::Segment &segment, const Collision::Plane &plane);

/// @brief 三角形と直線の衝突判定
/// @param triangle 三角形
/// @param line 直線
/// @return 判定結果
bool IsCollision(const Collision::Triangle &triangle, const Collision::Line &line);

/// @brief 三角形と半直線の衝突判定
/// @param triangle 三角形
/// @param ray 半直線
/// @return 判定結果
bool IsCollision(const Collision::Triangle &triangle, const Collision::Ray &ray);

/// @brief 三角形と線分の衝突判定
/// @param triangle 三角形
/// @param segment 線分
/// @return 判定結果
bool IsCollision(const Collision::Triangle &triangle, const Collision::Segment &segment);

/// @brief AABBとAABBの衝突判定
/// @param aabb1 AABB1
/// @param aabb2 AABB2
/// @return 判定結果
bool IsCollision(const Collision::AABB &aabb1, const Collision::AABB &aabb2);

/// @brief AABBと直線の衝突判定
/// @param aabb AABB
/// @param line 直線
/// @return 判定結果
bool IsCollision(const Collision::AABB &aabb, const Collision::Line &line);

/// @brief AABBと半直線の衝突判定
/// @param aabb AABB
/// @param ray 半直線
/// @return 判定結果
bool IsCollision(const Collision::AABB &aabb, const Collision::Ray &ray);

/// @brief AABBと線分の衝突判定
/// @param aabb AABB
/// @param segment 線分
/// @return 判定結果
bool IsCollision(const Collision::AABB &aabb, const Collision::Segment &segment);

/// @brief AABBと点の衝突判定
/// @param aabb AABB
/// @param point 点
/// @return 判定結果
bool IsCollision(const Collision::AABB &aabb, const Vector3 &point);

/// @brief 球と平面の衝突判定
/// @param sphere 球
/// @param plane 平面
/// @return 当たったかどうか
bool IsCollision(const Collision::Sphere &sphere, const Collision::Plane &plane);

/// @brief 球とAABBの衝突判定
/// @param sphere 球
/// @param aabb AABB
/// @return 判定結果
bool IsCollision(const Collision::Sphere &sphere, const Collision::AABB &aabb);

/// @brief 球とOBBの衝突判定
/// @param sphere 球
/// @param obb OBB
/// @return 判定結果
bool IsCollision(const Collision::Sphere &sphere, const Collision::OBB &obb);

/// @brief 球と球の衝突判定
/// @param sphere1 球1
/// @param sphere2 球2
/// @return 判定結果
bool IsCollision(const Collision::Sphere &sphere1, const Collision::Sphere &sphere2);

/// @brief カプセルと平面の衝突判定
/// @param capsule カプセル
/// @param plane 平面
/// @return 判定結果
bool IsCollision(const Collision::Capsule &capsule, const Collision::Plane &plane);

/// @brief カプセルとAABBの衝突判定
/// @param capsule カプセル
/// @param aabb AABB
/// @return 判定結果
bool IsCollision(const Collision::Capsule &capsule, const Collision::AABB &aabb);

/// @brief カプセルとOBBの衝突判定
/// @param capsule カプセル
/// @param obb OBB
/// @return 判定結果
bool IsCollision(const Collision::Capsule &capsule, const Collision::OBB &obb);

/// @brief カプセルとカプセルの衝突判定
/// @param capsule1 カプセル1
/// @param capsule2 カプセル2
/// @return 判定結果
bool IsCollision(const Collision::Capsule &capsule1, const Collision::Capsule &capsule2);