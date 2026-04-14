#include "Frustum.h"
#include "EntityComponentSystem.h"
#include "DebugRenderer.h"
#include "Camera.h"
#include "Transform.h"

Frustum MakeFrustum(const ViewProjectionData &viewProjection) {
	Frustum frustumData;
	Matrix4x4 viewProjectionMatrix = viewProjection.view * viewProjection.projection;
	Matrix4x4 transposedVPMatrix = viewProjectionMatrix.transpose();
	Vector4 vRight(transposedVPMatrix.m[0]);
	Vector4 vUp(transposedVPMatrix.m[1]);
	Vector4 vForward(transposedVPMatrix.m[2]);
	Vector4 vPosition(transposedVPMatrix.m[3]);
	frustumData.frustumPlanes[0] = vPosition + vRight;		// Left plane
	frustumData.frustumPlanes[1] = vPosition - vRight;		// Right plane
	frustumData.frustumPlanes[2] = vPosition + vUp;			// Bottom plane
	frustumData.frustumPlanes[3] = vPosition - vUp;			// Top plane
	frustumData.frustumPlanes[4] = vForward;				// Near plane
	frustumData.frustumPlanes[5] = vPosition - vForward;	// Far plane

	const Vector4 corners[] = {
		Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
		Vector4(1.0f, -1.0f, -1.0f, 1.0f),
		Vector4(1.0f, 1.0f, -1.0f, 1.0f),
		Vector4(-1.0f, 1.0f, -1.0f, 1.0f),
		Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
		Vector4(1.0f, -1.0f, 1.0f, 1.0f),
		Vector4(1.0f, 1.0f, 1.0f, 1.0f),
		Vector4(-1.0f, 1.0f, 1.0f, 1.0f)
	};

	Matrix4x4 inverseVPMatrix = viewProjectionMatrix.inverse();
	for (uint32_t i = 0; i < frustumData.frustumCorners.size(); ++i) {
		Vector4 transformedCorner = inverseVPMatrix * corners[i];
		frustumData.frustumCorners[i] = transformedCorner / transformedCorner.w;
	}
	return frustumData;
}

void FrustumRenderSystem::Update() {
#ifdef DRAW_LINE
	registry_->ForEach<Camera, Transform, CullingCamera, FrustumRenderer>([&](uint32_t entity, Camera *camera, Transform *transform, CullingCamera *cullingCamera, FrustumRenderer *frustumRenderer) {
		// ビュー行列とプロジェクション行列の計算
		ViewProjectionData viewProjection = MakeViewProjection(*camera, *transform);

		// 視錐台の平面と頂点の計算
		Frustum frustum = MakeFrustum(viewProjection);

		// 視錐台のラインの更新
		for (int i = 0; i < kMaxLineCount; ++i) {
			if (i < 4) {
				// Near plane lines
				debugRenderer_->AddLine({
						.start = {
							.x = frustum.frustumCorners[i].x,
							.y = frustum.frustumCorners[i].y,
							.z = frustum.frustumCorners[i].z
						},
						.end = {
							.x = frustum.frustumCorners[(i + 1) % 4].x,
							.y = frustum.frustumCorners[(i + 1) % 4].y,
							.z = frustum.frustumCorners[(i + 1) % 4].z
						},
						.color = { 0.0f, 0.0f, 1.0f, 1.0f }
					});
			} else if (i < 8) {
				// Far plane lines
				debugRenderer_->AddLine({
						.start = {
							.x = frustum.frustumCorners[i].x,
							.y = frustum.frustumCorners[i].y,
							.z = frustum.frustumCorners[i].z
						},
						.end = {
							.x = frustum.frustumCorners[(i + 1) % 4 + 4].x,
							.y = frustum.frustumCorners[(i + 1) % 4 + 4].y,
							.z = frustum.frustumCorners[(i + 1) % 4 + 4].z
						},
						.color = { 0.0f, 0.0f, 1.0f, 1.0f }
					});
			} else {
				// Side lines
				debugRenderer_->AddLine({
						.start = {
							.x = frustum.frustumCorners[i - 8].x,
							.y = frustum.frustumCorners[i - 8].y,
							.z = frustum.frustumCorners[i - 8].z
						},
						.end = {
							.x = frustum.frustumCorners[i - 4].x,
							.y = frustum.frustumCorners[i - 4].y,
							.z = frustum.frustumCorners[i - 4].z
						},
						.color = { 0.0f, 0.0f, 1.0f, 1.0f }
					});
			}
		}
		}, exclude<Disabled>());
#endif // DRAW_LINE
}