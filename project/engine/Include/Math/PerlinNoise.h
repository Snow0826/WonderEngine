#pragma once
#include <vector>

/// @brief パーリンノイズ
class PerlinNoise final {
public:
	/// @brief コンストラクタ
	PerlinNoise();

	/// @brief ノイズ値の取得
	/// @param x X座標
	/// @param y Y座標
	/// @param z Z座標
	/// @return ノイズ値
	float Noise(float x, float y, float z) const;

	/// @brief フラクタルノイズ値の取得
	/// @param x X座標
	/// @param y Y座標
	/// @param z Z座標
	/// @param octaves オクターブ数
	/// @param lacunarity 周波数の倍率
	/// @param gain 振幅の倍率
	/// @return フラクタルノイズ値
	float FractalNoise(float x, float y, float z, uint32_t octaves, float lacunarity = 2.0f, float gain = 0.5f) const;

private:
	std::vector<uint32_t> permutation_;	// パーミュテーションテーブル

	/// @brief フェード関数
	/// @param t 入力値
	/// @return 出力値
	float fade(float t) const;

	/// @brief 勾配関数
	/// @param hash ハッシュ値
	/// @param x X座標
	/// @param y Y座標
	/// @param z Z座標
	/// @return 出力値
	float grad(int32_t hash, float x, float y, float z) const;
};