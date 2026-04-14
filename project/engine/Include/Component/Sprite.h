#pragma once
#include "Vector2.h"
#include "Vector4.h"
#include <string>

/// @brief スプライトデータ
struct SpriteData final {
	Vector2 position = { 0.0f, 0.0f };			// 座標
	Vector2 size = { 512.0f, 512.0f };			// サイズ
	float rotation = 0.0f;						// 回転角
	Vector2 anchorPoint = { 0.0f, 0.0f };		// アンカーポイント
	bool isFlipX = false;						// 左右フリップ
	bool isFlipY = false;						// 上下フリップ
	Vector2 textureLeftTop = { 0.0f, 0.0f };	// テクスチャ左上座標
	Vector2 textureSize = { 512.0f, 512.0f };	// テクスチャサイズ
};

/// @brief スプライト
struct Sprite final {
	SpriteData initialSpriteData;	// 初期スプライトデータ
	SpriteData spriteData;			// スプライトデータ
	uint32_t meshHandle = 0;		// メッシュハンドル
	uint32_t textureHandle = 0;		// テクスチャハンドル
	bool enableMipMaps = true;		// ミップマップ有効フラグ
	std::string textureFileName;	// テクスチャファイル名
};

class TextureManager;
class MeshManager;
class Registry;

/// @brief スプライトマネージャー
class SpriteManager final {
public:
	/// @brief コンストラクタ
	/// @param textureManager テクスチャマネージャー
	/// @param meshManager メッシュマネージャー
	/// @param registry レジストリ
	SpriteManager(TextureManager *textureManager, MeshManager *meshManager, Registry *registry);

	/// @brief スプライトの生成
	/// @param textureFileName テクスチャファイル名
	/// @return スプライト
	Sprite CreateSprite(const std::string &textureFileName);

	/// @brief スプライトの更新
	void UpdateSprite();

private:
	TextureManager *textureManager_ = nullptr;	// テクスチャマネージャー
	MeshManager *meshManager_ = nullptr;		// メッシュマネージャー
	Registry *registry_ = nullptr;				// レジストリ
};

/// @brief スプライトインスペクター
class SpriteInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param textureManager テクスチャマネージャー
	SpriteInspector(Registry *registry, TextureManager *textureManager) : registry_(registry), textureManager_(textureManager) {}
	
	/// @brief スプライトインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;				// レジストリ
	TextureManager *textureManager_ = nullptr;	// テクスチャマネージャー
};