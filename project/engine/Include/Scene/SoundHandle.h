#pragma once
#include <unordered_map>

/// @brief BGMの種類
enum class BGMType {
	Title,	// タイトル
	Game,	// ゲーム
};

/// @brief SEの種類
enum class SEType {
	Start,		// スタート
	Decide,		// 決定
	Jump,		// ジャンプ
	OnGround,	// 着地
	Attack,		// 攻撃
	Dash,		// ダッシュ
};

/// @brief 音声ハンドル
struct SoundHandle {
	std::unordered_map<BGMType, uint32_t> bgmHandleMap_;	// BGMハンドルマップ
	std::unordered_map<SEType, uint32_t> seHandleMap_;		// SEハンドルマップ
};