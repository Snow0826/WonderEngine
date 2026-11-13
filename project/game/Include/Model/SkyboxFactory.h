#pragma once

class Registry;
class SkyboxGenerator;

/// @brief スカイボックスのエンティティ作成クラス
class SkyboxFactory {
public:
	/// @brief スカイボックスエンティティの作成
	/// @param registry レジストリ
	/// @param skyboxGenerator スカイボックスジェネレーター
	static void Create(Registry &registry, SkyboxGenerator &skyboxGenerator);
};