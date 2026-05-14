#include "BaseScene.h"
#include "SceneManager.h"
#include "Input.h"
#include "EntityComponentSystem.h"
#include "Renderer.h"
#include "World.h"
#include "Resource.h"
#include "DebugRenderer.h"
#include "IndirectCommand.h"
#include "Object.h"
#include "Texture.h"
#include "UVTransform.h"
#include "Material.h"
#include "Model.h"
#include "Particle.h"
#include "Sprite.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "RigidBody.h"
#include "Footprint.h"
#include "AABB.h"
#include "Sphere.h"
#include "Plane.h"
#include "OBB.h"
#include "Capsule.h"
#include "Frustum.h"
#include "Text.h"
#include "BitmapFont.h"
#include "Fade.h"
#include "Grid.h"
#include "DebugCamera.h"

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
	ModelManager *modelManager = sceneManager_->GetModelManager();
	ParticleManager *particleManager = sceneManager_->GetParticleManager();

	// レジストリの生成と初期化
	registry_ = std::make_unique<Registry>();
	renderer->SetRegistry(registry_.get());
	world->SetRegistry(registry_.get());
	particleManager->SetRegistry(registry_.get());

	// デバッグレンダラーの生成
	debugRenderer_ = std::make_unique<DebugRenderer>(world);
	renderer->SetDebugRenderer(debugRenderer_.get());

	// 間接コマンドマネージャーの生成
	indirectCommandManager_ = std::make_unique<IndirectCommandManager>(registry_.get(), world, meshManager);
	renderer->SetIndirectCommandManager(indirectCommandManager_.get());

	// フットプリントマネージャーの生成
	footprintManager_ = std::make_unique<FootprintManager>(registry_.get());
	renderer->SetFootprintManager(footprintManager_.get());

	// スプライトマネージャーの生成
	spriteManager_ = std::make_unique<SpriteManager>(textureManager, meshManager, registry_.get());

	// オブジェクトマネージャーの生成
	objectManager_ = std::make_unique<ObjectManager>(registry_.get());

	// テキストマネージャーの生成
	textManager_ = std::make_unique<TextManager>(registry_.get(), spriteManager_.get(), objectManager_.get());

	// システムの生成
	transformSystem_ = std::make_unique<TransformSystem>(registry_.get());
	cameraSystem_ = std::make_unique<CameraSystem>(registry_.get());
	physicalSystem_ = std::make_unique<PhysicalSystem>(registry_.get());
	animationSystem_ = std::make_unique<AnimationSystem>(registry_.get());
	aabbRenderSystem_ = std::make_unique<AABBRenderSystem>(registry_.get(), debugRenderer_.get());
	sphereRenderSystem_ = std::make_unique<SphereRenderSystem>(registry_.get(), debugRenderer_.get());
	planeRenderSystem_ = std::make_unique<PlaneRenderSystem>(registry_.get(), debugRenderer_.get());
	obbRenderSystem_ = std::make_unique<OBBRenderSystem>(registry_.get(), debugRenderer_.get());
	capsuleRenderSystem_ = std::make_unique<CapsuleRenderSystem>(registry_.get(), debugRenderer_.get());
	frustumRenderSystem_ = std::make_unique<FrustumRenderSystem>(registry_.get(), debugRenderer_.get());

	// インスペクターレジストリの生成
	inspectorRegistry_ = std::make_unique<InspectorRegistry>(registry_.get());

	// インスペクターの生成
	blendModeInspector_ = std::make_unique<BlendModeInspector>(registry_.get(), indirectCommandManager_.get());
	modelInspector_ = std::make_unique<ModelInspector>(registry_.get(), modelManager, indirectCommandManager_.get());
	spriteInspector_ = std::make_unique<SpriteInspector>(registry_.get(), textureManager);
	particleGroupInspector_ = std::make_unique<ParticleGroupInspector>(registry_.get(), textureManager);
	emitterInspector_ = std::make_unique<EmitterInspector>(registry_.get());
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
	inspectorRegistry_->RegisterInspector<BlendMode>([this](uint32_t entity) { blendModeInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<Model>([this](uint32_t entity) { modelInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<Sprite>([this](uint32_t entity) { spriteInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<ParticleGroup>([this](uint32_t entity) { particleGroupInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<Emitter>([this](uint32_t entity) { emitterInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<Field>([this](uint32_t entity) { fieldInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<Transform>([this](uint32_t entity) { transformInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<UVTransform>([this](uint32_t entity) { uvTransformInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<Material>([this](uint32_t entity) { materialInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<Camera>([this](uint32_t entity) { cameraInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<DirectionalLight>([this](uint32_t entity) { directionalLightInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<PointLight>([this](uint32_t entity) { pointLightInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<SpotLight>([this](uint32_t entity) { spotLightInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<RigidBody>([this](uint32_t entity) { rigidBodyInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<Footprint>([this](uint32_t entity) { footprintInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<Collision::AABB>([this](uint32_t entity) { aabbInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<Collision::Sphere>([this](uint32_t entity) { sphereInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<Collision::Plane>([this](uint32_t entity) { planeInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<Collision::OBB>([this](uint32_t entity) { obbInspector_->Draw(entity); });
	inspectorRegistry_->RegisterInspector<Collision::Capsule>([this](uint32_t entity) { capsuleInspector_->Draw(entity); });
	inspectorRegistry_->RegisterTagInspector<CullingCamera>("CullingCamera");
	inspectorRegistry_->RegisterTagInspector<RenderingCamera>("RenderingCamera");
	inspectorRegistry_->RegisterTagInspector<UseCulling>("UseCulling");
	inspectorRegistry_->RegisterTagInspector<HasParent>("HasParent");
	inspectorRegistry_->RegisterTagInspector<DirtyTransform>("DirtyTransform");
	inspectorRegistry_->RegisterTagInspector<DirtyMaterial>("DirtyMaterial");
	inspectorRegistry_->RegisterTagInspector<AABBRenderer>("AABBRenderer");
	inspectorRegistry_->RegisterTagInspector<SphereRenderer>("SphereRenderer");
	inspectorRegistry_->RegisterTagInspector<PlaneRenderer>("PlaneRenderer");
	inspectorRegistry_->RegisterTagInspector<OBBRenderer>("OBBRenderer");
	inspectorRegistry_->RegisterTagInspector<CapsuleRenderer>("CapsuleRenderer");
	inspectorRegistry_->RegisterTagInspector<FrustumRenderer>("FrustumRenderer");

	// メインカメラの生成と初期化
	cameraEntities_[mainCameraType_] = registry_->GenerateEntity();
	registry_->AddComponent(cameraEntities_[mainCameraType_], Camera{});
	registry_->AddComponent(cameraEntities_[mainCameraType_], Transform{ .translate = { .y = 5.0f, .z = -10.0f }, .rotateMatrix = LookAt({ .y = 5.0f, .z = -10.0f }, {}, { .y = 1.0f }) });
	registry_->AddComponent(cameraEntities_[mainCameraType_], RenderingCamera{});
	registry_->AddComponent(cameraEntities_[mainCameraType_], CullingCamera{});
	registry_->AddComponent(cameraEntities_[mainCameraType_], FrustumRenderer{});
	transformSystem_->MarkDirty(cameraEntities_[mainCameraType_]);

	// セカンダリカメラの生成と初期化
	cameraEntities_[secondaryCameraType_] = registry_->GenerateEntity();
	registry_->AddComponent(cameraEntities_[secondaryCameraType_], Camera{});
	registry_->AddComponent(cameraEntities_[secondaryCameraType_], Transform{ .translate = { .y = 5.0f, .z = -10.0f }, .rotateMatrix = LookAt({ .y = 5.0f, .z = -10.0f }, {}, { .y = 1.0f }) });
	transformSystem_->MarkDirty(cameraEntities_[secondaryCameraType_]);

	// 平行光源の生成と初期化
	directionalLightEntity_ = registry_->GenerateEntity();
	registry_->AddComponent(directionalLightEntity_, DirectionalLight{});

	// ビットマップフォントの初期化
	bitmapFont_ = std::make_unique<BitmapFont>(registry_.get(), spriteManager_.get(), objectManager_.get());

	// フェードの初期化
	fade_ = std::make_unique<Fade>(registry_.get(), spriteManager_.get(), objectManager_.get());

	// グリッドの生成
	grid_ = std::make_unique<Grid>(debugRenderer_.get());
	grid_->Initialize();

	// デバッグカメラの生成と初期化
	debugCamera_ = std::make_unique<DebugCamera>(registry_.get(), input);
	debugCamera_->Initialize(cameraEntities_[secondaryCameraType_]);

	// 派生クラスの初期化処理の呼び出し
	OnInitialize();
}

void BaseScene::Update() {
	World *world = sceneManager_->GetWorld();
	ParticleManager *particleManager = sceneManager_->GetParticleManager();

#ifdef USE_IMGUI
	// フレームレートの表示
	ImGui::Text("Framerate: %6.2f fps", ImGui::GetIO().Framerate);

	// ワールドの編集
	world->Edit();

	// グリッドの編集
	grid_->Edit();

	// デバッグカメラの切り替え
	if (ImGui::Checkbox("DebugCameraActive", &isDebugCameraActive_)) {
		if (isDebugCameraActive_) {
			debugCamera_->Reset();
			cameraSystem_->SwitchRenderingCamera(cameraEntities_[secondaryCameraType_]);
			cullingCameraEntity_ = cameraSystem_->GetCullingCameraEntity();
		} else {
			cameraSystem_->SwitchRenderingCamera(cullingCameraEntity_);
		}
	}

	// デバッグカメラの編集
	debugCamera_->Edit("DebugCamera");

	// 間接コマンド数のデバッグ表示
	indirectCommandManager_->Debug();

	// オブジェクト数のデバッグ表示
	objectManager_->Debug();

	// フットプリント数のデバッグ表示
	footprintManager_->Debug();

	// ライン数のデバッグ表示
	debugRenderer_->Debug();

	// インスペクターレジストリによるエンティティの描画
	inspectorRegistry_->DrawEntities();
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
	transformSystem_->UpdateWorldMatrix();

	// カリングデータの更新
	indirectCommandManager_->UpdateCullingData();

	// パーティクルの更新
	particleManager->UpdateParticle();

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

	// ワールドの更新
	world->Update();

	// デバッグレンダラーのフレーム終了
	debugRenderer_->EndFrame();
}