#pragma once

class Registry;
class Device;
class World;

/// @brief ゲームビューウィンドウ
class GameViewWindow {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param device デバイス
	/// @param world ワールド
	GameViewWindow(Registry *registry, Device *device, World *world) : registry_(registry), device_(device), world_(world) {}

	/// @brief 描画
	void Draw();

	/// @brief ウィンドウの開閉状態への参照を取得
	/// @return ウィンドウの開閉状態への参照
	bool &IsOpen() { return isOpen_; }

private:
	Registry *registry_ = nullptr;	// レジストリ
	Device *device_ = nullptr;		// デバイス
	World *world_ = nullptr;		// ワールド
	bool isOpen_ = true;			// ウィンドウの開閉状態
};