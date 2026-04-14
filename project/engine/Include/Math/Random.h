#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <random>

template <typename T>
constexpr bool false_v = false;

/// @brief 疑似乱数生成
struct Random final {
	/// @brief 疑似乱数生成器
	/// @tparam T 疑似乱数の型
	/// @param min 最小値
	/// @param max 最大値
	/// @return 疑似乱数
	template<typename T>
	static T generate(const T &min, const T &max) {
		std::mt19937 mt{ std::random_device{}() };
		if constexpr (std::is_integral_v<T>) {
			std::uniform_int_distribution<T> uid(min, max);
			return uid(mt);
		} else if constexpr (std::is_floating_point_v<T>) {
			std::uniform_real_distribution<T> urd(min, max);
			return urd(mt);
		} else {
			static_assert(false_v<T>);
		}
	}

	/// @brief 疑似乱数生成器 (2次元ベクトル)
	/// @param min 最小値
	/// @param max 最大値
	/// @return 疑似乱数
	static Vector2 generate(const Vector2& min, const Vector2& max) {
		return { generate(min.x, max.x), generate(min.y, max.y) };
	}

	/// @brief 疑似乱数生成器 (3次元ベクトル)
	/// @param min 最小値
	/// @param max 最大値
	/// @return 疑似乱数
	static Vector3 generate(const Vector3& min, const Vector3& max) {
		return {
			generate(min.x, max.x),
			generate(min.y, max.y),
			generate(min.z, max.z)
		};
	}

	/// @brief 疑似乱数生成器 (4次元ベクトル)
	/// @param min 最小値
	/// @param max 最大値
	/// @return 疑似乱数
	static Vector4 generate(const Vector4& min, const Vector4& max) {
		return {
			generate(min.x, max.x),
			generate(min.y, max.y),
			generate(min.z, max.z),
			generate(min.w, max.w)
		};
	}
};
