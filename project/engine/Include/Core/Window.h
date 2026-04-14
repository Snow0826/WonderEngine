#pragma once
#include <Windows.h>
#include <cstdint>

/// @brief WindowsAPI
class Window final {
public:
	/// @brief 初期化
	void Initialize();

	/// @brief メッセージ処理
	/// @return 終了メッセージが来たらtrueを返す
	bool ProcessMessage();

	/// @brief 解放
	void Finalize() const;

	/// @brief ウィンドウクラスを取得
	/// @return ウィンドウクラス
	HINSTANCE GetHInstance() const { return wndclass_.hInstance; }

	/// @brief ウィンドウハンドルを取得
	/// @return ウィンドウハンドル
	HWND GetHandle() const { return hwnd_; }

	/// @brief クライアント領域の幅を取得
	/// @return クライアント領域の幅
	uint32_t GetClientWidth() const { return kClientWidth; }

	/// @brief クライアント領域の高さを取得
	/// @return クライアント領域の高さ
	uint32_t GetClientHeight() const { return kClientHeight; }

private:
	WNDCLASS wndclass_{};	// ウィンドウクラス
	HWND hwnd_{};			// ウィンドウハンドル
	static inline const uint32_t kClientWidth = 1280;	// クライアント領域の幅
	static inline const uint32_t kClientHeight = 720;	// クライアント領域の高さ
	
	/// @brief 例外発生時にダンプファイルを出力する関数
	/// @param exception 例外情報（例外コードやコンテキストなど）
	/// @return 処理結果を示す値（EXCEPTION_EXECUTE_HANDLER など）
	static LONG WINAPI ExportDump(EXCEPTION_POINTERS *exception);

	/// @brief ウィンドウプロシージャ（メッセージ処理関数）
	/// @param hwnd 対象のウィンドウハンドル
	/// @param msg 受け取ったメッセージの種類
	/// @param wparam メッセージに付随する追加情報（ワード単位）
	/// @param lparam メッセージに付随する追加情報（ロングワード単位）
	/// @return メッセージ処理結果を示す値（DefWindowProcの戻り値など）
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};