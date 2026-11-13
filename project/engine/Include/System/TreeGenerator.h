#pragma once
#include "Vector3.h"
#include "CPUTimer.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <random>

/// @brief 樹種
enum class TreeSpecies {
	Zelkova,	// ケヤキ
	Kunugi,		// クヌギ
	Konara,		// コナラ
	Beech,		// ブナ
	Cedar,		// スギ
	Pine,		// マツ
	Willow		// ヤナギ
};

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
	Vector3 position;	// 位置
	Vector3 direction = { 0.0f, 1.0f, 0.0f };	// 方向
	float radius = 0.0f;	// 半径
	uint32_t generation = 0;	// 根からの世代数
	Branch *parent = nullptr;	// 親の枝
	std::vector<Branch *> children;	// 子の枝
	Vector3 growDirection;	// 成長方向
	uint32_t growCount = 0;	// 成長カウント
};

/// @brief 枝の経路上の点
struct BranchPathPoint final {
	Vector3 position;		// 位置
	Vector3 direction;		// 方向
	float radius = 0.0f;	// 半径
	float distance = 0.0f;	// 経路に沿った累積距離
};

/// @brief 枝の経路
struct BranchPath final {
	std::vector<BranchPathPoint> points;	// 経路上の点
};

/// @brief 枝のメッシュ
struct BranchMesh final {
	std::vector<BranchPath> paths;	// 枝の経路
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

/// @brief 樹冠
struct TreeCrown final {
	Vector3 center;	// 中心位置
	Vector3 radius;	// 半径
	CrownShape shape = CrownShape::Ellipsoid;	// 樹冠形状
	float surfaceBias = 0.0f;	// 樹冠表面へのバイアス
	float topBias = 0.0f;		// 樹冠上端へのバイアス
	float upwardBias = 0.0f;	// 樹冠上方向へのバイアス
	float outwardBias = 0.0f;	// 樹冠外方向へのバイアス
	float downwardBias = 0.0f;	// 樹冠下方向へのバイアス
};

/// @brief 木の設定
struct TreeConfig final {
	TreeSpecies species = TreeSpecies::Zelkova;	// 樹種
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

class PrimitiveGenerator;
struct Vector3;
struct Quaternion;
struct Primitive;

/// @brief 木の生成器
class TreeGenerator {
public:
	/// @brief コンストラクタ
	/// @param primitiveGenerator プリミティブジェネレーター
	/// @param logStream ログストリーム
	TreeGenerator(PrimitiveGenerator *primitiveGenerator, std::ofstream *logStream) : primitiveGenerator_(primitiveGenerator), logStream_(logStream) {}

	/// @brief 木の生成
	/// @param tree 木の設定
	/// @return 生成された木のプリミティブ
	Primitive Generate(const TreeConfig &tree);

private:
	PrimitiveGenerator *primitiveGenerator_ = nullptr;	// プリミティブジェネレーター
	std::ofstream *logStream_ = nullptr;				// ログストリーム
	std::vector<Leaf> leaves_;							// 葉のリスト
	std::vector<std::unique_ptr<Branch>> branches_;		// 枝のリスト
	std::unordered_map<GridCell, std::vector<Branch *>, GridCellHash> branchGrid_;	// グリッドセルごとの枝のマップ
	float gridCellSize_ = 1.0f;	// グリッドセルのサイズ
	std::mt19937 randomEngine_;	// 乱数生成器
	uint32_t treeMeshCounter_ = 0;	// 木のメッシュカウンター
	CPUTimer timer_;	// CPUタイマー

	/// @brief 葉の生成
	/// @param crown 樹冠
	/// @param leafCount 葉の数
	void GenerateLeaves(const TreeCrown &crown, uint32_t leafCount);

	/// @brief 根の枝の生成
	void GenerateRootBranch();

	/// @brief 幹の生成
	/// @param trunkHeight 幹の高さ
	/// @param branchLength 枝の長さ
	void GenerateTrunk(float trunkHeight, float branchLength);

	/// @brief 最も近い枝を見つける
	/// @param influenceRadius 影響半径
	/// @param killRadius 消滅半径
	/// @param crown 樹冠
	void FindClosestBranch(float influenceRadius, float killRadius, const TreeCrown &crown);

	/// @brief グリッド内で最も近い枝を見つける
	/// @param position 位置
	/// @param influenceRadius 影響半径
	/// @param killRadius 消滅半径
	/// @return 最も近い枝のポインタ、見つからなかった場合はnullptr
	Branch *FindClosestBranchInGrid(const Vector3 &position, float influenceRadius, float killRadius);

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

	/// @brief 枝のメッシュを作成する
	/// @return 作成された枝のメッシュ
	BranchMesh CreateBranchMesh();

	/// @brief 枝の経路を作る
	/// @param start 最初の枝
	/// @param firstChild 最初の子の枝
	/// @param result 枝の経路
	void TraceBranchPath(Branch *start, Branch *firstChild, BranchMesh &result);

	/// @brief 枝の経路上の点を作成する
	/// @param branch 枝
	/// @return 作成された枝の経路上の点
	BranchPathPoint MakeBranchPathPoint(const Branch *branch);

	/// @brief Hermite曲線の長さを近似する
	/// @param startPosition 始点
	/// @param endPosition 終点
	/// @param startTangent 始点の接線
	/// @param endTangent 終点の接線
	/// @param samples サンプル数
	/// @return 近似された長さ
	float ApproximateHermiteLength(const Vector3 &startPosition, const Vector3 &endPosition, const Vector3 &startTangent, const Vector3 &endTangent, uint32_t samples = 16);
};

class Registry;
class TreeEntityManager;

/// @brief 木の設定インスペクター
class TreeConfigInspector {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param treeGenerator 木の生成器
	/// @param treeEntityManager 木のエンティティマネージャー
	TreeConfigInspector(Registry *registry, TreeGenerator *treeGenerator, TreeEntityManager *treeEntityManager)
		: registry_(registry)
		, treeGenerator_(treeGenerator)
		, treeEntityManager_(treeEntityManager) {
	}

	/// @brief 木の設定インスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;	// レジストリ
	TreeGenerator *treeGenerator_ = nullptr;	// 木の生成器
	TreeEntityManager *treeEntityManager_ = nullptr;	// 木のエンティティマネージャー
	std::vector<uint32_t> treeEntities_;	// 木のエンティティリスト
};