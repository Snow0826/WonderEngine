#pragma once
#include <string>
#include <vector>

/// @brief テキストデータ
struct Text final {
	std::vector<uint32_t> entities;	// エンティティリスト
	bool visible = true;			// 描画フラグ
};

class Registry;
class SpriteManager;
class ObjectManager;
struct Matrix4x4;
struct Vector3;
struct Quaternion;

/// @brief テキスト
class TextManager final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param spriteManager スプライトマネージャー
	TextManager(Registry *registry, SpriteManager *spriteManager, ObjectManager *objectManager)
		: registry_(registry), spriteManager_(spriteManager), objectManager_(objectManager) {}

	/// @brief テキストの作成
	/// @param text テキスト
	/// @param x X座標
	/// @param y Y座標
	Text CreateText(const std::string& text, float x, float y);

	/// @brief 4x4行列テキストの作成
	/// @param label ラベル
	/// @param matrix 行列
	/// @param x X座標
	/// @param y Y座標
	void CreateMatrix4x4Text(const std::string &label, const Matrix4x4 &matrix, float x, float y);

	/// @brief 3次元ベクトルテキストの作成
	/// @param label ラベル
	/// @param vector 3次元ベクトル
	/// @param x X座標
	/// @param y Y座標
	void CreateVector3Text(const std::string &label, const Vector3 &vector, float x, float y);

	/// @brief クォータニオンテキストの作成
	/// @param label ラベル
	/// @param quaternion クォータニオン
	/// @param x X座標
	/// @param y Y座標
	void CreateQuaternionText(const std::string &label, const Quaternion &quaternion, float x, float y);

	/// @brief テキストの設定
	/// @param textData テキストデータ
	/// @param text テキスト
	void SetText(const Text &textData, const std::string &text);

	/// @brief テキストの位置設定
	/// @param textData テキストデータ
	/// @param x X座標
	/// @param y Y座標
	void SetTextPosition(const Text &textData, float x, float y);

private:
	Registry *registry_ = nullptr;							// レジストリ
	SpriteManager *spriteManager_ = nullptr;				// スプライトマネージャー
	ObjectManager *objectManager_ = nullptr;				// オブジェクトマネージャー
	static inline constexpr int32_t kFontWidth = 9;			// フォント画像内1文字分の横幅
	static inline constexpr int32_t kFontHeight = 18;		// フォント画像内1文字分の縦幅
	static inline constexpr int32_t kFontLineCount = 14;	// フォント画像内1行分の文字数
	static inline constexpr int32_t kRowHeight = 20;		// 行間の高さ
	static inline constexpr int32_t kColumnWidth = 100;		// 列間の幅
};