#pragma once
#include "D3DResourceLeakChecker.h"
#include <fstream>
#include <memory>

class Audio;
class Input;
class Device;
class Window;
class Renderer;
class SceneManager;

/// @brief アプリケーション
class Application final {
public:
	/// @brief コンストラクタ
	Application();

	/// @brief デストラクタ
	~Application();

	/// @brief アプリケーション実行
	void Run();

private:
	D3DResourceLeakChecker leakCheck_;						// DirectXリソースリークチェック
	std::ofstream logStream_;								// ログ出力ストリーム
	std::unique_ptr<Audio> audio_ = nullptr;				// オーディオ
	std::unique_ptr<Input> input_ = nullptr;				// インプット
	std::unique_ptr<Window> window_ = nullptr;				// ウィンドウ
	std::unique_ptr<Device> device_ = nullptr;				// デバイス
	std::unique_ptr<Renderer> renderer_ = nullptr;			// レンダラー
	std::unique_ptr<SceneManager> sceneManager_ = nullptr;	// シーンマネージャー
};