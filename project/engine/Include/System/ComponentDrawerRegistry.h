#pragma once
#include "EntityComponentSystem.h"
#include <functional>
#include <string>

/// @brief コンポーネント描画関数レジストリ
class ComponentDrawerRegistry final {
public:
	using DrawFunc = std::function<void(uint32_t)>;	// コンポーネント描画関数の型

	/// @brief コンストラクタ
	/// @param registry レジストリ
	ComponentDrawerRegistry(Registry *registry) : registry_(registry) {}

	/// @brief コンポーネント描画関数の登録
	/// @tparam T コンポーネントの型
	/// @param func コンポーネント描画関数
	template<typename T>
	void RegisterComponentDrawer(DrawFunc func) {
		size_t typeId = Registry::GetTypeId<T>();
		if (typeId >= componentDrawers_.size()) {
			componentDrawers_.resize(typeId + 1);
		}
		componentDrawers_[typeId] = func;
	}

	/// @brief タグコンポーネントの登録
	/// @tparam T コンポーネントの型
	/// @param tagName タグ名
	template<typename T>
	void RegisterTagComponent(const std::string &tagName) {
		size_t typeId = Registry::GetTypeId<T>();
		if (typeId >= tagComponents_.size()) {
			tagComponents_.resize(typeId + 1);
		}
		registry_->RegisterComponent<T>();
		tagComponents_[typeId] = tagName;
	}

	/// @brief コンポーネント描画関数の配列への参照を取得
	/// @return コンポーネント描画関数の配列への参照
	std::vector<DrawFunc> &GetComponentDrawers() { return componentDrawers_; }

	/// @brief タグコンポーネント用の配列への参照を取得
	/// @return タグコンポーネント用の配列への参照
	std::vector<std::string> &GetTagComponents() { return tagComponents_; }

private:
	Registry *registry_ = nullptr;				// レジストリ
	std::vector<DrawFunc> componentDrawers_;	// コンポーネント描画関数の配列
	std::vector<std::string> tagComponents_;	// タグコンポーネントの配列
};