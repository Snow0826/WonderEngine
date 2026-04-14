#pragma once
#include "BaseScene.h"
#include <list>
#include <vector>

class Player;
class Enemy;
class CameraController;
class BaseGameScenePhase;

/// @brief ゲームシーン
class GameScene : public BaseScene {
public:
	/// @brief デフォルトコンストラクタ
	GameScene();

	/// @brief デフォルトデストラクタ
	~GameScene() override;

	/// @brief 初期化
	void OnInitialize() override;

	/// @brief 更新
	void OnUpdate() override;

	/// @brief ワールド変換後処理
	void OnAfterTransform() override;

	/// @brief 全ての衝突判定をチェックする
	void CheckAllCollisions() const;

	/// @brief 敵の出現
	void PopEnemy();

	/// @brief 敵の死亡処理
	void DeadEnemy();

	/// @brief クリアフェーズに切り替え
	void ChangeToGameClearPhase();

	/// @brief UIのクリア
	void ClearUI();

	/// @brief フェーズの切り替え
	/// @param newBaseGameScenePhase 新しいフェーズ
	void ChangePhase(BaseGameScenePhase *newBaseGameScenePhase);

	/// @brief メインカメラエンティティの取得
	/// @return メインカメラエンティティ
	uint32_t GetMainCameraEntity() const { return cameraEntities_[mainCameraType_]; }

	/// @brief プレイヤーの取得
	/// @return プレイヤー
	Player *GetPlayer() const { return player_.get(); }

	/// @brief カメラコントローラーの取得
	/// @return カメラコントローラー
	CameraController *GetCameraController() const { return cameraController_.get(); }

	/// @brief フェードの取得
	/// @return フェード
	Fade *GetFade() const { return fade_.get(); }

	/// @brief ルールUIフェードの取得
	/// @return ルールUIフェード
	Fade *GetRuleUIFade() const { return ruleUIFade_.get(); }

	/// @brief ビットマップフォントの取得
	/// @return ビットマップフォント
	BitmapFont *GetBitmapFont() const { return bitmapFont_.get(); }

	/// @brief レジストリの取得
	/// @return レジストリ
	Registry *GetRegistry() const { return registry_.get(); }

	/// @brief シーンマネージャーの取得
	/// @return シーンマネージャー
	SceneManager *GetSceneManager() const { return sceneManager_; }

	/// @brief オブジェクトマネージャーの取得
	/// @return オブジェクトマネージャー
	ObjectManager *GetObjectManager() const { return objectManager_.get(); }

	/// @brief スプライトマネージャーの取得
	/// @return スプライトマネージャー
	SpriteManager *GetSpriteManager() const { return spriteManager_.get(); }

private:
	std::unique_ptr<Player> player_ = nullptr;						// プレイヤー
	std::list<std::unique_ptr<Enemy>> enemies_;						// 敵
	std::unique_ptr<CameraController> cameraController_ = nullptr;	// カメラコントローラー
	std::unique_ptr<Fade> ruleUIFade_ = nullptr;					// ルールUIフェード
	uint32_t uiEntity_ = 0;											// UIエンティティ
	uint32_t xEntity_ = 0;											// Xエンティティ
	uint32_t aEntity_ = 0;											// Aエンティティ
	uint32_t rbEntity_ = 0;											// RBエンティティ
	uint32_t playTimer_ = 0;										// プレイタイマー
	int32_t waitToPopTimer_ = 0;									// 敵出現待機タイマー
	bool isWaitingToPop_ = false;									// 敵出現待機中フラグ
	BaseGameScenePhase *phase_ = nullptr;							// 現在のフェーズ
	static inline constexpr uint32_t kPlayDuration = 1800;					// プレイ時間
	static inline constexpr uint32_t kMaxWaitToPopTime = kPlayDuration / 9;	// 敵出現最大待機時間
	static inline constexpr uint32_t kMaxEnemyCount = 64;					// 最大敵数
};