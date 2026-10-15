#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <random>
#include <type_traits>

template <typename T>
constexpr bool false_v = false;

/// @brief 範囲を表す構造体
/// @tparam T 範囲の型
template <typename T>
struct Range final {
	T min;	// 最小値
	T max;	// 最大値
};

/// @brief 疑似乱数生成
struct Random final {
	/// @brief 指定した乱数エンジンから値を生成
	/// @tparam T 整数または浮動小数点数
	template<typename T>
	static T Generate(std::mt19937 &engine, const T &min, const T &max) {
		if constexpr (std::is_integral_v<T>) {
			return std::uniform_int_distribution<T>(min, max)(engine);
		} else if constexpr (std::is_floating_point_v<T>) {
			return std::uniform_real_distribution<T>(min, max)(engine);
		} else {
			static_assert(false_v<T>);
		}
	}

	/// @brief 疑似乱数生成器
	/// @tparam T 疑似乱数の型
	/// @param min 最小値
	/// @param max 最大値
	/// @return 疑似乱数
	template<typename T>
	static T Generate(const T &min, const T &max) {
		static thread_local std::mt19937 engine{ std::random_device{}() };
		return Generate(engine, min, max);
	}

	/// @brief 疑似乱数生成器 (2次元ベクトル)
	/// @param engine 乱数エンジン
	/// @param min 最小値
	/// @param max 最大値
	/// @return 疑似乱数
	static Vector2 Generate(std::mt19937 &engine, const Vector2 &min, const Vector2 &max) {
		return { Generate(engine, min.x, max.x), Generate(engine, min.y, max.y) };
	}

	/// @brief 疑似乱数生成器 (3次元ベクトル)
	/// @param engine 乱数エンジン
	/// @param min 最小値
	/// @param max 最大値
	/// @return 疑似乱数
	static Vector3 Generate(std::mt19937 &engine, const Vector3 &min, const Vector3 &max) {
		return { Generate(engine, min.x, max.x), Generate(engine, min.y, max.y), Generate(engine, min.z, max.z) };
	}

	/// @brief 疑似乱数生成器 (4次元ベクトル)
	/// @param engine 乱数エンジン
	/// @param min 最小値
	/// @param max 最大値
	/// @return 疑似乱数
	static Vector4 Generate(std::mt19937 &engine, const Vector4 &min, const Vector4 &max) {
		return { Generate(engine, min.x, max.x), Generate(engine, min.y, max.y), Generate(engine, min.z, max.z), Generate(engine, min.w, max.w) };
	}

	/// @brief 疑似乱数生成器 (2次元ベクトル)
	/// @param min 最小値
	/// @param max 最大値
	/// @return 疑似乱数
	static Vector2 Generate(const Vector2 &min, const Vector2 &max) {
		return { Generate(min.x, max.x), Generate(min.y, max.y) };
	}

	/// @brief 疑似乱数生成器 (3次元ベクトル)
	/// @param min 最小値
	/// @param max 最大値
	/// @return 疑似乱数
	static Vector3 Generate(const Vector3 &min, const Vector3 &max) {
		return { Generate(min.x, max.x), Generate(min.y, max.y), Generate(min.z, max.z) };
	}

	/// @brief 疑似乱数生成器 (4次元ベクトル)
	/// @param min 最小値
	/// @param max 最大値
	/// @return 疑似乱数
	static Vector4 Generate(const Vector4 &min, const Vector4 &max) {
		return { Generate(min.x, max.x), Generate(min.y, max.y), Generate(min.z, max.z), Generate(min.w, max.w) };
	}
};