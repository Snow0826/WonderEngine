#pragma once
#include "Vector4.h"
#include <string>

class Registry;
class SpriteManager;

/// @brief フェード
class Fade final {
public:
	/// @brief フェードの状態
	enum class Status {
		None,   // フェードなし
		FadeIn, // フェードイン
		FadeOut // フェードアウト
	};

	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param spriteManager スプライトマネージャー
	Fade(Registry *registry, SpriteManager *spriteManager) : registry_(registry), spriteManager_(spriteManager) {}

	/// @brief フェードの追加
	/// @param textureFileName テクスチャファイル名
	/// @param color フェードカラー
	void Add(const std::string &textureFileName = "white8x8.png", const Vector4 &color = {});

	/// @brief フェードの削除
	void Remove();

	/// @brief 更新
	/// @param deltaTime デルタタイム
	void Update(float deltaTime);

	/// @brief フェードの開始
	/// @param status フェードの状態
	/// @param duration フェードの継続時間
	void Start(Status status, float duration);

	/// @brief フェードの停止
	void Stop();

	/// @brief フェードの終了判定
	/// @return フェードの終了フラグ
	bool IsFinished() const;

private:
	Registry *registry_ = nullptr;				// レジストリ
	SpriteManager *spriteManager_ = nullptr;	// スプライトマネージャー
	uint32_t entity_ = 0;						// エンティティ
	Status status_ = Status::None;				// 現在のフェードの状態
	float duration_ = 0.0f;						// フェードの継続時間
	float counter_ = 0.0f;						// フェードの経過時間
};