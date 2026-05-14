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
		// Windowsのメッセージ処理
		if(window_->ProcessMessage()) {
			break;
		}

		input_->Update();	// インプットの開始処理

		ImGuiManager::Begin();	// ImGuiの開始処理

		CPUTimer cpuTimer;	// CPUタイマー
		cpuTimer.Begin();	// CPUタイマーの開始

		sceneManager_->Update();	// 更新処理

		device_->NewFrame();	// 描画開始処理

		renderer_->Render();	// 描画処理

		cpuTimer.End();	// CPUタイマーの終了
		ImGui::Text("CPU Time: %.2f ms", cpuTimer.GetMs());	// CPUタイマーの結果をImGuiに表示

		ImGuiManager::Render(device_->GetCommandList());	// ImGuiの描画

		device_->EndFrame();	// 描画終了処理
	}
}