#pragma once
#include "SoundHandle.h"
#include <fstream>

class Device;
class Input;
class Audio;
class Renderer;
class World;
class TextureManager;
class MeshManager;
class ModelManager;
class ParticleManager;
class BaseScene;

/// @brief シーンマネージャー
class SceneManager final {
public:
	/// @brief コンストラクタ
	SceneManager();

	/// @brief デストラクタ
	~SceneManager();

	/// @brief 初期化
	/// @param device デバイス
	/// @param input 入力
	/// @param audio オーディオ
	/// @param renderer レンダラー
	/// @param logStream ログ出力ストリーム
	void Initialize(Device *device, Input *input, Audio *audio, Renderer *renderer, std::ofstream *logStream);

	/// @brief 更新
	void Update();

	/// @brief 次のシーンの設定
	/// @param nextScene 次のシーン
	void SetNextScene(BaseScene *nextScene) { nextScene_ = nextScene; }

	/// @brief デバイスの取得
	/// @return デバイス
	Device *GetDevice() const { return device_; }

	/// @brief 入力の取得
	/// @return 入力
	Input *GetInput() const { return input_; }

	/// @brief オーディオの取得
	/// @return オーディオ
	Audio *GetAudio() const { return audio_; }

	/// @brief 音声ハンドルの取得
	/// @return 音声ハンドル
	SoundHandle GetSoundHandle() const { return soundHandle_; }

	/// @brief レンダラーの取得
	/// @return レンダラー
	Renderer *GetRenderer() const { return renderer_; }

	/// @brief ワールドの取得
	/// @return ワールド
	World *GetWorld() const { return world_.get(); }

	/// @brief テクスチャマネージャーの取得
	/// @return テクスチャマネージャー
	TextureManager *GetTextureManager() const { return textureManager_.get(); }

	/// @brief メッシュマネージャーの取得
	/// @return メッシュマネージャー
	MeshManager *GetMeshManager() const { return meshManager_.get(); }

	/// @brief モデルマネージャーの取得
	/// @return モデルマネージャー
	ModelManager *GetModelManager() const { return modelManager_.get(); }

	/// @brief パーティクルマネージャーの取得
	/// @return パーティクルマネージャー
	ParticleManager *GetParticleManager() const { return particleManager_.get(); }

private:
	Device *device_ = nullptr;										// デバイス
	Input *input_ = nullptr;										// 入力
	Audio *audio_ = nullptr;										// オーディオ
	Renderer *renderer_ = nullptr;									// レンダラー
	std::ofstream *logStream_ = nullptr;							// ログ出力用のストリーム
	std::unique_ptr<World> world_ = nullptr;						// ワールド
	std::unique_ptr<TextureManager> textureManager_ = nullptr;		// テクスチャマネージャー
	std::unique_ptr<MeshManager> meshManager_ = nullptr;			// メッシュマネージャー
	std::unique_ptr<ModelManager> modelManager_ = nullptr;			// モデルマネージャー
	std::unique_ptr<ParticleManager> particleManager_ = nullptr;	// パーティクルマネージャー
	SoundHandle soundHandle_;										// 音声ハンドル
	BaseScene *currentScene_ = nullptr;								// 現在のシーン
	BaseScene *nextScene_ = nullptr;								// 次のシーン
};