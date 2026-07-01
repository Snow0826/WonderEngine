#pragma once
#include "Vector2.h"

class Registry;
class SpriteManager;
class ObjectManager;

/// @brief ボタン
class Button {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param spriteManager スプライトマネージャー
	/// @param objectManager オブジェクトマネージャー
	Button(Registry *registry, SpriteManager *spriteManager, ObjectManager *objectManager)
		: registry_(registry), spriteManager_(spriteManager), objectManager_(objectManager) {}

	/// @brief 初期化
	void Initialize();

	/// @brief 更新
	void Update();

	/// @brief アニメーションの開始
	void StartAnimation() { isAnimating_ = true; }

private:
	Registry *registry_ = nullptr;				// レジストリ
	SpriteManager *spriteManager_ = nullptr;	// スプライトマネージャー
	ObjectManager *objectManager_ = nullptr;	// オブジェクトマネージャー
	uint32_t entity_ = 0;						// エンティティ
	uint32_t animationTimer_ = 0;				// アニメーションタイマー
	bool isAnimating_ = false;					// アニメーション中かどうか
	static inline constexpr Vector2 kMaxButtonSize = { 128.0f, 128.0f };	// ボタンの最大サイズ
	static inline constexpr Vector2 kMinButtonSize = { 64.0f, 64.0f };		// ボタンの最小サイズ
	static inline constexpr uint32_t kAnimationDuration = 30;				// アニメーションの継続時間
};