#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <vector>

/// @brief フットプリント
struct Footprint final {
	uint32_t id = 0;	// ID
	Vector4 color;		// 色
};

/// @brief 一度きりのフットプリント
struct OnceFootprint final {};

class Registry;

/// @brief フットプリントマネージャー
class FootprintManager final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	FootprintManager(Registry *registry) : registry_(registry) {}

	/// @brief フットプリントの作成
	/// @param entity エンティティ
	/// @param color 色
	Footprint CreateFootprint(uint32_t entity, const Vector4 &color = {});

	/// @brief フットプリントの削除
	/// @param entity エンティティ
	void RemoveFootprint(uint32_t entity);

	/// @brief 一度きりのフットプリントの削除
	void RemoveOnceFootprint();

	/// @brief フットプリント数のデバッグ表示
	void Debug() const;

	/// @brief 現在のフットプリントインデックスの取得
	/// @return フットプリントインデックス
	uint32_t GetCurrentIndex() const { return static_cast<uint32_t>(entities_.size()); }

private:
	Registry *registry_ = nullptr;		// レジストリ
	std::vector<uint32_t> entities_;	// インデックスからエンティティへのマッピング
};

/// @brief フットプリントインスペクター
class FootprintInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	FootprintInspector(Registry *registry) : registry_(registry) {}

	/// @brief フットプリントインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;	// レジストリ
};