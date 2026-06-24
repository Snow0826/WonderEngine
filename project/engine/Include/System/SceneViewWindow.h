#pragma once
#include <memory>

class Registry;
class Device;
class World;
class TransformManipulator;
struct SelectionContext;

/// @brief シーンビューウィンドウ
class SceneViewWindow {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param device デバイス
	/// @param world ワールド
	/// @param selection 選択コンテキスト
	SceneViewWindow(Registry *registry, Device *device, World *world, SelectionContext *selection);

	/// @brief デストラクタ
	~SceneViewWindow();

	/// @brief 描画
	void Draw();

	/// @brief ウィンドウの開閉状態への参照を取得
	/// @return ウィンドウの開閉状態への参照
	bool &IsOpen() { return isOpen_; }

private:
	Registry *registry_ = nullptr;	// レジストリ
	Device *device_ = nullptr;	// デバイス
	World *world_ = nullptr;	// ワールド
	SelectionContext *selection_ = nullptr;	// 選択コンテキスト
	std::unique_ptr<TransformManipulator> transformManipulator_ = nullptr;	// 変換マニピュレーター
	bool isOpen_ = true;	// ウィンドウの開閉状態
};