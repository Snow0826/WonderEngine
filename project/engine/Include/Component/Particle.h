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
	uint32_t textureHandle = 0;		// テクスチャハンドル
	uint32_t resourceHandle = 0;	// リソースハンドル
	uint32_t srvHandle = 0;			// SRVハンドル
	uint32_t uavHandle = 0;			// UAVハンドル
	std::string meshName;			// メッシュ名
	std::string textureFileName;	// テクスチャファイル名
};

/// @brief 球状エミッター
struct EmitterSphere final {
	Vector3 translate;		// 位置
	float radius;			// 射出半径
	uint32_t count;			// 射出数
	float frequency;		// 射出頻度
	float frequencyTime;	// 射出頻度調整用時間
	uint32_t emit;			// 射出許可
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
class Registry;
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

	/// @brief パーティクルリソースの取得
	/// @param handle パーティクルリソースハンドル
	/// @return パーティクルリソース
	Resource *GetParticleResource(uint32_t handle);

	/// @brief 球状エミッターの更新
	/// @param deltaTime デルタタイム
	void UpdateEmitterSphere(float deltaTime);

	/// @brief レジストリの設定
	/// @param registry レジストリ
	void SetRegistry(Registry *registry) { registry_ = registry; }

private:
	Device *device_ = nullptr;										// デバイス
	TextureManager *textureManager_ = nullptr;						// テクスチャマネージャー
	MeshManager *meshManager_ = nullptr;							// メッシュマネージャー
	Registry *registry_ = nullptr;									// レジストリ
	std::ofstream *logStream_ = nullptr;							// ログ出力用のストリーム
	std::vector<std::unique_ptr<Resource>> particleResources_;		// パーティクルリソースリスト
	std::unordered_map<std::string, ParticleGroup> particleGroups_;	// パーティクルグループマップ
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

/// @brief 球状エミッターインスペクター
class EmitterSphereInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	EmitterSphereInspector(Registry *registry) : registry_(registry) {}

	/// @brief 球状エミッターインスペクターの描画
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