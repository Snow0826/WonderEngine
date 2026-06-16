#pragma once
#include <cstdint>

class Registry;
class CylinderGenerator;
class ObjectManager;
class IndirectCommandManager;
struct Vector3;
struct Quaternion;

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
	/// @param parent 親エンティティID
	/// @param start 開始位置
	/// @param direction 方向
	/// @param parentWorldRotate 親のワールド回転
	/// @param divide 分割数
	/// @param topRadius 上面の半径
	/// @param bottomRadius 下面の半径
	/// @param length 長さ
	/// @param depth 深さ
	/// @return 生成された木のエンティティID
	uint32_t Generate(uint32_t parent, const Vector3 &start, const Vector3 &direction, const Quaternion &parentWorldRotate, uint32_t divide, float topRadius, float bottomRadius, float length, int32_t depth);

	/// @brief 木の削除
	/// @param entity 削除する木のエンティティID
	void Delete(uint32_t entity);

private:
	Registry *registry_ = nullptr;								// レジストリ
	CylinderGenerator *cylinderGenerator_ = nullptr;			// 円柱ジェネレーター
	ObjectManager *objectManager_ = nullptr;					// オブジェクトマネージャー
	IndirectCommandManager *indirectCommandManager_ = nullptr;	// 間接コマンドマネージャー
};