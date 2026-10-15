#pragma once
#include "Vector3.h"
#include "CPUTimer.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <random>

/// @brief 樹冠形状
enum class CrownShape {
	Sphere,			// 球形
	Ellipsoid,		// 楕円体
	Dome,			// ドーム形
	Cone,			// 円錐形
	RoundedCone,	// 丸みを帯びた円錐形
	Cylinder,		// 円柱形
	Vase			// 花瓶形
};

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
	uint32_t generation = 0;		// 根からの世代数
	Branch *parent = nullptr;		// 親の枝
	std::vector<Branch *> children;	// 子の枝
	Vector3 growDirection;			// 成長方向
	uint32_t growCount = 0;			// 成長カウント
};

/// @brief グリッドセル
struct GridCell final {
	int32_t x = 0;	// X座標
	int32_t y = 0;	// Y座標
	int32_t z = 0;	// Z座標

	bool operator==(const GridCell &other) const {
		return x == other.x && y == other.y && z == other.z;
	}
};

/// @brief グリッドセルのハッシュ関数
struct GridCellHash final {
	size_t operator()(const GridCell &cell) const noexcept {
		size_t h1 = std::hash<int32_t>{}(cell.x);
		size_t h2 = std::hash<int32_t>{}(cell.y);
		size_t h3 = std::hash<int32_t>{}(cell.z);
		return h1 ^
			(h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2)) ^
			(h3 + 0x9e3779b9 + (h2 << 6) + (h2 >> 2));
	}
};

/// @brief 枝のデータ
struct BranchData final {
	float bottomRadius = 0.0f;	// 下半径
	float topRadius = 0.0f;		// 上半径
	float inverseLength = 0.0f;	// 長さの逆数
};

/// @brief 木のメンバー
struct TreeMember final {
	uint32_t treeId = 0;	// 木のID
};

/// @brief 木の境界ボックス
struct TreeBounds final {
	uint32_t treeId = 0;	// 木のID
	Vector3 min;	// 最小座標
	Vector3 max;	// 最大座標
};

/// @brief 樹冠
struct TreeCrown {
	Vector3 center = { 0.0f, 5.0f, 0.0f };	// 中心位置
	Vector3 radius = { 5.0f, 5.0f, 5.0f };	// 半径
	CrownShape shape = CrownShape::Ellipsoid;	// 樹冠形状
	float surfaceBias = 0.0f;	// 樹冠表面へのバイアス
	float topBias = 0.0f;		// 樹冠上端へのバイアス
	float upwardBias = 0.0f;	// 樹冠上方向へのバイアス
	float outwardBias = 0.0f;	// 樹冠外方向へのバイアス
	float downwardBias = 0.0f;	// 樹冠下方向へのバイアス
};

/// @brief 木の設定
struct TreeConfig {
	Vector3 rootPosition;	// 根の位置
	Vector3 rootDirection = { 0.0f, 1.0f, 0.0f };	// 根の方向
	TreeCrown crown;	// 樹冠
	uint32_t leafCount = 5000;	// 葉の数
	float minRadius = 0.01f;	// 枝の最小半径
	float branchRadius = 0.18f;	// 基準となる枝半径
	float tipRadius = 0.018f;	// 枝先の最小半径
	bool useRadiusTaper = false;	// 世代に応じた半径テーパーを使う
	float gamma = 2.0f;	// 枝の半径計算のガンマ値
	float influenceRadius = 16.0f;	// 影響半径
	float killRadius = 1.6f;	// 消滅半径
	float branchLength = 0.3f;	// 枝の長さ
	float trunkHeight = 0.0f;	// 根から樹冠下端までの幹の高さ。0なら幹を追加しない
	uint32_t randomSeed = 0; // 0なら毎回異なるseed
	uint32_t maxIterations = 0; // 0なら進捗停止まで
	uint32_t maxBranches = 0; // 0なら上限なし
};

class Registry;
class PrimitiveGenerator;
class InstanceAllocator;
struct Vector3;
struct Quaternion;
struct Primitive;

/// @brief 木の生成器
class TreeGenerator {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param primitiveGenerator プリミティブジェネレーター
	/// @param instanceAllocator インスタンスアロケーター
	/// @param logStream ログストリーム
	TreeGenerator(Registry *registry, PrimitiveGenerator *primitiveGenerator, InstanceAllocator *instanceAllocator, std::ofstream *logStream) : registry_(registry), primitiveGenerator_(primitiveGenerator), instanceAllocator_(instanceAllocator), logStream_(logStream) {}

	/// @brief 木の生成
	/// @param tree 木の設定
	/// @return 生成された木のエンティティID
	uint32_t Generate(const TreeConfig &tree);

	/// @brief 木の削除
	/// @param entity 削除する木のエンティティID
	void Delete(uint32_t entity);

private:
	Registry *registry_ = nullptr;						// レジストリ
	PrimitiveGenerator *primitiveGenerator_ = nullptr;	// プリミティブジェネレーター
	InstanceAllocator *instanceAllocator_ = nullptr;	// インスタンスアロケーター
	std::ofstream *logStream_ = nullptr;				// ログストリーム
	std::vector<Leaf> leaves_;							// 葉のリスト
	std::vector<std::unique_ptr<Branch>> branches_;		// 枝のリスト
	std::unordered_map<GridCell, std::vector<Branch *>, GridCellHash> branchGrid_;	// グリッドセルごとの枝のマップ
	float gridCellSize_ = 1.0f;	// グリッドセルのサイズ
	TreeCrown currentCrown_;	// 現在の樹冠
	std::mt19937 randomEngine_;	// 乱数生成器
	CPUTimer timer_;	// CPUタイマー
	
	/// @brief 葉の生成
	/// @param crownCenter 葉の生成範囲の中心
	/// @param crownRadius 葉の生成範囲の半径
	/// @param leafCount 葉の数
	void GenerateLeaves(const Vector3 &crownCenter, const Vector3 &crownRadius, uint32_t leafCount);

	/// @brief 根の枝の生成
	/// @param rootPosition 根の位置
	/// @param rootDirection 根の方向
	void GenerateRootBranch(const Vector3 &rootPosition, const Vector3 &rootDirection);

	/// @brief 幹の生成
	/// @param trunkHeight 幹の高さ
	/// @param branchLength 枝の長さ
	/// @param rootDirection 根の方向
	void GenerateTrunk(float trunkHeight, float branchLength, const Vector3 &rootDirection);

	/// @brief 最も近い枝を見つける
	/// @param influenceRadius 影響半径
	/// @param killRadius 消滅半径
	void FindClosestBranch(float influenceRadius, float killRadius);

	/// @brief グリッド内で最も近い枝を見つける
	/// @param position 位置
	/// @param influenceRadius 影響半径
	/// @param killRadius 消滅半径
	/// @return 最も近い枝のポインタ、見つからなかった場合はnullptr
	Branch* FindClosestBranchInGrid(const Vector3 &position, float influenceRadius, float killRadius);

	/// @brief 枝を成長させる
	/// @param branchLength 枝の長さ
	/// @param maxBranches 最大枝数
	void GrowBranches(float branchLength, uint32_t maxBranches);

	/// @brief 葉を削除する
	void RemoveLeaves();

	/// @brief 枝の半径を計算する
	/// @param branch 枝
	/// @param tree 木の設定
	/// @return 枝の半径
	float CalculateRadius(Branch *branch, const TreeConfig &tree);

	/// @brief 位置からグリッドセルを取得
	/// @param position 位置
	/// @return グリッドセル
	GridCell GetGridCell(const Vector3 &position) const;

	/// @brief 枝をグリッドに追加する
	/// @param branch 枝
	void AddBranchToGrid(Branch *branch);

	/// @brief 枝を再帰的に作成する
	/// @param branch 枝
	/// @param parentEntity 親のエンティティID
	/// @param parentWorldRotation 親のワールド回転
	/// @param branchLength 枝の長さ
	/// @param branchPrimitive 枝のプリミティブ
	/// @param treeId 木のID
	/// @return 作成された枝のエンティティID
	uint32_t CreateBranchRecursive(Branch *branch, uint32_t parentEntity, const Quaternion &parentWorldRotation, float branchLength, const Primitive &branchPrimitive, uint32_t treeId);

	/// @brief 葉を作成する
	/// @param parentEntity 親のエンティティID
	/// @param treeId 木のID
	/// @return 作成された葉のエンティティID
	uint32_t CreateLeaf(uint32_t parentEntity, uint32_t treeId);
};
