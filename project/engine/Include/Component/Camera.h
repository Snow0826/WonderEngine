#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"
#include <numbers>

/// @brief カメラ
struct Camera final {
	float fovY = std::numbers::pi_v<float> / 4.0f;	// 垂直方向の視野角（ラジアン）
	float aspectRatio = 16.0f / 9.0f;				// アスペクト比（横幅/縦幅）
	float nearZ = 0.1f;								// ニアクリップ面の距離
	float farZ = 1024.0f;							// ファークリップ面の距離
};

/// @brief レンダリングカメラ
struct RenderingCamera final {};

/// @brief カリングカメラ
struct CullingCamera final {};

/// @brief ビュープロジェクションデータ
struct ViewProjectionData final {
	Matrix4x4 view;			// ビュー行列
	Matrix4x4 projection;	// 射影行列
};

/// @brief カメラデータ(GPU)
struct CameraForGPU final {
	Vector3 worldPosition;	// ワールド座標
};

struct Transform;

/// @brief ビュープロジェクションデータの作成
/// @param camera カメラ
/// @param transform 変換データ
/// @return ビュープロジェクションデータ
ViewProjectionData MakeViewProjection(const Camera &camera, const Transform &transform);

class Registry;

/// @brief カメラシステム
class CameraSystem final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	CameraSystem(Registry *registry) : registry_(registry) {}

	/// @brief カメラの切り替え
	/// @param cameraEntity カメラエンティティ
	void SwitchCamera(uint32_t cameraEntity);

	/// @brief レンダリングカメラの切り替え
	/// @param cameraEntity カメラエンティティ
	void SwitchRenderingCamera(uint32_t cameraEntity);

	/// @brief カリングカメラエンティティの取得
	/// @return カリングカメラエンティティ
	uint32_t GetCullingCameraEntity() const;

private:
	Registry *registry_ = nullptr;	// レジストリ
};

/// @brief カメラインスペクター
class CameraInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	CameraInspector(Registry *registry) : registry_(registry) {}

	/// @brief カメラインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;	// レジストリ
};