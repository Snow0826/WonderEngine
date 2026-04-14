#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "MathUtility.h"

/// @brief 頂点データ
struct VertexData final {
	Vector4 position;	// 頂点座標
	Vector2 texcoord;	// テクスチャ座標
	Vector3 normal;		// 法線

	/// @brief 等価演算子
	/// @param other 比較対象の頂点データ
	/// @return 等価ならtrue、そうでなければfalse
	bool operator==(const VertexData &other) const {
		return NearlyEqual(position.x, other.position.x)
			&& NearlyEqual(position.y, other.position.y)
			&& NearlyEqual(position.z, other.position.z)
			&& NearlyEqual(position.w, other.position.w)
			&& NearlyEqual(texcoord.x, other.texcoord.x)
			&& NearlyEqual(texcoord.y, other.texcoord.y)
			&& NearlyEqual(normal.x, other.normal.x)
			&& NearlyEqual(normal.y, other.normal.y)
			&& NearlyEqual(normal.z, other.normal.z);
	}
};

struct VertexDataHash final {
	size_t operator()(const VertexData &vertex) const {
		size_t h1 = std::hash<int32_t>{}(Quantize(vertex.position.x));
		size_t h2 = std::hash<int32_t>{}(Quantize(vertex.position.y));
		size_t h3 = std::hash<int32_t>{}(Quantize(vertex.position.z));
		size_t h4 = std::hash<int32_t>{}(Quantize(vertex.position.w));
		size_t h5 = std::hash<int32_t>{}(Quantize(vertex.texcoord.x));
		size_t h6 = std::hash<int32_t>{}(Quantize(vertex.texcoord.y));
		size_t h7 = std::hash<int32_t>{}(Quantize(vertex.normal.x));
		size_t h8 = std::hash<int32_t>{}(Quantize(vertex.normal.y));
		size_t h9 = std::hash<int32_t>{}(Quantize(vertex.normal.z));

		// Combine the hash values
		size_t hash = h1;
		hash ^= h2 + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= h3 + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= h4 + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= h5 + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= h6 + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= h7 + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= h8 + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= h9 + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		return hash;
	}
};