#pragma once
#include <Vector3.h>
#include <Vector4.h>

/// @brief 剰余演算
/// @param x X
/// @param y Y
/// @return 剰余
float Mod(float x, float y);

/// @brief 範囲内に値をラップする
/// @param x 値
/// @param min 最小値
/// @param max 最大値
/// @return ラップ後の値
float Wrap(float x, float min, float max);

/// @brief ほぼ等しいかどうかを判定する
/// @param a 比較対象の値1
/// @param b 比較対象の値2
/// @param epsilon 許容誤差
/// @return ほぼ等しい場合はtrue、それ以外はfalse
bool NearlyEqual(float a, float b, float epsilon = 1e-5f);

/// @brief 量子化する
/// @param value 値
/// @param scale スケール
/// @return 量子化された値
int32_t Quantize(float value, float scale = 10000.0f);

/// @brief Vector4をVector3に変換する
/// @param v 変換するVector4
/// @return 変換後のVector3
Vector3 ToVector3(const Vector4 &v);