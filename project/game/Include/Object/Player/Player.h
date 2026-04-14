#pragma once
#include "SoundHandle.h"
#include "Vector3.h"
#include <string>
#include <vector>
#include <numbers>
#include <memory>

class Registry;
class IndirectCommandManager;
class ModelManager;
class ObjectManager;
class FootprintManager;
class ParticleManager;
class Input;
class Audio;
class ArmParticle;
class AttackParticle;
namespace Collision {
	struct Plane;
	struct AABB;
}
class BasePlayerBehavior;

/// @brief プレイヤー
class Player {
public:
	/// @brief 部位の種類
	enum class PartsType {
		kBody,		// 体
		kHead,		// 頭
		kLeftArm,	// 左腕
		kRightArm,	// 右腕
	};

	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param indirectCommandManager 間接コマンドマネージャー
	/// @param modelManager モデルマネージャー
	/// @param objectManager オブジェクトマネージャー
	/// @param footprintManager フットプリントマネージャー
	/// @param particleManager パーティクルマネージャー
	/// @param input 入力
	Player(Registry *registry, IndirectCommandManager *indirectCommandManager, ModelManager *modelManager, ObjectManager *objectManager, FootprintManager *footprintManager, ParticleManager *particleManager, Input *input, Audio *audio);

	/// @brief デストラクタ
	~Player();

	/// @brief 初期化
	/// @param position 初期位置
	void Initialize(const Vector3 &position);

	/// @brief 更新
	void Update();

	/// @brief ワールド変換後処理
	void AfterTransform();

	/// @brief 衝突応答
	/// @param plane 衝突した平面
	void OnCollision(const Collision::Plane &plane);

	/// @brief 衝突応答
	/// @param aabb 衝突したAABB
	void OnCollision(const Collision::AABB &aabb);

	/// @brief 衝突応答
	void OnCollision();

	/// @brief 死亡
	void Dead();

	/// @brief 復活
	void Revival();

	/// @brief 移動
	void Move();

	/// @brief 回転
	void Rotate();

	/// @brief 浮遊
	void Float();

	/// @brief 振る舞いの設定
	/// @param nextBehavior 次の振る舞い
	void SetNextBehavior(std::unique_ptr<BasePlayerBehavior> nextBehavior);

	/// @brief カメラエンティティの設定
	/// @param cameraEntity カメラエンティティ
	void SetCameraEntity(uint32_t cameraEntity) { cameraEntity_ = cameraEntity; }

	/// @brief 音声ハンドルの設定
	/// @param soundHandle 音声ハンドル
	void SetSoundHandle(const SoundHandle &soundHandle) { soundHandle_ = soundHandle; }

	/// @brief 音声ハンドルの取得
	/// @return 音声ハンドル
	SoundHandle GetSoundHandle() const { return soundHandle_; }

	/// @brief 部位エンティティの取得
	/// @param partsType 部位の種類
	/// @return 部位エンティティ
	uint32_t GetPartsEntity(PartsType partsType) const { return entities_[static_cast<size_t>(partsType)]; }

	/// @brief 攻撃パーティクルエンティティの取得
	/// @return 攻撃パーティクルエンティティ
	uint32_t GetAttackParticleEntity() const;

	/// @brief レジストリの取得
	/// @return レジストリ
	Registry *GetRegistry() const { return registry_; }

	/// @brief 入力の取得
	/// @return 入力
	Input *GetInput() const { return input_; }

	/// @brief オーディオの取得
	/// @return オーディオ
	Audio *GetAudio() const { return audio_; }

	/// @brief 攻撃パーティクルの取得
	/// @return 攻撃パーティクル
	AttackParticle *GetAttackParticle() const { return attackParticle_.get(); }

	/// @brief ジャンプカウンターの取得
	/// @return ジャンプカウンター
	uint32_t *GetJumpCounter() { return &jumpCounter_; }

	/// @brief 基準のY座標の取得
	/// @return 基準のY座標
	float GetBasePositionY() const { return basePositionY_; }

	/// @brief 死亡中かどうか判定
	/// @return 死亡中かどうか
	bool IsDead() const;

private:
	Registry *registry_ = nullptr;									// レジストリ
	IndirectCommandManager *indirectCommandManager_ = nullptr;		// 間接コマンドマネージャー
	ModelManager *modelManager_ = nullptr;							// モデルマネージャー
	ObjectManager *objectManager_ = nullptr;						// オブジェクトマネージャー
	FootprintManager *footprintManager_ = nullptr;					// フットプリントマネージャー
	ParticleManager *particleManager_ = nullptr;					// パーティクルマネージャー
	Input *input_ = nullptr;										// 入力
	Audio *audio_ = nullptr;										// オーディオ
	SoundHandle soundHandle_;										// 音声ハンドル
	uint32_t cameraEntity_ = 0;										// カメラエンティティ
	std::vector<uint32_t> entities_;								// プレイヤーのエンティティ群
	uint32_t footprintEntity_ = 0;									// フットプリントエンティティ
	uint32_t jumpCounter_ = 0;										// ジャンプカウンター
	std::unique_ptr<ArmParticle> leftArmParticle_ = nullptr;		// 左腕パーティクル
	std::unique_ptr<ArmParticle> rightArmParticle_ = nullptr;		// 右腕パーティクル
	std::unique_ptr<AttackParticle> attackParticle_ = nullptr;		// 攻撃パーティクル
	bool isPressMove_ = false;										// 移動入力があるかどうか
	float floatingParameter_ = 0.0f;								// 浮遊パラメーター
	float basePositionY_ = 0.0f;									// 基準のY座標
	Vector3 bodyRotate_;											// 体の回転角度
	std::unique_ptr<BasePlayerBehavior> currentBehavior_ = nullptr;	// 現在の振る舞い
	std::unique_ptr<BasePlayerBehavior> nextBehavior_ = nullptr;	// 次の振る舞い

	static inline constexpr float kBodyAmplitude = 0.3f;	// 体の浮遊の振幅
	static inline constexpr float kArmAmplitude = 0.2f;		// 腕の浮遊の振幅
	static inline constexpr uint32_t kCycle = 120;			// 浮遊の周期
	static inline constexpr float kStep = std::numbers::pi_v<float> *2.0f / kCycle;	// 浮遊のステップ
	static inline constexpr float kHeight = 1.5f;	// 浮遊の高さ

	/// @brief 部位の追加
	/// @param partsName 部位名
	void AddParts(const std::string &partsName);

	/// @brief 部位の削除
	/// @param partsType 部位の種類
	void RemoveParts(PartsType partsType);

	/// @brief 部位の復活
	/// @param partsType 部位の種類
	void ReviveParts(PartsType partsType);
};