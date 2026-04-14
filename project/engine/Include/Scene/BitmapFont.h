#pragma once
#include <vector>

class Registry;
class SpriteManager;
class ObjectManager;

/// @brief ビットマップフォント
class BitmapFont final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param spriteManager スプライトマネージャー
	/// @param objectManager オブジェクトマネージャー
	BitmapFont(Registry *registry, SpriteManager *spriteManager, ObjectManager *objectManager)
		: registry_(registry), spriteManager_(spriteManager), objectManager_(objectManager) {}

	/// @brief 初期化
	void Initialize();

	/// @brief スプライトに変換
	/// @param value 変換する値
	void ToSprite(uint32_t value);

	/// @brief ビットマップフォントのクリア
	void Clear();

	/// @brief テクスチャハンドルの追加
	/// @param textureHandle テクスチャハンドル
	void AddTextureHandle(uint32_t textureHandle) {
		textureHandles_.emplace_back(textureHandle);
	}

private:
	Registry *registry_ = nullptr;				// レジストリ
	SpriteManager *spriteManager_ = nullptr;	// スプライトマネージャー
	ObjectManager *objectManager_ = nullptr;	// オブジェクトマネージャー
	std::vector<uint32_t> digitEntities_;		// 桁のエンティティリスト
	std::vector<uint32_t> textureHandles_;		// テクスチャハンドルリスト
	static inline constexpr uint32_t kDigitCount = 2;	// 桁数
};