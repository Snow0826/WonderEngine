#pragma once

/// @brief メッシュの種類
enum class MeshType {
	kModel,				// モデル
	kPlane,				// 平面
	kBox,				// 立方体
	kRing,				// 円環
	kCylinder,			// 円柱
	kSkinned,			// スキニングされたメッシュ
	kCountOfMeshType	// メッシュの種類の数
};