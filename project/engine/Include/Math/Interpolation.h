#pragma once
#include <cstdint>

/// @brief 補間
/// @tparam T 補間する型
template<typename T = float>
struct Interpolation final {
	/// @brief 線形補間
	/// @param start 始点
	/// @param end 終点
	/// @param timer 時間
	/// @return 現在の点
	static constexpr T Lerp(const T &start, const T &end, float timer) {
		return (1.0f - timer) * start + timer * end;
	}

	/// @brief 線形補間
	/// @param start 始点
	/// @param end 終点
	/// @param timer 時間
	/// @param duration 最大時間
	/// @return 現在の点
	static constexpr T Lerp(const T &start, const T &end, const uint32_t &timer, const uint32_t &duration) {
		return Lerp(start, end, static_cast<float>(timer) / static_cast<float>(duration));
	}

	/// @brief ベジェ曲線
	/// @param start 始点
	/// @param mid 中間点
	/// @param end 終点
	/// @param timer 時間
	/// @return 現在の点
	static constexpr T Bezier(const T &start, const T &mid, const T &end, float timer) {
		return Lerp(Lerp(start, mid, timer), Lerp(mid, end, timer), timer);
	}

	/// @brief ベジェ曲線
	/// @param start 始点
	/// @param mid 中間点
	/// @param end 終点
	/// @param timer 時間
	/// @param duration 最大時間
	/// @return 現在の点
	static constexpr T Bezier(const T &start, const T &mid, const T &end, const uint32_t &timer, const uint32_t &duration) {
		return Bezier(start, mid, end, static_cast<float>(timer) / static_cast<float>(duration));
	}

	/// @brief エルミート曲線
	/// @param startPosition 始点
	/// @param endPosition 終点
	/// @param startTangent 始点の接線
	/// @param endTangent 終点の接線
	/// @param timer 時間
	/// @return 現在の点
	static constexpr T Hermite(const T &startPosition, const T &endPosition, const T &startTangent, const T &endTangent, float timer) {
		float t2 = timer * timer;
		float t3 = t2 * timer;
		float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
		float h10 = t3 - 2.0f * t2 + timer;
		float h01 = -2.0f * t3 + 3.0f * t2;
		float h11 = t3 - t2;
		return h00 * startPosition + h10 * startTangent + h01 * endPosition + h11 * endTangent;
	}

	/// @brief エルミート曲線
	/// @param startPosition 始点
	/// @param endPosition 終点
	/// @param startTangent 始点の接線
	/// @param endTangent 終点の接線
	/// @param timer 時間
	/// @param duration 最大時間
	/// @return 現在の点
	static constexpr T Hermite(const T &startPosition, const T &endPosition, const T &startTangent, const T &endTangent, const uint32_t &timer, const uint32_t &duration) {
		return Hermite(startPosition, endPosition, startTangent, endTangent, static_cast<float>(timer) / static_cast<float>(duration));
	}

	/// @brief エルミート曲線の導関数
	/// @param startPosition 始点
	/// @param endPosition 終点
	/// @param startTangent 始点の接線
	/// @param endTangent 終点の接線
	/// @param timer 時間
	/// @return 現在の点の導関数
	static constexpr T HermiteDerivative(const T &startPosition, const T &endPosition, const T &startTangent, const T &endTangent, float timer) {
		float t2 = timer * timer;
		float h00 = 6.0f * t2 - 6.0f * timer;
		float h10 = 3.0f * t2 - 4.0f * timer + 1.0f;
		float h01 = -6.0f * t2 + 6.0f * timer;
		float h11 = 3.0f * t2 - 2.0f * timer;
		return h00 * startPosition + h10 * startTangent + h01 * endPosition + h11 * endTangent;
	}

	/// @brief エルミート曲線の導関数
	/// @param startPosition 始点
	/// @param endPosition 終点
	/// @param startTangent 始点の接線
	/// @param endTangent 終点の接線
	/// @param timer 時間
	/// @param duration 最大時間
	/// @return 現在の点の導関数
	static constexpr T HermiteDerivative(const T &startPosition, const T &endPosition, const T &startTangent, const T &endTangent, const uint32_t &timer, const uint32_t &duration) {
		return HermiteDerivative(startPosition, endPosition, startTangent, endTangent, static_cast<float>(timer) / static_cast<float>(duration));
	}
};