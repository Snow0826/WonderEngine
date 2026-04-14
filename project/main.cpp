#include "Application.h"
#include <d3d12.h>

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	Application app;	// アプリケーションのインスタンスを作成
	app.Run();			// アプリケーションの実行
	return 0;
}