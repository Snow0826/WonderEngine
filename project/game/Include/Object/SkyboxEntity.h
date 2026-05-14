#pragma once

class Registry;
class SkyboxGenerator;
class ObjectManager;

/// @brief スカイボックスエンティティ
class SkyboxEntity {
public:
	/// @brief 天球の作成
	/// @param registry レジストリ
	/// @param skyboxGenerator スカイボックスジェネレーター
	/// @param objectManager オブジェクトマネージャー
	static void Create(Registry *registry, SkyboxGenerator *skyboxGenerator, ObjectManager *objectManager);
};