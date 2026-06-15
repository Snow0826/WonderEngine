#pragma once
#include <cstdint>

class Registry;
class ComponentDrawerRegistry;
struct SelectionContext;

/// @brief インスペクタウィンドウ
class InspectorWindow final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param componentDrawerRegistry コンポーネント描画関数レジストリ
	/// @param selection 選択コンテキスト
	InspectorWindow(Registry *registry, ComponentDrawerRegistry *componentDrawerRegistry, SelectionContext *selection)
		: registry_(registry), componentDrawerRegistry_(componentDrawerRegistry), selection_(selection) {}

	/// @brief 描画
	void Draw();

	/// @brief ウィンドウの開閉状態への参照を取得
	/// @return ウィンドウの開閉状態への参照
	bool &IsOpen() { return isOpen_; }

private:
	Registry *registry_ = nullptr;	// レジストリ
	ComponentDrawerRegistry *componentDrawerRegistry_ = nullptr;	// コンポーネント描画関数レジストリ
	SelectionContext *selection_ = nullptr;	// 選択コンテキスト
	bool isOpen_ = true;	// ウィンドウの開閉状態

	/// @brief コンポーネントの描画
	/// @param entity エンティティ
	void DrawComponents(uint32_t entity);

	/// @brief タグコンポーネントの描画
	/// @param entity エンティティ
	void DrawTagComponents(uint32_t entity);
};