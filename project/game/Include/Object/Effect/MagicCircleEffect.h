#pragma once
#include <cstdint>

class Registry;
class IndirectCommandManager;
class PrimitiveGenerator;
class ObjectManager;

/// @brief 魔法陣エフェクト
class MagicCircleEffect {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param indirectCommandManager 間接コマンドマネージャー
	/// @param primitiveGenerator プリミティブジェネレーター
	/// @param objectManager オブジェクトマネージャー
	MagicCircleEffect(Registry *registry, IndirectCommandManager *indirectCommandManager, PrimitiveGenerator *primitiveGenerator, ObjectManager *objectManager)
		: registry_(registry), indirectCommandManager_(indirectCommandManager), primitiveGenerator_(primitiveGenerator), objectManager_(objectManager) {
	}

	/// @brief 初期化
	void Initialize();

	/// @brief 更新
	void Update();

private:
	Registry *registry_ = nullptr;								// レジストリ
	IndirectCommandManager *indirectCommandManager_ = nullptr;	// 間接コマンドマネージャー
	PrimitiveGenerator *primitiveGenerator_ = nullptr;			// プリミティブジェネレーター
	ObjectManager *objectManager_ = nullptr;					// オブジェクトマネージャー
	uint32_t entity_ = 0;										// エンティティ
};