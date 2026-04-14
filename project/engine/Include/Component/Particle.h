#pragma once
#include "Vector4.h"
#include "Transform.h"
#include "Collision.h"
#include <list>
#include <unordered_map>
#include <memory>

/// @brief パーティクル(GPU)
struct ParticleForGPU final {
	Matrix4x4 worldMatrix;	// ワールド行列
	Vector4 color;			// 色
};

/// @brief パーティクル
struct Particle final {
	Transform transform;		// SRTデータ
	Vector3 velocity;			// 速度
	Vector4 color;				// 色
	float lifeTime = 0.0f;		// 寿命
	float age = 0.0f;			// 経過時間
	float angle = 0.0f;			// 角度
	bool collidedField = false;	// 場と衝突したかどうか
};

/// @brief パーティクルグループ
struct ParticleGroup final {
	std::list<Particle> particles;	// パーティクルリスト
	bool isBillboard = true;		// ビルボード有効フラグ
	uint32_t meshHandle = 0;		// メッシュハンドル
	uint32_t textureHandle = 0;		// テクスチャハンドル
	uint32_t instanceHandle = 0;	// インスタンスハンドル
	uint32_t numInstance = 0;		// インスタンス数
	std::string textureFileName;	// テクスチャファイル名
	std::string particleDataName;	// パーティクルデータ名
};

template<typename T>
struct Range final {
	T min;	// 最小値
	T max;	// 最大値	
};

/// @brief エミッター
struct Emitter final {
	Transform transform;		// SRTデータ
	Collision::AABB area;		// 発生範囲
	Range<Vector3> scale;		// 大きさ
	Range<Vector3> velocity;	// 速度
	Range<Vector4> color;		// 色
	Range<float> lifeTime;		// 寿命
	uint32_t count;				// 発生数
	float frequency;			// 発生頻度
	float frequencyTime;		// 発生頻度用時刻
};

/// @brief 場
struct Field final {
	Collision::AABB area;			// 範囲
	Vector3 acceleration;			// 加速度
	float angularVelocity = 0.0f;	// 角速度
	float radius = 0.0f;			// 半径
};

class Device;
class Resource;
class TextureManager;
class MeshManager;
class Registry;

/// @brief パーティクルマネージャー
class ParticleManager final {
public:
	/// @brief コンストラクタ
	/// @param device デバイス
	/// @param textureManager テクスチャマネージャー
	/// @param meshManager メッシュマネージャー
	/// @param logStream ログストリーム
	ParticleManager(Device *device, TextureManager *textureManager, MeshManager *meshManager, std::ofstream *logStream);

	/// @brief デストラクタ
	~ParticleManager();

	/// @brief パーティクルグループの生成
	/// @param name パーティクルグループ名
	/// @param textureFileName テクスチャファイル名
	void CreateParticleGroup(const std::string &name, const std::string &textureFileName);

	/// @brief パーティクルグループの検索
	/// @param name パーティクルグループ名
	/// @return パーティクルグループ
	ParticleGroup FindParticleGroup(const std::string &name);

	/// @brief パーティクルの発生
	/// @param entity エンティティ
	void Emit(uint32_t entity);

	/// @brief パーティクルの更新
	void UpdateParticle();

	/// @brief レジストリの設定
	/// @param registry レジストリ
	void SetRegistry(Registry *registry) { registry_ = registry; }

private:
	static inline constexpr uint32_t kMaxParticle = 4096;							// 最大パーティクル数
	Device *device_ = nullptr;														// デバイス
	TextureManager *textureManager_ = nullptr;										// テクスチャマネージャー
	MeshManager *meshManager_ = nullptr;											// メッシュマネージャー
	Registry *registry_ = nullptr;													// レジストリ
	std::ofstream *logStream_ = nullptr;											// ログ出力用のストリーム
	std::unordered_map<std::string, ParticleGroup> particleGroups_;					// パーティクルグループマップ
	std::unordered_map<std::string, std::unique_ptr<Resource>> particleResources_;	// パーティクルリソースマップ
	std::unordered_map<std::string, ParticleForGPU *> particleDataList_;			// パーティクルデータマップ
	static inline constexpr float kDeltaTime = 1.0f / 60.0f;						// デルタタイム

	/// @brief パーティクルの生成
	/// @param entity エンティティ
	/// @return パーティクル
	Particle CreateParticle(uint32_t entity);
};

/// @brief パーティクルグループインスペクター
class ParticleGroupInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param textureManager テクスチャマネージャー
	ParticleGroupInspector(Registry *registry, TextureManager *textureManager) : registry_(registry), textureManager_(textureManager) {}

	/// @brief パーティクルグループインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;				// レジストリ
	TextureManager *textureManager_ = nullptr;	// テクスチャマネージャー
};

/// @brief エミッターインスペクター
class EmitterInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	EmitterInspector(Registry *registry) : registry_(registry) {}

	/// @brief エミッターインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;	// レジストリ
};

/// @brief フィールドインスペクター
class FieldInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	FieldInspector(Registry *registry) : registry_(registry) {}
	
	/// @brief フィールドインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;	// レジストリ
};