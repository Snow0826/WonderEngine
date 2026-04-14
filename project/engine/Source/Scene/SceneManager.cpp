#include "SceneManager.h"
#include "SampleScene.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "Renderer.h"
#include "World.h"
#include "Audio.h"
#include "Texture.h"
#include "Model.h"
#include "Particle.h"
#include <cassert>

SceneManager::SceneManager() = default;
SceneManager::~SceneManager() {
	delete currentScene_;
	currentScene_ = nullptr;
}

void SceneManager::Initialize(Device *device, Input *input, Audio *audio, Renderer *renderer, std::ofstream *logStream) {
	// デバイスの設定
	assert(device);
	device_ = device;

	// 入力の設定
	assert(input);
	input_ = input;

	// オーディオの設定
	assert(audio);
	audio_ = audio;

	// レンダラーの設定
	assert(renderer);
	renderer_ = renderer;

	// ログストリームの設定
	assert(logStream);
	logStream_ = logStream;

	// ワールドの生成
	world_ = std::make_unique<World>(device_, *logStream_);
	renderer_->SetWorld(world_.get());

	// 音声データの読み込み
	soundHandle_.bgmHandleMap_[BGMType::Title] = audio_->Load("titleBGM.mp3");
	soundHandle_.bgmHandleMap_[BGMType::Game] = audio_->Load("gameBGM.mp3");
	soundHandle_.seHandleMap_[SEType::Start] = audio_->Load("startSE.mp3");
	soundHandle_.seHandleMap_[SEType::Decide] = audio_->Load("decideSE.mp3");
	soundHandle_.seHandleMap_[SEType::Jump] = audio_->Load("jumpSE.mp3");
	soundHandle_.seHandleMap_[SEType::OnGround] = audio_->Load("onGroundSE.mp3");
	soundHandle_.seHandleMap_[SEType::Attack] = audio_->Load("attackSE.mp3");
	soundHandle_.seHandleMap_[SEType::Dash] = audio_->Load("dashSE.mp3");

	// テクスチャの読み込み
	textureManager_ = std::make_unique<TextureManager>(device_, logStream_);
	textureManager_->LoadTexture("white8x8.png");
	textureManager_->LoadTexture("debugfont.png");
	textureManager_->LoadTexture("win.png");
	textureManager_->LoadTexture("lose.png");
	textureManager_->LoadTexture("UI.png");
	textureManager_->LoadTexture("T_X_X_Color_3D.png");
	textureManager_->LoadTexture("T_X_A_Color_3D.png");
	textureManager_->LoadTexture("T_X_RB_3D.png");
	textureManager_->LoadTexture("rule.png");
	for (size_t i = 0; i < 10; i++) { textureManager_->LoadTexture(std::to_string(i) + ".png"); }
	renderer_->SetTextureManager(textureManager_.get());

	// メッシュマネージャーの生成
	meshManager_ = std::make_unique<MeshManager>(device_);
	renderer_->SetMeshManager(meshManager_.get());

	// モデルの読み込み
	modelManager_ = std::make_unique<ModelManager>(textureManager_.get(), meshManager_.get(), logStream_);
	modelManager_->LoadModel("skydome.obj");
	modelManager_->LoadModel("ground.obj");
	modelManager_->LoadModel("wall.gltf");
	modelManager_->LoadModel("historic_european_brick_building_roebuck.gltf");
	modelManager_->LoadModel("angel_statue.gltf");
	modelManager_->LoadModel("title0.obj");
	modelManager_->LoadModel("title1.obj");
	modelManager_->LoadModel("title2.obj");
	modelManager_->LoadModel("body.obj");
	modelManager_->LoadModel("head.obj");
	modelManager_->LoadModel("leftArm.obj");
	modelManager_->LoadModel("rightArm.obj");
	modelManager_->LoadModel("enemy.obj");
	modelManager_->LoadModel("cube.obj");

	// パーティクルグループの作成
	particleManager_ = std::make_unique<ParticleManager>(device_, textureManager_.get(), meshManager_.get(), logStream_);
	particleManager_->CreateParticleGroup("leftArm", "speckle.png");
	particleManager_->CreateParticleGroup("rightArm", "speckle.png");
	particleManager_->CreateParticleGroup("attack", "circle.png");
	particleManager_->CreateParticleGroup("death", "gradation.png");

	// 現在のシーンの初期化
	currentScene_ = new SampleScene;
	currentScene_->Initialize(this);
}

void SceneManager::Update() {
	currentScene_->Update();

	if (nextScene_) {
		// 古いシーンを削除
		if (currentScene_) {
			delete currentScene_;
			currentScene_ = nullptr;
		}

		// 新しいシーンを設定
		currentScene_ = nextScene_;
		nextScene_ = nullptr;

		// 新しいシーンの初期化
		currentScene_->Initialize(this);
		currentScene_->Update();
	}
}