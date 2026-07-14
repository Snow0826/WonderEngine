#pragma once
#include <cstdint>

/// @brief ブレンドモード
enum class BlendMode {
	kBlendModeNone,				// ブレンドなし
	kBlendModeNormal,			// 通常αブレンド
	kBlendModeAdditive,			// 加算
	kBlendModeSubtractive,		// 減算
	kBlendModeMultiplicative,	// 乗算
	kBlendModeScreen,			// スクリーン
	kCountOfBlendMode			// ブレンドモードの数
};

class Registry;

/// @brief ブレンドモードインスペクター
class BlendModeInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	BlendModeInspector(Registry *registry) : registry_(registry) {}

	/// @brief ブレンドモードインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;	// レジストリ
};