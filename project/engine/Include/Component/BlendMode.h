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
class IndirectCommandManager;

/// @brief ブレンドモードインスペクター
class BlendModeInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param indirectCommandManager 間接コマンドマネージャー
	BlendModeInspector(Registry *registry, IndirectCommandManager *indirectCommandManager)
		: registry_(registry), indirectCommandManager_(indirectCommandManager) {}

	/// @brief ブレンドモードインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;								// レジストリ
	IndirectCommandManager *indirectCommandManager_ = nullptr;	// 間接コマンドマネージャー
};