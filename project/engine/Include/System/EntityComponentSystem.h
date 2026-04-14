#pragma once
#include "ImGuiManager.h"
#include <vector>
#include <memory>
#include <string>
#include <functional>

template<typename... Ts>
struct exclude final {};

struct Disabled final {};

/// @brief コンポーネントプールのインターフェース
class IComponentPool {
public:
	/// @brief デストラクタ
	virtual ~IComponentPool() = default;

	/// @brief コンポーネントの追加
	/// @param entity エンティティ
	virtual void AddComponent(uint32_t entity) = 0;

	/// @brief コンポーネントの削除
	/// @param entity エンティティ
	virtual void RemoveComponent(uint32_t entity) = 0;

	/// @brief コンポーネントの有無を確認
	/// @param entity エンティティ
	/// @return コンポーネントが存在する場合はtrueを返す
	virtual bool HasComponent(uint32_t entity) const = 0;

	/// @brief エンティティの配列を取得
	/// @return エンティティの配列への参照
	virtual const std::vector<uint32_t> &GetEntities() const = 0;

	/// @brief コンポーネント数の取得
	/// @return コンポーネント数
	virtual size_t GetSize() const = 0;
};

/// @brief コンポーネントプール
/// @tparam T コンポーネントの型
template<typename T>
class ComponentPool final : public IComponentPool {
public:
	ComponentPool() {
		dense_.reserve(kMaxEntity);
		entities_.reserve(kMaxEntity);
		sparse_.reserve(kMaxEntity);
	}

	/// @brief コンポーネントの追加
	/// @param entity エンティティ
	/// @param component コンポーネント
	void AddComponent(uint32_t entity, const T &component) {
		if (entity >= sparse_.size()) {
			sparse_.resize(entity + 1, kInvalid);
		}

		if (HasComponent(entity)) {
			dense_[sparse_[entity]] = component;
			return;
		}

		uint32_t index = static_cast<uint32_t>(dense_.size());
		dense_.emplace_back(component);
		entities_.emplace_back(entity);
		sparse_[entity] = index;
	}

	/// @brief コンポーネントの追加（デフォルトコンストラクタ版）
	/// @param entity エンティティ
	void AddComponent(uint32_t entity) override {
		AddComponent(entity, T{});
	}

	/// @brief コンポーネントの削除
	/// @param entity エンティティ
	void RemoveComponent(uint32_t entity) override {
		if (!HasComponent(entity)) return;
		uint32_t index = sparse_[entity];
		uint32_t last = static_cast<uint32_t>(dense_.size() - 1);
		dense_[index] = dense_[last];
		entities_[index] = entities_[last];
		sparse_[entities_[index]] = index;
		dense_.pop_back();
		entities_.pop_back();
		sparse_[entity] = kInvalid;
	}

	/// @brief コンポーネントの有無を確認
	/// @param entity エンティティ
	/// @return コンポーネントが存在する場合はtrueを返す
	bool HasComponent(uint32_t entity) const override {
		return entity < sparse_.size() && sparse_[entity] != kInvalid;
	}

	/// @brief コンポーネントの取得
	/// @param entity エンティティ
	/// @return コンポーネントへのポインタ、存在しない場合はnullptrを返す
	T *GetComponent(uint32_t entity) {
		return HasComponent(entity) ? &dense_[sparse_[entity]] : nullptr;
	}

	/// @brief エンティティの配列を取得
	/// @return エンティティの配列への参照
	const std::vector<uint32_t> &GetEntities() const override { return entities_; }

	/// @brief コンポーネント数の取得
	/// @return コンポーネント数
	size_t GetSize() const override { return dense_.size(); }

private:
	static inline constexpr uint32_t kInvalid = UINT32_MAX;	// 無効なインデックス値
	static inline constexpr uint32_t kMaxEntity = 1048576;	// 最大エンティティ数
	std::vector<T> dense_;				// 実データの配列
	std::vector<uint32_t> entities_;	// エンティティの配列
	std::vector<uint32_t> sparse_;		// スパース配列
};

/// @brief レジストリ
class Registry final {
public:
	/// @brief エンティティの生成
	/// @return 生成されたエンティティID
	uint32_t GenerateEntity() { return entity_++; }

	/// @brief コンポーネントの登録
	/// @tparam T コンポーネントの型
	template<typename T>
	void RegisterComponent() {
		size_t typeId = GetTypeId<T>();
		if (typeId >= pools_.size()) {
			pools_.resize(typeId + 1);
		}
		if (!pools_[typeId]) {
			pools_[typeId] = std::make_unique<ComponentPool<T>>();
		}
	}

	/// @brief コンポーネントの追加
	/// @tparam T コンポーネントの型
	/// @param entity エンティティ
	/// @param component コンポーネント
	template<typename T>
	void AddComponent(uint32_t entity, const T &component) {
		size_t typeId = GetTypeId<T>();
		if (typeId >= pools_.size()) {
			pools_.resize(typeId + 1);
		}
		if (!pools_[typeId]) {
			pools_[typeId] = std::make_unique<ComponentPool<T>>();
		}
		static_cast<ComponentPool<T> *>(pools_[typeId].get())->AddComponent(entity, component);
	}

	/// @brief コンポーネントの追加（デフォルトコンストラクタ版）
	/// @tparam T コンポーネントの型
	/// @param entity エンティティ
	template<typename T>
	void AddComponent(uint32_t entity) {
		size_t typeId = GetTypeId<T>();
		if (typeId >= pools_.size()) {
			pools_.resize(typeId + 1);
		}
		if (!pools_[typeId]) {
			pools_[typeId] = std::make_unique<ComponentPool<T>>();
		}
		pools_[typeId]->AddComponent(entity);
	}

	/// @brief コンポーネントの削除
	/// @tparam T コンポーネントの型
	/// @param entity エンティティ
	template<typename T>
	void RemoveComponent(uint32_t entity) {
		size_t typeId = GetTypeId<T>();
		if (typeId >= pools_.size() || !pools_[typeId]) return;
		pools_[typeId]->RemoveComponent(entity);
	}

	/// @brief コンポーネントの取得
	/// @tparam T コンポーネントの型
	/// @param entity エンティティ
	/// @return コンポーネントへのポインタ、存在しない場合はnullptrを返す
	template<typename T>
	T *GetComponent(uint32_t entity) {
		size_t typeId = GetTypeId<T>();
		if (typeId >= pools_.size() || !pools_[typeId]) return nullptr;
		return static_cast<ComponentPool<T>*>(pools_[typeId].get())->GetComponent(entity);
	}

	/// @brief コンポーネントの有無を確認
	/// @tparam T コンポーネントの型
	/// @param entity エンティティ
	/// @return コンポーネントが存在する場合はtrueを返す
	template<typename T>
	bool HasComponent(uint32_t entity) const {
		size_t typeId = GetTypeId<T>();
		if (typeId >= pools_.size() || !pools_[typeId]) return false;
		return pools_[typeId]->HasComponent(entity);
	}

	/// @brief 全てのコンポーネントを削除
	/// @param entity エンティティ
	void RemoveAllComponents(uint32_t entity) {
		for (auto &pool : pools_) {
			if (pool) {
				pool->RemoveComponent(entity);
			}
		}
	}

	/// @brief 指定した型IDのコンポーネントを追加
	/// @param typeId 型ID
	/// @param entity エンティティ
	void AddComponentById(size_t typeId, uint32_t entity) {
		if (typeId >= pools_.size() || !pools_[typeId]) return;
		pools_[typeId]->AddComponent(entity);
	}

	/// @brief 指定した型IDのコンポーネントを削除
	/// @param typeId 型ID
	/// @param entity エンティティ
	void RemoveComponentById(size_t typeId, uint32_t entity) {
		if (typeId >= pools_.size() || !pools_[typeId]) return;
		pools_[typeId]->RemoveComponent(entity);
	}

	/// @brief 指定した型IDのコンポーネントの有無を確認
	/// @param typeId 型ID
	/// @param entity エンティティ
	/// @return コンポーネントが存在する場合はtrueを返す
	bool HasComponentById(size_t typeId, uint32_t entity) const {
		if (typeId >= pools_.size() || !pools_[typeId]) return false;
		return pools_[typeId]->HasComponent(entity);
	}

	/// @brief 指定したコンポーネントの数を取得
	/// @tparam T コンポーネントの型
	/// @return コンポーネントの数
	template<typename T>
	size_t GetComponentCount() const {
		size_t typeId = GetTypeId<T>();
		if (typeId >= pools_.size() || !pools_[typeId]) return 0;
		return pools_[typeId]->GetSize();
	}

	/// @brief エンティティの数を取得
	/// @return エンティティの数
	uint32_t GetEntityCount() const {
		return entity_;
	}

	/// @brief 指定したコンポーネントを持つ全てのエンティティに対して関数を実行する
	/// @tparam Require 必須コンポーネントの型
	/// @tparam Exclude 除外コンポーネントの型
	/// @tparam Func 関数の型
	/// @param func 実行する関数
	template<typename... Require, typename... Exclude, typename Func>
	void ForEach(Func &&func, exclude<Exclude...> = {}) {
		IComponentPool *smallest = nullptr;
		size_t minSize = SIZE_MAX;
		bool valid = true;

		// 最小のコンポーネントプールを見つける
		(([&] {
			size_t typeId = GetTypeId<Require>();
			if (typeId >= pools_.size() || !pools_[typeId]) {
				valid = false;
				return;
			}

			size_t size = pools_[typeId]->GetSize();
			if (size < minSize) {
				minSize = size;
				smallest = pools_[typeId].get();
			}
			}()), ...);

		if (!smallest || !valid) return;

		// 最小のコンポーネントプールのエンティティに対して関数を実行する
		for (uint32_t entity : smallest->GetEntities()) {
			// 必須コンポーネントを全て持っているか確認
			if (!(HasComponent<Require>(entity) && ...)) continue;

			// 除外コンポーネントを持っていないか確認
			if ((HasComponent<Exclude>(entity) || ...)) continue;

			// 関数を実行
			func(entity, GetComponent<Require>(entity)...);
		}
	}

	/// @brief 型IDの取得
	/// @tparam T コンポーネントの型
	/// @return 型ID
	template<typename T>
	static size_t GetTypeId() {
		static size_t id = GenerateTypeId();
		return id;
	}

private:
	uint32_t entity_ = 0;									// エンティティ
	std::vector<std::unique_ptr<IComponentPool>> pools_;	// コンポーネントプールの配列

	/// @brief 型IDの生成
	/// @return 生成された型ID
	static size_t GenerateTypeId() {
		static size_t counter = 0;
		return counter++;
	}
};

/// @brief インスペクターレジストリ
class InspectorRegistry final {
public:
	using DrawFunc = std::function<void(uint32_t)>;	// インスペクター描画関数の型

	/// @brief コンストラクタ
	/// @param registry レジストリ
	InspectorRegistry(Registry *registry) : registry_(registry) {}

	/// @brief インスペクターの登録
	/// @tparam T コンポーネントの型
	/// @param func インスペクター描画関数
	template<typename T>
	void RegisterInspector(DrawFunc func) {
		size_t typeId = Registry::GetTypeId<T>();
		if (typeId >= inspectors_.size()) {
			inspectors_.resize(typeId + 1);
		}
		inspectors_[typeId] = func;
	}

	/// @brief タグインスペクターの登録
	/// @tparam T コンポーネントの型
	/// @param tagName タグ名
	template<typename T>
	void RegisterTagInspector(const std::string &tagName) {
		size_t typeId = Registry::GetTypeId<T>();
		if (typeId >= tagInspectors_.size()) {
			tagInspectors_.resize(typeId + 1);
		}
		registry_->RegisterComponent<T>();
		tagInspectors_[typeId] = tagName;
	}

	/// @brief 全てのエンティティのインスペクターの描画
	void DrawEntities() {
#ifdef USE_IMGUI
		if (ImGui::TreeNode(("Entities " + std::to_string(registry_->GetEntityCount())).c_str())) {
			for (uint32_t e = 0; e < registry_->GetEntityCount(); ++e) {
				ImGui::PushID(e);
				if (ImGui::TreeNode(("Entity " + std::to_string(e)).c_str())) {
					DrawInspectors(e);
					DrawTagInspectors(e);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
#endif // USE_IMGUI
	}

private:
	Registry *registry_ = nullptr;				// レジストリ
	std::vector<DrawFunc> inspectors_;			// インスペクターの配列
	std::vector<std::string> tagInspectors_;	// タグインスペクターの配列

	/// @brief インスペクターの描画
	/// @param entity エンティティ
	void DrawInspectors(uint32_t entity) {
		for (size_t i = 0; i < inspectors_.size(); ++i) {
			if (!inspectors_[i]) {
				continue;
			}

			if (!registry_->HasComponentById(i, entity)) {
				continue;
			}

			inspectors_[i](entity);
		}
	}

	/// @brief タグインスペクターの描画
	/// @param entity エンティティ
	void DrawTagInspectors(uint32_t entity) {
#ifdef USE_IMGUI
		for (size_t i = 0; i < tagInspectors_.size(); i++) {
			if (tagInspectors_[i].empty()) {
				continue;
			}

			bool visible = registry_->HasComponentById(i, entity);
			if (ImGui::Checkbox(tagInspectors_[i].c_str(), &visible)) {
				if (visible) {
					registry_->AddComponentById(i, entity);
				} else {
					registry_->RemoveComponentById(i, entity);
				}
			}
		}
#endif // USE_IMGUI
	}
};