#pragma once
#include "Vector3.h"
#include "Quaternion.h"
#include <string>
#include <vector>

/// @brief キーフレーム
/// @tparam tValue キーフレームの値の型
template<typename tValue>
struct KeyFrame final {
	float time = 0.0f;	// 時間
	tValue value;		// 値
};

/// @brief 3次元ベクトルのキーフレーム
using KeyFrameVector3 = KeyFrame<Vector3>;

/// @brief クォータニオンのキーフレーム
using KeyFrameQuaternion = KeyFrame<Quaternion>;

/// @brief アニメーションカーブ
/// @tparam tValue アニメーションカーブの値の型
template<typename tValue>
struct AnimationCurve final {
	std::vector<KeyFrame<tValue>> keyframes;	// キーフレームリスト
};

/// @brief ノードアニメーション
struct NodeAnimation final {
	std::string name;					// ノード名
	AnimationCurve<Vector3> translate;	// 位置のキーフレームリスト
	AnimationCurve<Quaternion> rotate;	// 回転のキーフレームリスト
	AnimationCurve<Vector3> scale;		// スケーリングのキーフレームリスト
};

/// @brief アニメーションクリップ
struct AnimationClip final {
	std::string name;							// アニメーション名
	float duration = 0.0f;						// アニメーションの長さ
	std::vector<NodeAnimation> nodeAnimations;	// ノードアニメーションリスト
};

/// @brief アニメーション補間モード
enum class AnimationInterpolationMode {
	Linear,	// 線形補間
	Step	// ステップ補間
};

/// @brief アニメーションプレイヤー
struct AnimationPlayer final {
	uint32_t animationIndex = 0;	// アニメーションインデックス
	float currentTime = 0.0f;		// 現在の時間
	float speed = 1.0f;				// 再生速度
	bool isLoop = true;				// ループ再生するかどうか
};

class Registry;

/// @brief アニメーションシステム
class AnimationSystem final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	AnimationSystem(Registry *registry) : registry_(registry) {}

	/// @brief アニメーションの更新
	/// @param deltaTime デルタタイム
	void Update(float deltaTime);

private:
	Registry *registry_ = nullptr;	// レジストリ

	/// @brief 線形補完による3次元ベクトルのサンプリング
	/// @param keyframes キーフレームリスト
	/// @param time サンプリング時間
	/// @return サンプリング結果の3次元ベクトル
	Vector3 SampleLinearVector3(const std::vector<KeyFrameVector3> &keyframes, float time);

	/// @brief 線形補完によるクォータニオンのサンプリング
	/// @param keyframes キーフレームリスト
	/// @param time サンプリング時間
	/// @return サンプリング結果のクォータニオン
	Quaternion SampleLinearQuaternion(const std::vector<KeyFrameQuaternion> &keyframes, float time);

	/// @brief ステップ補間による3次元ベクトルのサンプリング
	/// @param keyframes キーフレームリスト
	/// @param time サンプリング時間
	/// @return サンプリング結果の3次元ベクトル
	Vector3 SampleStepVector3(const std::vector<KeyFrameVector3> &keyframes, float time);

	/// @brief ステップ補間によるクォータニオンのサンプリング
	/// @param keyframes キーフレームリスト
	/// @param time サンプリング時間
	/// @return サンプリング結果のクォータニオン
	Quaternion SampleStepQuaternion(const std::vector<KeyFrameQuaternion> &keyframes, float time);
};