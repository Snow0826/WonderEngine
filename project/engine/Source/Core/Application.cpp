#include "Application.h"
#include "Audio.h"
#include "Device.h"
#include "Window.h"
#include "Input.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "ImGuiManager.h"
#include "CPUTimer.h"
#include "Logger.h"
#include <cassert>

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "Dbghelp.lib")

Application::Application() {
	// COMライブラリの初期化
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	// ログファイルを開く
	logStream_ = Logger::OpenLogFile();

	// ウィンドウの初期化
	window_ = std::make_unique<Window>();
	window_->Initialize();
	Logger::Log(logStream_, "Create Window\n");

	// デバイスの初期化
	device_ = std::make_unique<Device>();
	device_->Initialize(logStream_, *window_);
	Logger::Log(logStream_, "Create Device\n");

	// オーディオの初期化
	audio_ = std::make_unique<Audio>(&logStream_);
	audio_->Initialize();
	Logger::Log(logStream_, "Create Audio\n");

	// インプットの初期化
	input_ = std::make_unique<Input>();
	input_->Initialize(*window_);
	Logger::Log(logStream_, "Create Input\n");

	// レンダラーの初期化
	renderer_ = std::make_unique<Renderer>(device_.get());
	renderer_->Initialize(logStream_);
	Logger::Log(logStream_, "Create Renderer\n");

	// シーンマネージャーの初期化
	sceneManager_ = std::make_unique<SceneManager>();
	sceneManager_->Initialize(device_.get(), input_.get(), audio_.get(), renderer_.get(), &logStream_);
	Logger::Log(logStream_, "Create SceneManager\n");

	// デルタタイマーの初期化
	deltaTimer_ = std::make_unique<CPUTimer>();

	// CPUタイマーの初期化
	cpuTimer_ = std::make_unique<CPUTimer>();
}

Application::~Application() {
	device_->Finalize();	// デバイスの終了処理
	window_->Finalize();	// ウィンドウの終了処理

	// ログファイルを閉じる
	if (logStream_.is_open()) {
		logStream_.close();
	}

	CoUninitialize();	// COMライブラリの終了
}

void Application::Run() {
	// ウィンドウの×ボタンが押されるまでループ
	while (true) {
		float deltaTime = static_cast<float>(deltaTimer_->GetMs() / 1000.0);	// デルタタイムの取得
		deltaTimer_->Begin();		// デルタタイマーの開始

		// Windowsのメッセージ処理
		if (window_->ProcessMessage()) {
			break;
		}

		input_->Update();	// インプットの開始処理

		ImGuiManager::Begin();	// ImGuiの開始処理

#ifdef USE_IMGUI
		ImGui::Text("Framerate: %.2f fps", ImGui::GetIO().Framerate);	// フレームレートをImGuiに表示
		ImGui::Text("CPU Time: %.2f ms", cpuTimer_->GetMs());	// CPUタイムをImGuiに表示
#endif // USE_IMGUI

		cpuTimer_->Begin();	// CPUタイマーの開始

		sceneManager_->Update(deltaTime);	// 更新処理

		ImGuiManager::End();	// ImGuiの終了処理

		device_->NewFrame();	// 描画開始処理

		renderer_->Render();	// 描画処理

		cpuTimer_->End();	// CPUタイマーの終了

		ImGuiManager::Draw(device_->GetCommandList());	// ImGuiの描画

		device_->EndFrame();	// 描画終了処理

		deltaTimer_->End();	// デルタタイマーの終了
	}
}