#include "PerlinNoise.h"
#include "Easing.h"
#include <random>

PerlinNoise::PerlinNoise() {
	uint32_t permutation[] = {
		151, 160, 137,  91,  90,  15, 131,  13, 201,  95,  96,  53, 194, 233,   7, 225,
		140,  36, 103,  30,  69, 142,   8,  99,  37, 240,  21,  10,  23, 190,   6, 148,
		247, 120, 234,  75,   0,  26, 197,  62,  94, 252, 219, 203, 117,  35,  11,  32,
		 57, 177,  33,  88, 237, 149,  56,  87, 174,  20, 125, 136, 171, 168,  68, 175,
		 74, 165,  71, 134, 139,  48,  27, 166,  77, 146, 158, 231,  83, 111, 229, 122,
		 60, 211, 133, 230, 220, 105,  92,  41,  55,  46, 245,  40, 244, 102, 143,  54,
		 65,  25,  63, 161,   1, 216,  80,  73, 209,  76, 132, 187, 208,  89,  18, 169,
		200, 196, 135, 130, 116, 188, 159,  86, 164, 100, 109, 198, 173, 186,   3,  64,
		 52, 217, 226, 250, 124, 123,   5, 202,  38, 147, 118, 126, 255,  82,  85, 212,
		207, 206,  59, 227,  47,  16,  58,  17, 182, 189,  28,  42, 223, 183, 170, 213,
		119, 248, 152,   2,  44, 154, 163,  70, 221, 153, 101, 155, 167,  43, 172,   9,
		129,  22,  39, 253,  19,  98, 108, 110,  79, 113, 224, 232, 178, 185, 112, 104,
		218, 246,  97, 228, 251,  34, 242, 193, 238, 210, 144,  12, 191, 179, 162, 241,
		 81,  51, 145, 235, 249,  14, 239, 107,  49, 192, 214,  31, 181, 199, 106, 157,
		184,  84, 204, 176, 115, 121,  50,  45, 127,   4, 150, 254, 138, 236, 205,  93,
		222, 114,  67,  29,  24,  72, 243, 141, 128, 195,  78,  66, 215,  61, 156, 180
	};

	permutation_.resize(256);
	for (int i = 0; i < 256; ++i) permutation_[i] = permutation[i];

	// 拡張して256 * 2にする
	permutation_.insert(permutation_.end(), permutation_.begin(), permutation_.end());
}

float PerlinNoise::Noise(float x, float y, float z) const {
	// Find unit cube that contains the point
	uint32_t X = static_cast<uint32_t>(std::floor(x)) & 255;
	uint32_t Y = static_cast<uint32_t>(std::floor(y)) & 255;
	uint32_t Z = static_cast<uint32_t>(std::floor(z)) & 255;
	// Relative x, y, z in cube
	x -= std::floor(x);
	y -= std::floor(y);
	z -= std::floor(z);
	// Compute fade curves for x, y, z
	float u = fade(x);
	float v = fade(y);
	float w = fade(z);
	// Hash coordinates of the 8 cube corners
	uint32_t A = permutation_[X] + Y;
	uint32_t AA = permutation_[A] + Z;
	uint32_t AB = permutation_[A + 1] + Z;
	uint32_t B = permutation_[X + 1] + Y;
	uint32_t BA = permutation_[B] + Z;
	uint32_t BB = permutation_[B + 1] + Z;
	// Blend the results from the corners
	return Easing<float>::Lerp(
		Easing<float>::Lerp(
			Easing<float>::Lerp(
				grad(permutation_[AA	], x	, y		, z		),
				grad(permutation_[BA	], x - 1, y		, z		), u),
			Easing<float>::Lerp(
				grad(permutation_[AB	], x	, y - 1	, z		),
				grad(permutation_[BB	], x - 1, y - 1	, z		), u), v),
		Easing<float>::Lerp(
			Easing<float>::Lerp(
				grad(permutation_[AA + 1], x	, y		, z - 1	),
				grad(permutation_[BA + 1], x - 1, y		, z - 1	), u),
			Easing<float>::Lerp(
				grad(permutation_[AB + 1], x	, y - 1	, z - 1	),
				grad(permutation_[BB + 1], x - 1, y - 1	, z - 1	), u), v), w);
}

float PerlinNoise::FractalNoise(float x, float y, float z, uint32_t octaves, float lacunarity, float gain) const {
	float total = 0.0f;
	float frequency = 1.0f;
	float amplitude = 1.0f;
	float maxAmplitude = 0.0f; // 正規化のため

	for (uint32_t i = 0; i < octaves; ++i) {
		total += Noise(x * frequency, y * frequency, z * frequency) * amplitude;
		maxAmplitude += amplitude;
		frequency *= lacunarity; // 周波数倍加
		amplitude *= gain;       // 振幅減少
	}

	return total / maxAmplitude; // -1.0〜+1.0 に正規化
}

float PerlinNoise::fade(float t) const {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

float PerlinNoise::grad(int32_t hash, float x, float y, float z) const {
	uint32_t h = hash & 15; // Convert hash to 0-15
	float u = h < 8 ? x : y; // Gradient direction
	float v = h < 4 ? y : (h == 12 || h == 14 ? x : z); // Second gradient direction
	return ((h & 1) ? -u : u) + ((h & 2) ? -v : v); // Return the final gradient value
}