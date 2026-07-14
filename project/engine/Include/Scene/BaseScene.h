#pragma once
#include <array>
#include <memory>
#include <string>

/// @brief カメラの種類
enum class CameraType {
	kMainCamera,		// メインカメラ
	kDebugCamera,		// デバッグカメラ
	kCountOfCameraType	// カメラの種類の数
};

class SceneManager;
class FootprintManager;
class TextManager;
class SpriteManager;
class World;
class DebugRenderer;
class Registry;
class ComponentDrawerRegistry;
class HierarchyWindow;
class InspectorWindow;
class SceneViewWindow;
class GameViewWindow;
class BlendModeInspector;
class ModelInspector;
class SpriteInspector;
class ParticleGroupInspector;
class EmitterSphereInspector;
class FieldInspector;
class TransformInspector;
class UVTransformInspector;
class MaterialInspector;
class CameraInspector;
class DirectionalLightInspector;
class PointLightInspector;
class SpotLightInspector;
class RigidBodyInspector;
class FootprintInspector;
class AABBInspector;
class SphereInspector;
class PlaneInspector;
class OBBInspector;
class CapsuleInspector;
class TransformSystem;
class CameraSystem;
class PhysicalSystem;
class CollisionSystem;
class AnimationSystem;
class AABBRenderSystem;
class SphereRenderSystem;
class PlaneRenderSystem;
class OBBRenderSystem;
class CapsuleRenderSystem;
class FrustumRenderSystem;
class SkeletonRenderSystem;
class BitmapFont;
class Fade;
class Grid;
class DebugCamera;
struct SelectionContext;

/// @brief 基底シーン
class BaseScene {
public:
	/// @brief コンストラクタ
	BaseScene();

	/// @brief デストラクタ
	virtual ~BaseScene();

	/// @brief 初期化
	/// @param sceneManager シーンマネージャー
	void Initialize(SceneManager *sceneManager);

	/// @brief 更新
	void Update();

	/// @brief シーンの初期化処理(派生クラスで実装)
	virtual void OnInitialize() = 0;

	/// @brief シーンの更新処理(派生クラスで実装)
	virtual void OnUpdate() = 0;

	/// @brief シーンの物理更新後処理(派生クラスで実装)
	virtual void OnAfterPhysicalUpdate() {}

	/// @brief シーンのワールド変換後処理(派生クラスで実装)
	virtual void OnAfterTransform() {}

	/// @brief シーンマネージャーを取得する
	/// @return シーンマネージャー
	SceneManager *GetSceneManager() const { return sceneManager_; }

protected:
	using CameraEntities = std::array<uint32_t, static_cast<uint32_t>(CameraType::kCountOfCameraType)>;
	SceneManager *sceneManager_ = nullptr;												// シーンマネージャー
	std::unique_ptr<FootprintManager> footprintManager_ = nullptr;						// フットプリントマネージャー
	std::unique_ptr<TextManager> textManager_ = nullptr;								// テキストマネージャー
	std::unique_ptr<SpriteManager> spriteManager_ = nullptr;							// スプライトマネージャー
	std::unique_ptr<DebugRenderer> debugRenderer_ = nullptr;							// デバッグレンダラー
	std::unique_ptr<Registry> registry_ = nullptr;										// レジストリ
	std::unique_ptr<ComponentDrawerRegistry> componentDrawerRegistry_ = nullptr;		// コンポーネント描画関数レジストリ
	std::unique_ptr<HierarchyWindow> hierarchyWindow_ = nullptr;						// 階層ウィンドウ
	std::unique_ptr<InspectorWindow> inspectorWindow_ = nullptr;						// インスペクタウィンドウ
	std::unique_ptr<SceneViewWindow> sceneViewWindow_ = nullptr;						// シーンビューウィンドウ
	std::unique_ptr<GameViewWindow> gameViewWindow_ = nullptr;							// ゲームビューウィンドウ
	std::unique_ptr<SelectionContext> selection_ = nullptr;								// 選択コンテキスト
	std::unique_ptr<BlendModeInspector> blendModeInspector_ = nullptr;					// ブレンドモードインスペクター
	std::unique_ptr<ModelInspector> modelInspector_ = nullptr;							// モデルインスペクター
	std::unique_ptr<SpriteInspector> spriteInspector_ = nullptr;						// スプライトインスペクター
	std::unique_ptr<ParticleGroupInspector> particleGroupInspector_ = nullptr;			// パーティクルグループインスペクター
	std::unique_ptr<EmitterSphereInspector> emitterSphereInspector_ = nullptr;			// 球状エミッターインスペクター
	std::unique_ptr<FieldInspector> fieldInspector_ = nullptr;							// フィールドインスペクター
	std::unique_ptr<TransformInspector> transformInspector_ = nullptr;					// 変換データインスペクター
	std::unique_ptr<UVTransformInspector> uvTransformInspector_ = nullptr;				// UV変換データインスペクター
	std::unique_ptr<MaterialInspector> materialInspector_ = nullptr;					// マテリアルインスペクター
	std::unique_ptr<CameraInspector> cameraInspector_ = nullptr;						// カメラインスペクター
	std::unique_ptr<DirectionalLightInspector> directionalLightInspector_ = nullptr;	// 平行光源インスペクター
	std::unique_ptr<PointLightInspector> pointLightInspector_ = nullptr;				// 点光源インスペクター
	std::unique_ptr<SpotLightInspector> spotLightInspector_ = nullptr;					// スポットライトインスペクター
	std::unique_ptr<RigidBodyInspector> rigidBodyInspector_ = nullptr;					// 剛体インスペクター
	std::unique_ptr<FootprintInspector> footprintInspector_ = nullptr;					// フットプリントインスペクター
	std::unique_ptr<AABBInspector> aabbInspector_ = nullptr;							// AABBインスペクター
	std::unique_ptr<SphereInspector> sphereInspector_ = nullptr;						// 球インスペクター
	std::unique_ptr<PlaneInspector> planeInspector_ = nullptr;							// 平面インスペクター
	std::unique_ptr<OBBInspector> obbInspector_ = nullptr;								// OBBインスペクター
	std::unique_ptr<CapsuleInspector> capsuleInspector_ = nullptr;						// カプセルインスペクター
	std::unique_ptr<TransformSystem> transformSystem_ = nullptr;						// 変換システム
	std::unique_ptr<CameraSystem> cameraSystem_ = nullptr;								// カメラシステム
	std::unique_ptr<PhysicalSystem> physicalSystem_ = nullptr;							// 物理システム
	std::unique_ptr<CollisionSystem> collisionSystem_ = nullptr;						// 衝突システム
	std::unique_ptr<AnimationSystem> animationSystem_ = nullptr;						// アニメーションシステム
	std::unique_ptr<AABBRenderSystem> aabbRenderSystem_ = nullptr;						// AABBの描画システム
	std::unique_ptr<SphereRenderSystem> sphereRenderSystem_ = nullptr;					// 球の描画システム
	std::unique_ptr<PlaneRenderSystem> planeRenderSystem_ = nullptr;					// 平面の描画システム
	std::unique_ptr<OBBRenderSystem> obbRenderSystem_ = nullptr;						// OBBの描画システム
	std::unique_ptr<CapsuleRenderSystem> capsuleRenderSystem_ = nullptr;				// カプセルの描画システム
	std::unique_ptr<FrustumRenderSystem> frustumRenderSystem_ = nullptr;				// 視錐台の描画システム
	std::unique_ptr<SkeletonRenderSystem> skeletonRenderSystem_ = nullptr;				// スケルトンの描画システム
	std::unique_ptr<BitmapFont> bitmapFont_ = nullptr;									// ビットマップフォント
	std::unique_ptr<Fade> fade_ = nullptr;												// フェード
	std::unique_ptr<Grid> grid_ = nullptr;												// グリッド
	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;								// デバッグカメラ
	bool isDebugCameraActive_ = false;													// デバッグカメラの有効フラグ
	uint32_t directionalLightEntity_ = 0;												// 平行光源のエンティティ
	CameraEntities cameraEntities_{};													// カメラエンティティリスト
	uint32_t mainCameraType_ = static_cast<uint32_t>(CameraType::kMainCamera);			// メインカメラのタイプ
	uint32_t debugCameraType_ = static_cast<uint32_t>(CameraType::kDebugCamera);		// デバッグカメラのタイプ
	static inline constexpr float kDeltaTime = 1.0f / 60.0f;							// デルタタイム
};