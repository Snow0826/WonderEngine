#include "Collision.h"
#include "Matrix4x4.h"
#include <algorithm>

using namespace Collision;

Vector3 GetCenter(const Collision::AABB &aabb) {
	return (aabb.min + aabb.max) * 0.5f;
}

Vector3 GetExtent(const Collision::AABB &aabb) {
	return (aabb.max - aabb.min) * 0.5f;
}

Matrix4x4 MakeOBBMatrix(const Collision::OBB &obb) {
	Matrix4x4 obbMatrix = {
		obb.orientations[0].x, obb.orientations[0].y, obb.orientations[0].z, 0.0f,
		obb.orientations[1].x, obb.orientations[1].y, obb.orientations[1].z, 0.0f,
		obb.orientations[2].x, obb.orientations[2].y, obb.orientations[2].z, 0.0f,
		obb.center.x, obb.center.y, obb.center.z, 1.0f
	};
	return obbMatrix;
}

Vector3 Reflect(const Vector3 &input, const Vector3 &normal) {
	return input - 2.0f * (input.dot(normal)) * normal;
}

Vector3 Project(const Vector3 &v1, const Vector3 &v2) {
	Vector3 v2n = v2.normalized();
	float d = v1.dot(v2n);
	return v2n * d;
}

Vector3 ClosestPoint(const Vector3 &point, const Line &line) {
	float t = (point - line.origin).dot(line.diff) / line.diff.lengthSquare();
	return line.origin + t * line.diff;
}

Vector3 ClosestPoint(const Vector3 &point, const Ray &ray) {
	float t = (point - ray.origin).dot(ray.diff) / ray.diff.lengthSquare();
	t = std::max(t, 0.0f);
	return ray.origin + t * ray.diff;
}

Vector3 ClosestPoint(const Vector3 &point, const Segment &segment) {
	float t = (point - segment.origin).dot(segment.diff) / segment.diff.lengthSquare();
	t = std::clamp(t, 0.0f, 1.0f);
	return segment.origin + t * segment.diff;
}

Vector3 ClosestPoint(const Vector3 &point, const Collision::Plane &plane) {
	return point - Distance(point, plane) * plane.normal;
}

Vector3 ClosestPoint(const Vector3 &point, const Collision::AABB &aabb) {
	Vector3 closestPoint{
		std::clamp(point.x, aabb.min.x, aabb.max.x),
		std::clamp(point.y, aabb.min.y, aabb.max.y),
		std::clamp(point.z, aabb.min.z, aabb.max.z)
	};
	return closestPoint;
}

Vector3 ClosestPoint(const Vector3 &point, const Collision::OBB &obb) {
	Matrix4x4 obbWorldMatrix = MakeOBBMatrix(obb);
	Matrix4x4 inverseOBBWorldMatrix = obbWorldMatrix.inverse();
	Vector3 centerInOBBLocalSpace = point * inverseOBBWorldMatrix;
	AABB aabbInOBBLocalSpace{ .min = {-obb.size}, .max = {obb.size} };
	Vector3 closestPointLocal = ClosestPoint(centerInOBBLocalSpace, aabbInOBBLocalSpace);
	Vector3 closestPointWorld = closestPointLocal * obbWorldMatrix;
	return closestPointWorld;
}

Vector3 ClosestPoint(const Collision::Segment &segment, const Collision::Plane &plane) {
	float dot = plane.normal.dot(segment.diff);
	if (dot == 0.0f) {
		return segment.origin; // 平行
	}
	float t = (plane.distance - plane.normal.dot(segment.origin)) / dot;
	t = std::clamp(t, 0.0f, 1.0f);
	return segment.origin + t * segment.diff;
}

Vector3 ClosestPoint(const Collision::Segment &segment, const Collision::AABB &aabb) {
	Vector3 tMin{
		(aabb.min.x - segment.origin.x) / segment.diff.x,
		(aabb.min.y - segment.origin.y) / segment.diff.y,
		(aabb.min.z - segment.origin.z) / segment.diff.z
	};
	Vector3 tMax{
		(aabb.max.x - segment.origin.x) / segment.diff.x,
		(aabb.max.y - segment.origin.y) / segment.diff.y,
		(aabb.max.z - segment.origin.z) / segment.diff.z
	};
	Vector3 tNear{
		std::min(tMin.x, tMax.x),
		std::min(tMin.y, tMax.y),
		std::min(tMin.z, tMax.z)
	};
	float tmin = std::max(std::max(tNear.x, tNear.y), tNear.z);
	tmin = std::clamp(tmin, 0.0f, 1.0f);
	return segment.origin + tmin * segment.diff;
}

Vector3 ClosestPoint(const Collision::Segment &segment, const Collision::OBB &obb) {
	Matrix4x4 obbWorldMatrix = MakeOBBMatrix(obb);
	Matrix4x4 inverseOBBWorldMatrix = obbWorldMatrix.inverse();
	Vector3 startInOBBLocalSpace = segment.origin * inverseOBBWorldMatrix;
	Vector3 endInOBBLocalSpace = (segment.origin + segment.diff) * inverseOBBWorldMatrix;
	Segment segmentInOBBLocalSpace{ .origin = startInOBBLocalSpace, .diff = endInOBBLocalSpace - startInOBBLocalSpace };
	AABB aabbInOBBLocalSpace{ .min = -obb.size, .max = obb.size };
	Vector3 closestPointLocal = ClosestPoint(segmentInOBBLocalSpace, aabbInOBBLocalSpace);
	Vector3 closestPointWorld = closestPointLocal * obbWorldMatrix;
	return closestPointWorld;
}

Vector3 ClosestPoint(const Collision::Segment &segment1, const Collision::Segment &segment2) {
	Vector3 d1 = segment1.diff;
	Vector3 d2 = segment2.diff;
	Vector3 r = segment1.origin - segment2.origin;
	float a = d1.dot(d1);
	float e = d2.dot(d2);
	float f = d2.dot(r);
	float s, t;
	if (a <= 1e-6f && e <= 1e-6f) {
		s = t = 0.0f;
		return segment1.origin;
	}
	if (a <= 1e-6f) {
		s = 0.0f;
		t = std::clamp(f / e, 0.0f, 1.0f);
	} else {
		float c = d1.dot(r);
		if (e <= 1e-6f) {
			t = 0.0f;
			s = std::clamp(-c / a, 0.0f, 1.0f);
		} else {
			float b = d1.dot(d2);
			float denom = a * e - b * b;
			if (denom != 0.0f) {
				s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
			} else {
				s = 0.0f;
			}
			t = (b * s + f) / e;
			if (t < 0.0f) {
				t = 0.0f;
				s = std::clamp(-c / a, 0.0f, 1.0f);
			} else if (t > 1.0f) {
				t = 1.0f;
				s = std::clamp((b - c) / a, 0.0f, 1.0f);
			}
		}
	}
	return segment1.origin + d1 * s;
}

Vector3 ClosestPoint(const Collision::Sphere &sphere1, const Collision::Sphere &sphere2) {
	Vector3 direction = (sphere2.center - sphere1.center).normalized();
	return sphere1.center + direction * sphere1.radius;
}

Vector3 Normal(const Vector3 &point, const Collision::AABB &aabb) {
	Vector3 closestPoint = ClosestPoint(point, aabb);
	Vector3 normal = closestPoint.normalized(point);
	if (normal.isZero()) {
		Vector3 center = (aabb.min + aabb.max) * 0.5f;
		Vector3 toMin = point - aabb.min;
		Vector3 toMax = aabb.max - point;
		float minDist = std::min({ toMin.x, toMin.y, toMin.z, toMax.x, toMax.y, toMax.z });
		if (minDist == toMin.x) {
			normal = { -1.0f, 0.0f, 0.0f };
		} else if (minDist == toMin.y) {
			normal = { 0.0f, -1.0f, 0.0f };
		} else if (minDist == toMin.z) {
			normal = { 0.0f, 0.0f, -1.0f };
		} else if (minDist == toMax.x) {
			normal = { 1.0f, 0.0f, 0.0f };
		} else if (minDist == toMax.y) {
			normal = { 0.0f, 1.0f, 0.0f };
		} else if (minDist == toMax.z) {
			normal = { 0.0f, 0.0f, 1.0f };
		}
	}
	return normal;
}

float Distance(const Vector3 &point, const Collision::Plane &plane) {
	return std::abs(plane.normal.dot(point) - plane.distance);
}

float Distance(const Vector3 &point, const Collision::AABB &aabb) {
	Vector3 closestPoint = ClosestPoint(point, aabb);
	return closestPoint.distanceFrom(point);
}

float Distance(const Vector3 &point, const Collision::OBB &obb) {
	Vector3 closestPoint = ClosestPoint(point, obb);
	return closestPoint.distanceFrom(point);
}

float Distance(const Collision::Segment &segment, const Collision::Plane &plane) {
	Vector3 closestPoint = ClosestPoint(segment, plane);
	return Distance(closestPoint, plane);
}

float Distance(const Collision::Segment &segment, const Collision::AABB &aabb) {
	Vector3 closestPoint = ClosestPoint(segment, aabb);
	return Distance(closestPoint, aabb);
}

float Distance(const Collision::Segment &segment, const Collision::OBB &obb) {
	Vector3 closestPoint = ClosestPoint(segment, obb);
	return Distance(closestPoint, obb);
}

float Distance(const Collision::Segment &segment1, const Collision::Segment &segment2) {
	//線分同士の最近接点を求める
	Vector3 closestPoint1 = ClosestPoint(segment1, segment2);
	Vector3 closestPoint2 = ClosestPoint(segment2, segment1);
	//最近接点同士の距離を求める
	return closestPoint2.distanceFrom(closestPoint1);
}

float PenetrationDepth(const Collision::Sphere &sphere, const Collision::Plane &plane) {
	return sphere.radius - Distance(sphere.center, plane);
}

float PenetrationDepth(const Collision::Sphere &sphere, const Collision::AABB &aabb) {
	Vector3 closestPoint = ClosestPoint(sphere.center, aabb);
	return sphere.radius - closestPoint.distanceFrom(sphere.center);
}

bool IsCollision(const Line &line, const Plane &plane) {
	float dot = plane.normal.dot(line.diff);
	if (dot == 0.0f) {
		return false; // 平行
	}
	return true;
}

bool IsCollision(const Ray &ray, const Plane &plane) {
	float dot = plane.normal.dot(ray.diff);
	if (dot == 0.0f) {
		return false; // 平行
	}
	float t = (plane.distance - plane.normal.dot(ray.origin)) / dot;
	return t >= 0.0f;
}

bool IsCollision(const Segment &segment, const Plane &plane) {
	float dot = plane.normal.dot(segment.diff);
	if (dot == 0.0f) {
		return false; // 平行
	}
	float t = (plane.distance - plane.normal.dot(segment.origin)) / dot;
	return t >= 0.0f && t <= 1.0f;
}

bool IsCollision(const Triangle &triangle, const Line &line) {
	Plane plane;
	plane.normal = (triangle.vertices[1] - triangle.vertices[0]).cross(triangle.vertices[2] - triangle.vertices[0]);
	plane.normal = plane.normal.normalized();
	plane.distance = plane.normal.dot(triangle.vertices[0]);
	if (IsCollision(line, plane)) {
		Vector3 closestPoint = ClosestPoint(line.origin, line);
		Vector3 edge[3];
		Vector3 cross[3];
		for (uint32_t i = 0; i < 3; ++i) {
			edge[i] = triangle.vertices[(i + 1) % 3] - triangle.vertices[i];
			cross[i] = edge[i].cross(closestPoint - triangle.vertices[i]);
			if (cross[i].dot(plane.normal) < 0.0f) {
				return false;
			}
		}
		return true;
	}
	return false;
}

bool IsCollision(const Triangle &triangle, const Ray &ray) {
	Plane plane;
	plane.normal = (triangle.vertices[1] - triangle.vertices[0]).cross(triangle.vertices[2] - triangle.vertices[0]);
	plane.normal = plane.normal.normalized();
	plane.distance = plane.normal.dot(triangle.vertices[0]);
	if (IsCollision(ray, plane)) {
		Vector3 closestPoint = ClosestPoint(ray.origin, ray);
		Vector3 edge[3];
		Vector3 cross[3];
		for (uint32_t i = 0; i < 3; ++i) {
			edge[i] = triangle.vertices[(i + 1) % 3] - triangle.vertices[i];
			cross[i] = edge[i].cross(closestPoint - triangle.vertices[i]);
			if (cross[i].dot(plane.normal) < 0.0f) {
				return false;
			}
		}
		return true;
	}
	return false;
}

bool IsCollision(const Triangle &triangle, const Segment &segment) {
	Plane plane;
	plane.normal = (triangle.vertices[1] - triangle.vertices[0]).cross(triangle.vertices[2] - triangle.vertices[0]);
	plane.normal = plane.normal.normalized();
	plane.distance = plane.normal.dot(triangle.vertices[0]);
	if (IsCollision(segment, plane)) {
		Vector3 closestPoint = ClosestPoint(segment.origin, segment);
		Vector3 edge[3];
		Vector3 cross[3];
		for (uint32_t i = 0; i < 3; ++i) {
			edge[i] = triangle.vertices[(i + 1) % 3] - triangle.vertices[i];
			cross[i] = edge[i].cross(closestPoint - triangle.vertices[i]);
			if (cross[i].dot(plane.normal) < 0.0f) {
				return false;
			}
		}
		return true;
	}
	return false;
}

bool IsCollision(const AABB &aabb1, const AABB &aabb2) {
	if ((aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) &&
		(aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) &&
		(aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z)) {
		return true;
	}
	return false;
}

bool IsCollision(const AABB &aabb, const Line &line) {
	Vector3 tMin{
		(aabb.min.x - line.origin.x) / line.diff.x,
		(aabb.min.y - line.origin.y) / line.diff.y,
		(aabb.min.z - line.origin.z) / line.diff.z
	};
	Vector3 tMax{
		(aabb.max.x - line.origin.x) / line.diff.x,
		(aabb.max.y - line.origin.y) / line.diff.y,
		(aabb.max.z - line.origin.z) / line.diff.z
	};
	Vector3 tNear{
		std::min(tMin.x, tMax.x),
		std::min(tMin.y, tMax.y),
		std::min(tMin.z, tMax.z)
	};
	Vector3 tFar{
		std::max(tMin.x, tMax.x),
		std::max(tMin.y, tMax.y),
		std::max(tMin.z, tMax.z)
	};
	float tmin = std::max(std::max(tNear.x, tNear.y), tNear.z);
	float tmax = std::min(std::min(tFar.x, tFar.y), tFar.z);
	return tmin <= tmax;
}

bool IsCollision(const AABB &aabb, const Ray &ray) {
	Vector3 tMin{
		(aabb.min.x - ray.origin.x) / ray.diff.x,
		(aabb.min.y - ray.origin.y) / ray.diff.y,
		(aabb.min.z - ray.origin.z) / ray.diff.z
	};
	Vector3 tMax{
		(aabb.max.x - ray.origin.x) / ray.diff.x,
		(aabb.max.y - ray.origin.y) / ray.diff.y,
		(aabb.max.z - ray.origin.z) / ray.diff.z
	};
	Vector3 tNear{
		std::min(tMin.x, tMax.x),
		std::min(tMin.y, tMax.y),
		std::min(tMin.z, tMax.z)
	};
	Vector3 tFar{
		std::max(tMin.x, tMax.x),
		std::max(tMin.y, tMax.y),
		std::max(tMin.z, tMax.z)
	};
	float tmin = std::max(std::max(tNear.x, tNear.y), tNear.z);
	float tmax = std::min(std::min(tFar.x, tFar.y), tFar.z);
	return tmin <= tmax && tmax >= 0.0f;
}

bool IsCollision(const AABB &aabb, const Segment &segment) {
	Vector3 tMin{
		(aabb.min.x - segment.origin.x) / segment.diff.x,
		(aabb.min.y - segment.origin.y) / segment.diff.y,
		(aabb.min.z - segment.origin.z) / segment.diff.z
	};
	Vector3 tMax{
		(aabb.max.x - segment.origin.x) / segment.diff.x,
		(aabb.max.y - segment.origin.y) / segment.diff.y,
		(aabb.max.z - segment.origin.z) / segment.diff.z
	};
	Vector3 tNear{
		std::min(tMin.x, tMax.x),
		std::min(tMin.y, tMax.y),
		std::min(tMin.z, tMax.z)
	};
	Vector3 tFar{
		std::max(tMin.x, tMax.x),
		std::max(tMin.y, tMax.y),
		std::max(tMin.z, tMax.z)
	};
	float tmin = std::max(std::max(tNear.x, tNear.y), tNear.z);
	float tmax = std::min(std::min(tFar.x, tFar.y), tFar.z);
	return tmin <= tmax && tmax >= 0.0f && tmin <= 1.0f;
}

bool IsCollision(const AABB &aabb, const Vector3 &point) {
	return (point.x >= aabb.min.x && point.x <= aabb.max.x)
		&& (point.y >= aabb.min.y && point.y <= aabb.max.y)
		&& (point.z >= aabb.min.z && point.z <= aabb.max.z);
}

bool IsCollision(const Sphere &sphere, const Plane &plane) {
	return Distance(sphere.center, plane) <= sphere.radius;
}

bool IsCollision(const Sphere &sphere, const AABB &aabb) {
	return Distance(sphere.center, aabb) <= sphere.radius;
}

bool IsCollision(const Sphere &sphere, const OBB &obb) {
	return Distance(sphere.center, obb) <= sphere.radius;
}

bool IsCollision(const Sphere &sphere1, const Sphere &sphere2) {
	float distance = sphere1.center.distanceFrom(sphere2.center);
	return distance <= sphere1.radius + sphere2.radius;
}

bool IsCollision(const Capsule &capsule, const Plane &plane) {
	return Distance(capsule.segment, plane) <= capsule.radius;
}

bool IsCollision(const Capsule &capsule, const AABB &aabb) {
	return Distance(capsule.segment, aabb) <= capsule.radius;
}

bool IsCollision(const Capsule &capsule, const OBB &obb) {
	return Distance(capsule.segment, obb) <= capsule.radius;
}

bool IsCollision(const Capsule &capsule1, const Capsule &capsule2) {
	return Distance(capsule1.segment, capsule2.segment) <= capsule1.radius + capsule2.radius;
}