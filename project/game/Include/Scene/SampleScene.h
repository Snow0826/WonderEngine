#pragma once
#include "BaseScene.h"
#include <vector>

class Player;

/// @brief サンプルシーン
class SampleScene : public BaseScene {
public:
	/// @brief コンストラクタ
	SampleScene();

	/// @brief デストラクタ
	~SampleScene() override;

	/// @brief 初期化
	void OnInitialize() override;

	/// @brief 更新
	/// @param deltaTime デルタタイム
	void OnUpdate(float deltaTime) override;

private:
	std::unique_ptr<DebugCamera> mainCamera_;	// メインカメラ
	std::vector<uint32_t> treeEntities_;		// 木のエンティティIDのリスト
	uint32_t softBodyEntity_ = 0;				// ソフトボディのエンティティID
	std::unique_ptr<Player> player_;			// プレイヤー
};