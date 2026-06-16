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
	renderer_->SetTextureManager(textureManager_.get());
	textureManager_->LoadTexture("kloofendal_48d_partly_cloudy_puresky_2k.dds");
	textureManager_->LoadTexture("Bark001_1K-JPG_Color.jpg");
	textureManager_->LoadTexture("white8x8.png");

	// メッシュマネージャーの生成
	meshManager_ = std::make_unique<MeshManager>(device_);
	renderer_->SetMeshManager(meshManager_.get());

	// モデルの読み込み
	modelManager_ = std::make_unique<ModelManager>(textureManager_.get(), meshManager_.get(), logStream_);
	modelManager_->LoadModel("AnimatedCube.gltf");
	modelManager_->LoadModel("simpleSkin.gltf");
	modelManager_->LoadModel("walk.gltf");
	modelManager_->LoadModel("sneakWalk.gltf");

	// パーティクルグループの作成
	particleManager_ = std::make_unique<ParticleManager>(device_, textureManager_.get(), meshManager_.get(), logStream_);
	particleManager_->CreateParticleGroup("hitEffect", MeshType::kPlane, "circle2.png");
	particleManager_->CreateParticleGroup("slashEffect", MeshType::kPlane, "circle2.png");
	particleManager_->CreateParticleGroup("hitRingEffect", MeshType::kRing, "gradationLine.png");
	particleManager_->CreateParticleGroup("slashRingEffect", MeshType::kRing, "gradationLine.png");

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