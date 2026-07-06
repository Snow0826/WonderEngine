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
	Branch* parent = nullptr;		// 親の枝
	std::vector<Branch*> children;	// 子の枝
	Vector3 growDirection;			// 成長方向
	uint32_t growCount = 0;			// 成長カウント
};

class Registry;
class CylinderGenerator;
class ObjectManager;
class IndirectCommandManager;
struct Vector3;
struct Quaternion;

/// @brief 木の生成器
class TreeGenerator {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param cylinderGenerator 円柱ジェネレーター
	/// @param objectManager オブジェクトマネージャー
	/// @param indirectCommandManager 間接コマンドマネージャー
	TreeGenerator(Registry *registry, CylinderGenerator *cylinderGenerator, ObjectManager *objectManager, IndirectCommandManager *indirectCommandManager)
		: registry_(registry),
		cylinderGenerator_(cylinderGenerator),
		objectManager_(objectManager),
		indirectCommandManager_(indirectCommandManager) {
	}

	/// @brief 木の生成
	/// @param leafRadius 葉の半径
	/// @param leafCount 葉の数
	/// @param influenceRadius 影響半径
	/// @param killRadius 消滅半径
	/// @param branchLength 枝の長さ
	/// @return 生成された木のエンティティID
	uint32_t Generate(float leafRadius, uint32_t leafCount, float influenceRadius, float killRadius, float branchLength);

	/// @brief 木の削除
	/// @param entity 削除する木のエンティティID
	void Delete(uint32_t entity);

private:
	Registry *registry_ = nullptr;								// レジストリ
	CylinderGenerator *cylinderGenerator_ = nullptr;			// 円柱ジェネレーター
	ObjectManager *objectManager_ = nullptr;					// オブジェクトマネージャー
	IndirectCommandManager *indirectCommandManager_ = nullptr;	// 間接コマンドマネージャー
	std::vector<Leaf> leaves_;									// 葉のリスト
	std::vector<std::unique_ptr<Branch>> branches_;				// 枝のリスト

	/// @brief 葉の生成
	/// @param leafRadius 葉の半径
	/// @param leafCount 葉の数
	void GenerateLeaves(float leafRadius, uint32_t leafCount);

	/// @brief 根の枝の生成
	/// @param influenceRadius 影響半径
	/// @param branchLength 枝の長さ
	void GenerateRootBranch(float influenceRadius, float branchLength);

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
	/// @return 枝の半径
	float CalculateRadius(Branch *branch);

	/// @brief 枝を再帰的に作成する
	/// @param branch 枝
	/// @param parentEntity 親のエンティティID
	/// @param parentWorldRotation 親のワールド回転
	/// @param branchLength 枝の長さ
	/// @return 作成された枝のエンティティID
	uint32_t CreateBranchRecursive(Branch *branch, uint32_t parentEntity, const Quaternion &parentWorldRotation, float branchLength);
};