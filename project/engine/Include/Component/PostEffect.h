#pragma once

/// @brief ポストエフェクト
enum class PostEffect {
	kNone,					// なし
	kGrayscale,				// グレースケール
	kVignette,				// ビネット
	kBoxFilter,				// ボックスフィルター
	kGaussianFilter,		// ガウシアンフィルター
	kLuminanceBasedOutline,	// 輝度ベースの輪郭抽出
	kDepthBasedOutline,		// 深度ベースの輪郭抽出
	kRadialBlur,			// ラジアルブラー
	kDissolve,				// ディゾルブ
	kNoise,					// ノイズ
	kCountOfPostEffect		// ポストエフェクトの数
};