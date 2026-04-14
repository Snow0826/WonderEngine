#include "Window.h"
#include "ImGuiManager.h"
#ifdef USE_IMGUI
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
#endif // USE_IMGUI
#include <strsafe.h>
#include <DbgHelp.h>

#pragma comment(lib, "winmm.lib")

void Window::Initialize() {
	// タイマーの分解能を1msに設定
	timeBeginPeriod(1);

	// 例外発生時にダンプファイルを出力する関数を登録
	SetUnhandledExceptionFilter(ExportDump);

	// ウィンドウクラスの定義
	wndclass_.lpfnWndProc = Window::WindowProc;			// ウィンドウプロシージャ
	wndclass_.lpszClassName = L"CG2WindowClass";		// ウィンドウクラス名
	wndclass_.hInstance = GetModuleHandle(nullptr);		// インスタンスハンドル
	wndclass_.hCursor = LoadCursor(nullptr, IDC_ARROW);	// カーソル
	RegisterClass(&wndclass_);							// ウィンドウクラスを登録

	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0, 0, kClientWidth, kClientHeight };

	// クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウの作成
	hwnd_ = CreateWindow(
		wndclass_.lpszClassName,		// ウィンドウクラス名
		L"LE2A_05_クラタ_ユウキ_染め戦",	// タイトルバーの文字列
		WS_OVERLAPPEDWINDOW,			// ウィンドウスタイル
		CW_USEDEFAULT,					// ウィンドウのX座標
		CW_USEDEFAULT,					// ウィンドウのY座標
		wrc.right - wrc.left,			// ウィンドウの幅
		wrc.bottom - wrc.top,			// ウィンドウの高さ
		nullptr,						// 親ウィンドウハンドル
		nullptr,						// メニューハンドル
		wndclass_.hInstance,			// インスタンスハンドル
		nullptr);						// その他のパラメータ

	// ウィンドウを表示
	ShowWindow(hwnd_, SW_SHOW);

}

bool Window::ProcessMessage() {
	MSG msg{};

	// メッセージが来ていたら取得して処理する
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 終了メッセージが来ていたらtrueを返す
	if (msg.message == WM_QUIT) {
		return true;
	}
	return false;
}

void Window::Finalize() const {
	CloseWindow(hwnd_);	// ウィンドウを閉じる
}

LONG WINAPI Window::ExportDump(EXCEPTION_POINTERS *exception) {
	// 時刻を指定して、時刻を名前に入れたファイルを作成する。Dumpsディレクトリ以下に出力
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	// processId(このexeのId)とクラッシュ(例外)の発生したthreadIdを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();

	// 設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
	minidumpInformation.ThreadId = threadId;	// スレッドID
	minidumpInformation.ExceptionPointers = exception;	// 例外情報
	minidumpInformation.ClientPointers = TRUE;	// クライアントポインタを使用する

	// Dumpを出力。MiniDumpNormalは最低限の情報を出力するフラグ
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);

	// 他に関連付けられているSEH例外ハンドラがあれば実行。通常はプロセスを終了する
	return EXCEPTION_EXECUTE_HANDLER;
}

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
#ifdef USE_IMGUI
	// ImGuiのウィンドウプロシージャを最初に実行する
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif // USE_IMGUI

	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		// ウィンドウが破棄されたとき
		case WM_DESTROY:
			// OSに対して、アプリの終了を伝える
			PostQuitMessage(0);
			break;
	}

	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}