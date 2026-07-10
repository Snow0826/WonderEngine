#pragma once
#include "Vector4.h"
#include "Transform.h"
#include "Collision.h"
#include "MeshType.h"
#include <unordered_map>
#include <memory>

/// @brief パーティクル
struct Particle final {
	Vector3 translate;	// 平行移動
	Vector3 scale;		// スケール
	float lifeTime;		// 寿命
	Vector3 velocity;	// 速度
	float currentTime;	// 経過時間
	Vector4 color;		// 色
};

/// @brief パーティクルグループ
struct ParticleGroup final {
	uint32_t meshHandle = 0;		// メッシュハンドル
	uint32_t textureHandle = 0;		// テクスチャハンドル
	uint32_t srvHandle = 0;			// SRVハンドル
	uint32_t uavHandle = 0;			// UAVハンドル
	std::string textureFileName;	// テクスチャファイル名
};

template<typename T>
struct Range final {
	T min;	// 最小値
	T max;	// 最大値	
};

/// @brief エミッター
struct Emitter final {
	EulerTransform transform;	// SRTデータ
	Collision::AABB area;		// 発生範囲
	Range<Vector3> scale;		// 大きさ
	Range<Vector3> rotate;		// 回転
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
class TextureManager;
class MeshManager;
class Resource;

/// @brief パーティクルマネージャー
class ParticleManager final {
public:
	static inline constexpr uint32_t kMaxParticle = 1024;	// 最大パーティクル数

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
	/// @param meshType メッシュタイプ
	/// @param textureFileName テクスチャファイル名
	void CreateParticleGroup(const std::string &name, MeshType meshType, const std::string &textureFileName);

	/// @brief パーティクルグループの検索
	/// @param name パーティクルグループ名
	/// @return パーティクルグループ
	ParticleGroup FindParticleGroup(const std::string &name);

private:
	Device *device_ = nullptr;														// デバイス
	TextureManager *textureManager_ = nullptr;										// テクスチャマネージャー
	MeshManager *meshManager_ = nullptr;											// メッシュマネージャー
	std::ofstream *logStream_ = nullptr;											// ログ出力用のストリーム
	std::unordered_map<std::string, std::unique_ptr<Resource>> particleResources_;	// パーティクルリソースマップ
	std::unordered_map<std::string, ParticleGroup> particleGroups_;					// パーティクルグループマップ
};

class Registry;

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