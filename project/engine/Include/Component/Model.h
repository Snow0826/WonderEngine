#pragma once
#include "Animation.h"
#include "Mesh.h"
#include "Transform.h"
#include <map>
#include <assimp/scene.h>
#include <optional>

/// @brief ノード
struct Node final {
	QuaternionTransform transform;	// 変換
	std::string name;				// ノード名
	std::vector<Node> children;		// 子ノードリスト
};

/// @brief ジョイント
struct Joint final {
	QuaternionTransform transform;	// 変換
	std::string name;				// ジョイント名
	Matrix4x4 skeletonSpaceMatrix;	// スケルトンスペース行列
	std::vector<int32_t> children;	// 子ジョイントインデックスリスト
	int32_t index = 0;				// ジョイントインデックス
	std::optional<int32_t> parent;	// 親ジョイントインデックス
};

/// @brief スケルトン
struct Skeleton final {
	int32_t root = 0;	// ルートジョイントインデックス
	std::map<std::string, int32_t> jointIndexByName;	// ジョイント名からジョイントインデックスへのマップ
	std::vector<Joint> joints;	// ジョイントリスト
};

/// @brief スキンメッシュタグ
struct SkinMesh final{};

/// @brief 頂点ウェイトデータ
struct VertexWeightData final {
	float weight;			// ウェイト
	uint32_t vertexIndex;	// 頂点インデックス
};

/// @brief ジョイントウェイトデータ
struct JointWeightData final {
	Matrix4x4 inverseBindPoseMatrix;	// 逆バインドポーズ行列
	std::vector<VertexWeightData> vertexWeights;	// 頂点ウェイトデータリスト
};

/// @brief マテリアルデータ
struct MaterialData final {
	std::string textureFilePath;	// テクスチャファイルパス
};

/// @brief モデルデータ
struct ModelData final {
	std::string format;	// フォーマット名
	std::vector<MeshData> meshes;	// メッシュデータ
	std::vector<MaterialData> materials;	// マテリアルデータ
	std::vector<AnimationClip> animations;	// アニメーションクリップ
	std::map<std::string, JointWeightData> skinClusterData;	// スキンクラスター名からジョイントウェイトデータへのマップ
	Skeleton skeleton;	// スケルトン
	Node rootNode;	// ルートノード
};

/// @brief モデル
struct Model final {
	ModelData modelData;					// モデルデータ
	std::vector<uint32_t> textureHandle;	// テクスチャハンドル
	std::vector<bool> enableMipMaps;		// ミップマップ有効フラグ
	uint32_t skinClusterHandle = 0;			// スキンクラスターハンドル
	std::string name;						// モデル名
};

class TextureManager;
class MeshManager;
class SkinClusterManager;

/// @brief モデルマネージャー
class ModelManager final {
public:
	/// @brief コンストラクタ
	/// @param textureManager テクスチャマネージャー
	/// @param meshManager メッシュマネージャー
	/// @param skinClusterManager スキンクラスターマネージャー
	/// @param registry レジストリ
	/// @param logStream ログストリーム
	ModelManager(TextureManager *textureManager, MeshManager *meshManager, SkinClusterManager *skinClusterManager, std::ofstream *logStream)
		: textureManager_(textureManager)
		, meshManager_(meshManager)
		, skinClusterManager_(skinClusterManager)
		, logStream_(logStream) {
	}

	/// @brief モデルの読み込み
	/// @param fileName モデル名
	void LoadModel(const std::string &fileName);

	/// @brief モデルの検索
	/// @param fileName モデル名
	/// @return モデル
	Model FindModel(const std::string &fileName);

	/// @brief モデルコンボボックスの表示
	/// @param label ラベル
	/// @param model モデル
	/// @return 選択されたかどうか
	bool Combo(const std::string &label, Model *model);

	/// @brief スケルトンの更新
	/// @param skeleton スケルトン
	static void UpdateSkeleton(Skeleton &skeleton);

	/// @brief ローカル行列の作成
	/// @param node ノード
	/// @return ローカル行列
	static Matrix4x4 MakeLocalMatrix(const Node &node);

private:
	TextureManager *textureManager_ = nullptr;				// テクスチャマネージャー
	MeshManager *meshManager_ = nullptr;					// メッシュマネージャー
	SkinClusterManager *skinClusterManager_ = nullptr;		// スキンクラスターマネージャー
	std::ofstream *logStream_ = nullptr;					// ログ出力用のストリーム
	std::map<std::string, std::unique_ptr<Model>> models_;	// 読み込んだモデルのマップ

	/// @brief モデルデータの読み込み
	/// @param fileName モデル名
	/// @return モデルデータ
	ModelData LoadModelData(const std::string &fileName);

	/// @brief ノードの読み込み
	/// @param node Assimpのノード
	/// @return ノード
	static Node ReadNode(const aiNode *node);

	/// @brief スケルトンの作成
	/// @param rootNode ルートノード
	/// @return スケルトン
	static Skeleton CreateSkeleton(const Node &rootNode);

	/// @brief ジョイントの作成
	/// @param node ノード
	/// @param parent 親ジョイントインデックス
	/// @param joints ジョイントリスト
	/// @return ジョイントインデックス
	static int32_t CreateJoint(const Node &node, const std::optional<int32_t> &parent, std::vector<Joint> &joints);
};

class Registry;
class IndirectCommandManager;

/// @brief モデルインスペクター
class ModelInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param modelManager モデルマネージャー
	/// @param indirectCommandManager 間接コマンドマネージャー
	ModelInspector(Registry *registry, ModelManager *modelManager, IndirectCommandManager *indirectCommandManager)
		: registry_(registry)
		, modelManager_(modelManager)
		, indirectCommandManager_(indirectCommandManager) {
	}

	/// @brief モデルインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;								// レジストリ
	ModelManager *modelManager_ = nullptr;						// モデルマネージャー
	IndirectCommandManager *indirectCommandManager_ = nullptr;	// 間接コマンドマネージャー
};