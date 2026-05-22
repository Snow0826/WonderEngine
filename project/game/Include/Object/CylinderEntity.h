#pragma once
#include <cstdint>

class Registry;
class CylinderGenerator;
class ObjectManager;
class IndirectCommandManager;

/// @brief 円柱エンティティ
class CylinderEntity {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param cylinderGenerator 円柱ジェネレーター
	/// @param objectManager オブジェクトマネージャー
	/// @param indirectCommandManager 間接コマンドマネージャー
	CylinderEntity(Registry *registry, CylinderGenerator *cylinderGenerator, ObjectManager *objectManager, IndirectCommandManager *indirectCommandManager)
		: registry_(registry),
		cylinderGenerator_(cylinderGenerator),
		objectManager_(objectManager),
		indirectCommandManager_(indirectCommandManager) {
	}

	/// @brief 円柱の作成
	/// @param divide 分割数
	/// @param topRadius 上面の半径
	/// @param bottomRadius 下面の半径
	/// @param height 高さ
	void Create(uint32_t divide, float topRadius, float bottomRadius, float height);

private:
	Registry *registry_ = nullptr;
	CylinderGenerator *cylinderGenerator_ = nullptr;
	ObjectManager *objectManager_ = nullptr;
	IndirectCommandManager *indirectCommandManager_ = nullptr;
};