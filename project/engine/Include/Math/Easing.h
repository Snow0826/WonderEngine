#pragma once
#include <Interpolation.h>
#include <cmath>

/// @brief イージング
/// @tparam T 補間する型
template<typename T = float>
struct Easing final {
	/// @brief イージングイン (サークル関数)
	/// @param timer 時間
	/// @return イージング時間
	static float InCirc(float timer) {
		return 1.0f - std::sqrt(1.0f - std::pow(timer, 2.0f));
	}

	/// @brief イージングイン (サークル関数)
	/// @param timer 時間
	/// @return イージング時間
	static float OutCirc(float timer) {
		return std::sqrt(1.0f - std::pow(timer - 1.0f, 2.0f));
	}

	/// @brief イージングインアウト (サークル関数)
	/// @param timer 時間
	/// @return イージング時間
	static float InOutCirc(float timer) {
		if (timer < 0.5) {
			return (1.0f - std::sqrt(1.0f - std::pow(2.0f * timer, 2.0f))) / 2.0f;
		} else {
			return (std::sqrt(1.0f - std::pow(-2.0f * timer + 2.0f, 2.0f)) + 1.0f) / 2.0f;
		}
	}

	/// @brief イージングイン (サークル関数)
	/// @param start 始点
	/// @param end 終点
	/// @param timer 時間
	/// @param duration 最大時間
	/// @return 現在の点
	static T InCirc(const T &start, const T &end, const uint32_t &timer, const uint32_t &duration) {
		return Interpolation<T>::Lerp(start, end, InCirc(static_cast<float>(timer) / static_cast<float>(duration)));
	}

	/// @brief イージングアウト (サークル関数)
	/// @param start 始点
	/// @param end 終点
	/// @param timer 時間
	/// @param duration 最大時間
	/// @return 現在の点
	static T OutCirc(const T &start, const T &end, const uint32_t &timer, const uint32_t &duration) {
		return Interpolation<T>::Lerp(start, end, OutCirc(static_cast<float>(timer) / static_cast<float>(duration)));
	}

	/// @brief イージングインアウト (サークル関数)
	/// @param start 始点
	/// @param end 終点
	/// @param timer 時間
	/// @param duration 最大時間
	/// @return 現在の点
	static T InOutCirc(const T &start, const T &end, const uint32_t &timer, const uint32_t &duration) {
		return Interpolation<T>::Lerp(start, end, InOutCirc(static_cast<float>(timer) / static_cast<float>(duration)));
	}

	/// @brief イージングイン (サークル関数)
	/// @param start 始点
	/// @param mid 中間点
	/// @param end 終点
	/// @param timer 時間
	/// @param duration 最大時間
	/// @return 現在の点
	static T InCirc(const T &start, const T &mid, const T &end, const uint32_t &timer, const uint32_t &duration) {
		return Interpolation<T>::Bezier(start, mid, end, InCirc(static_cast<float>(timer) / static_cast<float>(duration)));
	}

	/// @brief イージンアウト (サークル関数)
	/// @param start 始点
	/// @param mid 中間点
	/// @param end 終点
	/// @param timer 時間
	/// @param duration 最大時間
	/// @return 現在の点
	static T OutCirc(const T &start, const T &mid, const T &end, const uint32_t &timer, const uint32_t &duration) {
		return Interpolation<T>::Bezier(start, mid, end, OutCirc(static_cast<float>(timer) / static_cast<float>(duration)));
	}

	/// @brief イージングインアウト (サークル関数)
	/// @param start 始点
	/// @param mid 中間点
	/// @param end 終点
	/// @param timer 時間
	/// @param duration 最大時間
	/// @return 現在の点
	static T InOutCirc(const T &start, const T &mid, const T &end, const uint32_t &timer, const uint32_t &duration) {
		return Interpolation<T>::Bezier(start, mid, end, InOutCirc(static_cast<float>(timer) / static_cast<float>(duration)));
	}
};