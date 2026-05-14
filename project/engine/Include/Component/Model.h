#pragma once
#include "Animation.h"
#include "Matrix4x4.h"
#include "Mesh.h"
#include <map>
#include <assimp/scene.h>

/// @brief ノード
struct Node final {
	std::string name;			// ノード名
	Vector3 translation;		// 平行移動
	Quaternion rotation;		// 回転（クォータニオン）
	Vector3 scale;				// スケーリング
	std::vector<Node> children;	// 子ノードリスト
};

/// @brief マテリアルデータ
struct MaterialData final {
	std::string textureFilePath;	// テクスチャファイルパス
};

/// @brief モデルデータ
struct ModelData final {
	std::string format;						// フォーマット名
	std::vector<MeshData> meshes;			// メッシュデータ
	std::vector<MaterialData> materials;	// マテリアルデータ
	std::vector<AnimationClip> animations;	// アニメーションクリップ
	Node rootNode;							// ルートノード
};

/// @brief モデル
struct Model final {
	ModelData modelData;					// モデルデータ
	std::vector<uint32_t> textureHandle;	// テクスチャハンドル
	std::vector<bool> enableMipMaps;		// ミップマップ有効フラグ
	std::string name;						// モデル名
};

/// @brief カリングコンポーネント
struct UseCulling final {};

class TextureManager;
class MeshManager;

/// @brief モデルマネージャー
class ModelManager final {
public:
	/// @brief コンストラクタ
	/// @param textureManager テクスチャマネージャー
	/// @param meshManager メッシュマネージャー
	/// @param registry レジストリ
	/// @param logStream ログストリーム
	ModelManager(TextureManager *textureManager, MeshManager *meshManager, std::ofstream *logStream)
		: textureManager_(textureManager)
		, meshManager_(meshManager)
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

	/// @brief ローカル行列の作成
	/// @param node ノード
	/// @return ローカル行列
	static Matrix4x4 MakeLocalMatrix(const Node &node);

private:
	TextureManager *textureManager_ = nullptr;				// テクスチャマネージャー
	MeshManager *meshManager_ = nullptr;					// メッシュマネージャー
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