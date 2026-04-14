#include "MathUtility.h"
#include <cmath>

float Mod(float x, float y) {
	return x - y * std::floor(x / y);
}

float Wrap(float x, float min, float max) {
	const float range = max - min;
	return std::fmodf(std::fmodf(x - min, range) + range, range) + min;
}

bool NearlyEqual(float a, float b, float epsilon) {
	return std::abs(a - b) < epsilon;
}

int32_t Quantize(float value, float scale) {
	return static_cast<int32_t>(std::round(value * scale));
}
