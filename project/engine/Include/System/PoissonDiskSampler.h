#pragma once
#include <vector>
#include <Vector2.h>

/// @brief ポアソンディスクサンプリング
class PoissonDiskSampler {
public:
	/// @brief ポアソンディスクサンプリングを生成する関数
	/// @param width 横幅
	/// @param height 縦幅
	/// @param radius 半径
	/// @param attempts 試行回数
	/// @return サンプル点のリスト
    std::vector<Vector2> Generate(float width, float height, float radius, uint32_t attempts = 30);

private:
	float width_ = 0.0f;	// 横幅
	float height_ = 0.0f;	// 縦幅
	float radius_ = 0.0f;	// 半径
	float cellSize_ = 0.0f;	// セルサイズ
	int32_t gridWidth_ = 0;	// グリッドの横幅
	int32_t gridHeight_ = 0;	// グリッドの縦幅
	std::vector<Vector2> samples_;	// サンプル点のリスト

    // Gridにはsampleのindexを入れる
    std::vector<int32_t> grid_;

	/// @brief 中心点から半径内のランダムな点を生成する関数
	/// @param center 中心点
	/// @return 半径内のランダムな点
    Vector2 GenerateCandidate(const Vector2 &center);

	/// @brief 候補点が有効かどうかを判定する関数
	/// @param candidate 候補点
	/// @return 有効かどうか
    bool IsValid(const Vector2 &candidate);

	/// @brief サンプル点を追加する関数
	/// @param point 追加するサンプル点
    void AddSample(const Vector2 &point);
};