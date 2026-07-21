#pragma once
#include "Vector3.h"
#include <vector>
#include <memory>

/// @brief 葉（AttractionPoint）
struct Leaf final {
	Vector3 position;		// 位置
	bool reached = false;	// 到達フラグ
};

/// @brief 枝（Branch）
struct Branch final {
	Vector3 position;				// 位置
	Vector3 direction;				// 方向
	float radius = 0.0f;			// 半径
	Branch *parent = nullptr;		// 親の枝
	std::vector<Branch *> children;	// 子の枝
	Vector3 growDirection;			// 成長方向
	uint32_t growCount = 0;			// 成長カウント
};

class Registry;
class PrimitiveGenerator;
class InstanceAllocator;
struct Vector3;
struct Quaternion;

/// @brief 木の生成器
class TreeGenerator {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param primitiveGenerator プリミティブジェネレーター
	/// @param instanceAllocator インスタンスアロケーター
	TreeGenerator(Registry *registry, PrimitiveGenerator *primitiveGenerator, InstanceAllocator *instanceAllocator) : registry_(registry), primitiveGenerator_(primitiveGenerator), instanceAllocator_(instanceAllocator) {}

	/// @brief 木の生成
	/// @param rootPosition 根の位置
	/// @param rootDirection 根の方向
	/// @param crownCenter 葉の生成範囲の中心
	/// @param crownRadius 葉の生成範囲の半径
	/// @param leafCount 葉の数
	/// @param minRadius 枝の最小半径
	/// @param gamma 枝の半径計算のガンマ値
	/// @param influenceRadius 影響半径
	/// @param killRadius 消滅半径
	/// @param branchLength 枝の長さ
	/// @return 生成された木のエンティティID
	uint32_t Generate(const Vector3 &rootPosition, const Vector3 &rootDirection, const Vector3 &crownCenter, const Vector3 &crownRadius, uint32_t leafCount, float minRadius, float gamma, float influenceRadius, float killRadius, float branchLength);

	/// @brief 木の削除
	/// @param entity 削除する木のエンティティID
	void Delete(uint32_t entity);

private:
	Registry *registry_ = nullptr;						// レジストリ
	PrimitiveGenerator *primitiveGenerator_ = nullptr;	// プリミティブジェネレーター
	InstanceAllocator *instanceAllocator_ = nullptr;	// インスタンスアロケーター
	std::vector<Leaf> leaves_;							// 葉のリスト
	std::vector<std::unique_ptr<Branch>> branches_;		// 枝のリスト

	/// @brief 葉の生成
	/// @param crownCenter 葉の生成範囲の中心
	/// @param crownRadius 葉の生成範囲の半径
	/// @param leafCount 葉の数
	void GenerateLeaves(const Vector3 &crownCenter, const Vector3 &crownRadius, uint32_t leafCount);

	/// @brief 根の枝の生成
	/// @param rootPosition 根の位置
	/// @param rootDirection 根の方向
	void GenerateRootBranch(const Vector3 &rootPosition, const Vector3 &rootDirection);

	/// @brief 最も近い枝を見つける
	/// @param influenceRadius 影響半径
	/// @param killRadius 消滅半径
	void FindClosestBranch(float influenceRadius, float killRadius);

	/// @brief 枝を成長させる
	/// @param branchLength 枝の長さ
	void GrowBranches(float branchLength);

	/// @brief 葉を削除する
	void RemoveLeaves();

	/// @brief 枝の半径を計算する
	/// @param branch 枝
	/// @param minRadius 最小半径
	/// @param gamma ガンマ値
	/// @return 枝の半径
	float CalculateRadius(Branch *branch, float minRadius, float gamma);

	/// @brief 枝を再帰的に作成する
	/// @param branch 枝
	/// @param parentEntity 親のエンティティID
	/// @param parentWorldRotation 親のワールド回転
	/// @param branchLength 枝の長さ
	/// @return 作成された枝のエンティティID
	uint32_t CreateBranchRecursive(Branch *branch, uint32_t parentEntity, const Quaternion &parentWorldRotation, float branchLength);
};