#pragma once
#include <cstdint>

class Registry;
class CylinderGenerator;
class ObjectManager;
class IndirectCommandManager;
struct Vector3;

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
	/// @param start 開始位置
	/// @param direction 方向
	/// @param divide 分割数
	/// @param topRadius 上面の半径
	/// @param bottomRadius 下面の半径
	/// @param length 長さ
	/// @param depth 深さ
	void Generate(const Vector3 &start, const Vector3 &direction, uint32_t divide, float topRadius, float bottomRadius, float length, int32_t depth);

private:
	Registry *registry_ = nullptr;
	CylinderGenerator *cylinderGenerator_ = nullptr;
	ObjectManager *objectManager_ = nullptr;
	IndirectCommandManager *indirectCommandManager_ = nullptr;
};