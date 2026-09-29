#pragma once

class Registry;
class SkyboxGenerator;

/// @brief スカイボックスエンティティ
class SkyboxEntity {
public:
	/// @brief スカイボックスエンティティの作成
	/// @param registry レジストリ
	/// @param skyboxGenerator スカイボックスジェネレーター
	static void Create(Registry *registry, SkyboxGenerator *skyboxGenerator);
};