#include "BaseScene.h"
#include "SceneManager.h"
#include "Input.h"
#include "ComponentDrawerRegistry.h"
#include "HierarchyWindow.h"
#include "InspectorWindow.h"
#include "SceneViewWindow.h"
#include "GameViewWindow.h"
#include "SelectionContext.h"
#include "Window.h"
#include "Device.h"
#include "Renderer.h"
#include "World.h"
#include "Resource.h"
#include "DebugRenderer.h"
#include "Texture.h"
#include "SkinCluster.h"
#include "Material.h"
#include "UVTransform.h"
#include "Model.h"
#include "Particle.h"
#include "Sprite.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "RigidBody.h"
#include "Footprint.h"
#include "CollisionSystem.h"
#include "AABBRenderer.h"
#include "SphereRenderer.h"
#include "PlaneRenderer.h"
#include "OBBRenderer.h"
#include "CapsuleRenderer.h"
#include "FrustumRenderer.h"
#include "SkeletonRenderer.h"
#include "Text.h"
#include "BitmapFont.h"
#include "Fade.h"
#include "Grid.h"
#include "DebugCamera.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

BaseScene::BaseScene() = default;
BaseScene::~BaseScene() = default;

void BaseScene::Initialize(SceneManager *sceneManager) {
	assert(sceneManager);
	sceneManager_ = sceneManager;
	Device *device = sceneManager_->GetDevice();
	Input *input = sceneManager_->GetInput();
	Renderer *renderer = sceneManager_->GetRenderer();
	World *world = sceneManager_->GetWorld();
	TextureManager *textureManager = sceneManager_->GetTextureManager();
	MeshManager *meshManager = sceneManager_->GetMeshManager();
	SkinClusterManager *skinClusterManager = sceneManager_->GetSkinClusterManager();
	ModelManager *modelManager = sceneManager_->GetModelManager();
	ParticleManager *particleManager = sceneManager_->GetParticleManager();

	// レジストリの生成と初期化
	registry_ = std::make_unique<Registry>();
	renderer->SetRegistry(registry_.get());
	world->SetRegistry(registry_.get());
	skinClusterManager->SetRegistry(registry_.get());
	particleManager->SetRegistry(registry_.get());

	// デバッグレンダラーの生成
	debugRenderer_ = std::make_unique<DebugRenderer>(world);
	renderer->SetDebugRenderer(debugRenderer_.get());

	// フットプリントマネージャーの生成
	footprintManager_ = std::make_unique<FootprintManager>(registry_.get());
	renderer->SetFootprintManager(footprintManager_.get());

	// スプライトマネージャーの生成
	spriteManager_ = std::make_unique<SpriteManager>(textureManager, meshManager, registry_.get());

	// テキストマネージャーの生成
	textManager_ = std::make_unique<TextManager>(registry_.get(), spriteManager_.get());

	// システムの生成
	transformSystem_ = std::make_unique<TransformSystem>(registry_.get());
	cameraSystem_ = std::make_unique<CameraSystem>(registry_.get());
	physicalSystem_ = std::make_unique<PhysicalSystem>(registry_.get());
	collisionSystem_ = std::make_unique<CollisionSystem>(registry_.get());
	animationSystem_ = std::make_unique<AnimationSystem>(registry_.get());
	aabbRenderSystem_ = std::make_unique<AABBRenderSystem>(registry_.get(), debugRenderer_.get());
	sphereRenderSystem_ = std::make_unique<SphereRenderSystem>(registry_.get(), debugRenderer_.get());
	planeRenderSystem_ = std::make_unique<PlaneRenderSystem>(registry_.get(), debugRenderer_.get());
	obbRenderSystem_ = std::make_unique<OBBRenderSystem>(registry_.get(), debugRenderer_.get());
	capsuleRenderSystem_ = std::make_unique<CapsuleRenderSystem>(registry_.get(), debugRenderer_.get());
	frustumRenderSystem_ = std::make_unique<FrustumRenderSystem>(registry_.get(), debugRenderer_.get());
	skeletonRenderSystem_ = std::make_unique<SkeletonRenderSystem>(registry_.get(), debugRenderer_.get());

	// インスペクターの生成
	blendModeInspector_ = std::make_unique<BlendModeInspector>(registry_.get());
	modelInspector_ = std::make_unique<ModelInspector>(registry_.get(), modelManager);
	spriteInspector_ = std::make_unique<SpriteInspector>(registry_.get(), textureManager);
	particleGroupInspector_ = std::make_unique<ParticleGroupInspector>(registry_.get(), textureManager);
	emitterSphereInspector_ = std::make_unique<EmitterSphereInspector>(registry_.get());
	fieldInspector_ = std::make_unique<FieldInspector>(registry_.get());
	transformInspector_ = std::make_unique<TransformInspector>(registry_.get());
	uvTransformInspector_ = std::make_unique<UVTransformInspector>(registry_.get());
	materialInspector_ = std::make_unique<MaterialInspector>(registry_.get());
	cameraInspector_ = std::make_unique<CameraInspector>(registry_.get());
	directionalLightInspector_ = std::make_unique<DirectionalLightInspector>(registry_.get());
	pointLightInspector_ = std::make_unique<PointLightInspector>(registry_.get());
	spotLightInspector_ = std::make_unique<SpotLightInspector>(registry_.get());
	rigidBodyInspector_ = std::make_unique<RigidBodyInspector>(registry_.get());
	footprintInspector_ = std::make_unique<FootprintInspector>(registry_.get());
	aabbInspector_ = std::make_unique<AABBInspector>(registry_.get());
	sphereInspector_ = std::make_unique<SphereInspector>(registry_.get());
	planeInspector_ = std::make_unique<PlaneInspector>(registry_.get());
	obbInspector_ = std::make_unique<OBBInspector>(registry_.get());
	capsuleInspector_ = std::make_unique<CapsuleInspector>(registry_.get());

	// インスペクターの登録
	componentDrawerRegistry_ = std::make_unique<ComponentDrawerRegistry>(registry_.get());
	componentDrawerRegistry_->RegisterComponentDrawer<BlendMode>([this](uint32_t entity) { blendModeInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<Model>([this](uint32_t entity) { modelInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<Sprite>([this](uint32_t entity) { spriteInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<ParticleGroup>([this](uint32_t entity) { particleGroupInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<EmitterSphere>([this](uint32_t entity) { emitterSphereInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<Field>([this](uint32_t entity) { fieldInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<EulerTransform>([this](uint32_t entity) { transformInspector_->DrawEulerTransform(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<QuaternionTransform>([this](uint32_t entity) { transformInspector_->DrawQuaternionTransform(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<UVTransform>([this](uint32_t entity) { uvTransformInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<Material>([this](uint32_t entity) { materialInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<Camera>([this](uint32_t entity) { cameraInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<DirectionalLight>([this](uint32_t entity) { directionalLightInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<PointLight>([this](uint32_t entity) { pointLightInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<SpotLight>([this](uint32_t entity) { spotLightInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<RigidBody>([this](uint32_t entity) { rigidBodyInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<Footprint>([this](uint32_t entity) { footprintInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<Collision::AABB>([this](uint32_t entity) { aabbInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<Collision::Sphere>([this](uint32_t entity) { sphereInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<Collision::Plane>([this](uint32_t entity) { planeInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<Collision::OBB>([this](uint32_t entity) { obbInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterComponentDrawer<Collision::Capsule>([this](uint32_t entity) { capsuleInspector_->Draw(entity); });
	componentDrawerRegistry_->RegisterTagComponent<MainCamera>("MainCamera");
	componentDrawerRegistry_->RegisterTagComponent<RenderingCamera>("RenderingCamera");
	componentDrawerRegistry_->RegisterTagComponent<UseCulling>("UseCulling");
	componentDrawerRegistry_->RegisterTagComponent<DirtyTransform>("DirtyTransform");
	componentDrawerRegistry_->RegisterTagComponent<DirtyMaterial>("DirtyMaterial");
	componentDrawerRegistry_->RegisterTagComponent<DirtyTextureData>("DirtyTextureData");
	componentDrawerRegistry_->RegisterTagComponent<NoCollision>("NoCollision");
	componentDrawerRegistry_->RegisterTagComponent<AABBRenderer>("AABBRenderer");
	componentDrawerRegistry_->RegisterTagComponent<SphereRenderer>("SphereRenderer");
	componentDrawerRegistry_->RegisterTagComponent<PlaneRenderer>("PlaneRenderer");
	componentDrawerRegistry_->RegisterTagComponent<OBBRenderer>("OBBRenderer");
	componentDrawerRegistry_->RegisterTagComponent<CapsuleRenderer>("CapsuleRenderer");
	componentDrawerRegistry_->RegisterTagComponent<FrustumRenderer>("FrustumRenderer");
	componentDrawerRegistry_->RegisterTagComponent<SkeletonRenderer>("SkeletonRenderer");

	// 選択テキストの生成
	selection_ = std::make_unique<SelectionContext>();

	// 階層ウィンドウの生成
	hierarchyWindow_ = std::make_unique<HierarchyWindow>(registry_.get(), selection_.get());

	// インスペクタウィンドウの生成
	inspectorWindow_ = std::make_unique<InspectorWindow>(registry_.get(), componentDrawerRegistry_.get(), selection_.get());

	// シーンビューウィンドウの生成
	sceneViewWindow_ = std::make_unique<SceneViewWindow>(registry_.get(), device, world, selection_.get());

	// ゲームビューウィンドウの生成
	gameViewWindow_ = std::make_unique<GameViewWindow>(registry_.get(), device, world);

	// メインカメラの生成と初期化
	cameraEntities_[mainCameraType_] = registry_->GenerateEntity();
	registry_->AddComponent(cameraEntities_[mainCameraType_], Camera{});
	registry_->AddComponent(cameraEntities_[mainCameraType_], QuaternionTransform{ .translate = {.y = 5.0f, .z = -10.0f }, .rotateMatrix = LookAt({.y = 5.0f, .z = -10.0f }, {}, {.y = 1.0f }) });
	registry_->AddComponent(cameraEntities_[mainCameraType_], Relationship{});
	registry_->AddComponent(cameraEntities_[mainCameraType_], RenderingCamera{});
	registry_->AddComponent(cameraEntities_[mainCameraType_], MainCamera{});
	registry_->AddComponent(cameraEntities_[mainCameraType_], FrustumRenderer{});
	transformSystem_->MarkDirty(cameraEntities_[mainCameraType_]);

	// デバッグカメラの生成と初期化
	cameraEntities_[debugCameraType_] = registry_->GenerateEntity();
	registry_->AddComponent(cameraEntities_[debugCameraType_], Camera{});
	registry_->AddComponent(cameraEntities_[debugCameraType_], QuaternionTransform{ .translate = {.y = 5.0f, .z = -10.0f }, .rotateMatrix = LookAt({.y = 5.0f, .z = -10.0f }, {}, {.y = 1.0f }) });
	registry_->AddComponent(cameraEntities_[debugCameraType_], Relationship{});
	transformSystem_->MarkDirty(cameraEntities_[debugCameraType_]);

	// 平行光源の生成と初期化
	directionalLightEntity_ = registry_->GenerateEntity();
	registry_->AddComponent(directionalLightEntity_, DirectionalLight{});
	registry_->AddComponent(directionalLightEntity_, Relationship{});

	// ビットマップフォントの初期化
	bitmapFont_ = std::make_unique<BitmapFont>(registry_.get(), spriteManager_.get());

	// フェードの初期化
	fade_ = std::make_unique<Fade>(registry_.get(), spriteManager_.get());

	// グリッドの生成
	grid_ = std::make_unique<Grid>(debugRenderer_.get());
	grid_->Initialize();

	// デバッグカメラの生成と初期化
	debugCamera_ = std::make_unique<DebugCamera>(registry_.get(), input);
	debugCamera_->Initialize(cameraEntities_[debugCameraType_]);

	// 派生クラスの初期化処理の呼び出し
	OnInitialize();

	// パーティクルの初期化
	renderer->InitializeParticle();
}

void BaseScene::Update() {
	Renderer *renderer = sceneManager_->GetRenderer();
	World *world = sceneManager_->GetWorld();
	SkinClusterManager *skinClusterManager = sceneManager_->GetSkinClusterManager();
	ParticleManager *particleManager = sceneManager_->GetParticleManager();

#ifdef USE_IMGUI
	// フレームレートの表示
	ImGui::Text("Framerate: %6.2f fps", ImGui::GetIO().Framerate);

	// ワールドの編集
	world->Edit();

	// ウィンドウの表示切り替え
	ImGui::Checkbox("Hierarchy", &hierarchyWindow_->IsOpen());
	ImGui::Checkbox("Inspector", &inspectorWindow_->IsOpen());
	ImGui::Checkbox("SceneView", &sceneViewWindow_->IsOpen());
	ImGui::Checkbox("GameView", &gameViewWindow_->IsOpen());
	renderer->SetSceneViewVisible(sceneViewWindow_->IsOpen());
	renderer->SetGameViewVisible(gameViewWindow_->IsOpen());

	// グリッドの編集
	grid_->Edit();

	// デバッグカメラの切り替え
	if (ImGui::Checkbox("DebugCameraActive", &isDebugCameraActive_)) {
		if (isDebugCameraActive_) {
			debugCamera_->Reset();
			cameraSystem_->SwitchRenderingCamera(cameraEntities_[debugCameraType_]);
		} else {
			cameraSystem_->SwitchRenderingCamera(cameraEntities_[mainCameraType_]);
		}
	}

	// デバッグカメラの編集
	debugCamera_->Edit("DebugCamera");

	// フットプリント数のデバッグ表示
	footprintManager_->Debug();

	// ライン数のデバッグ表示
	debugRenderer_->Debug();

	// 階層ウィンドウの描画
	hierarchyWindow_->Draw();

	// インスペクタウィンドウの描画
	inspectorWindow_->Draw();

	// シーンビューウィンドウの描画
	sceneViewWindow_->Draw();

	// ゲームビューウィンドウの描画
	gameViewWindow_->Draw();
#endif // USE_IMGUI

	// デバッグレンダラーのフレーム開始
	debugRenderer_->BeginFrame();

	// グリッドの描画
	grid_->Update();

	// デバッグカメラの更新
	if (isDebugCameraActive_) {
		debugCamera_->Update();
	}

	// 派生クラスの更新処理の呼び出し
	OnUpdate();

	// スプライトの更新
	spriteManager_->UpdateSprite();

	// アニメーションの更新
	animationSystem_->Update(kDeltaTime);

	// 物理システムの更新
	physicalSystem_->Update(kDeltaTime);

	// ワールド行列の更新
	transformSystem_->Update();

	// コリジョンシステムの更新
	collisionSystem_->Update();

	// スキンクラスターの更新
	skinClusterManager->Update();

	// 球状エミッターの更新
	particleManager->UpdateEmitterSphere(kDeltaTime);

	// ワールド変換後処理の呼び出し
	OnAfterTransform();

	// 球のデバッグ描画の更新
	sphereRenderSystem_->Update();

	// AABBのデバッグ描画の更新
	aabbRenderSystem_->Update();

	// 平面のデバッグ描画の更新
	planeRenderSystem_->Update();

	// OBBのデバッグ描画の更新
	obbRenderSystem_->UpdateOrientations();
	obbRenderSystem_->Update();

	// カプセルのデバッグ描画の更新
	capsuleRenderSystem_->Update();

	// 視錐台のデバッグ描画の更新
	frustumRenderSystem_->Update();

	// スケルトンのデバッグ描画の更新
	skeletonRenderSystem_->Update();

	// ワールドの更新
	world->Update();

	// パーティクルの発生
	renderer->EmitParticle();

	// デバッグレンダラーのフレーム終了
	debugRenderer_->EndFrame();
}