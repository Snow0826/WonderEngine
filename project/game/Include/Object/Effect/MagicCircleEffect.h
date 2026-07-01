#pragma once
#include <cstdint>

class Registry;
class IndirectCommandManager;
class CylinderGenerator;
class ObjectManager;

/// @brief 魔法陣エフェクト
class MagicCircleEffect {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param indirectCommandManager 間接コマンドマネージャー
	/// @param cylinderGenerator 円柱ジェネレーター
	/// @param objectManager オブジェクトマネージャー
	MagicCircleEffect(Registry *registry, IndirectCommandManager *indirectCommandManager, CylinderGenerator *cylinderGenerator, ObjectManager *objectManager)
		: registry_(registry), indirectCommandManager_(indirectCommandManager), cylinderGenerator_(cylinderGenerator), objectManager_(objectManager) {
	}

	/// @brief 初期化
	void Initialize();

	/// @brief 更新
	void Update();

private:
	Registry *registry_ = nullptr;								// レジストリ
	IndirectCommandManager *indirectCommandManager_ = nullptr;	// 間接コマンドマネージャー
	CylinderGenerator *cylinderGenerator_ = nullptr;			// 円柱ジェネレーター
	ObjectManager *objectManager_ = nullptr;					// オブジェクトマネージャー
	uint32_t entity_ = 0;										// エンティティ
};