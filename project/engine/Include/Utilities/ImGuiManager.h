#pragma once
#ifdef USE_IMGUI
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#endif // USE_IMGUI
#include <wrl/client.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <string>
#include <vector>

class DescriptorHeap;

/// @brief ImGuiマネージャー
struct ImGuiManager final {
	/// @brief ImGuiの初期化
	/// @param hwnd ウィンドウハンドル
	/// @param device デバイス
	/// @param swapChainDesc スワップチェーンのオプション
	/// @param rtvDesc RTVのオプション
	/// @param cbvSrvUavDescriptorHeap CBV/SRV/UAV用のディスクリプタヒープ
	static void Initialize(HWND hwnd, const Microsoft::WRL::ComPtr<ID3D12Device> &device, const DXGI_SWAP_CHAIN_DESC1 &swapChainDesc, const D3D12_RENDER_TARGET_VIEW_DESC &rtvDesc, DescriptorHeap &cbvSrvUavDescriptorHeap, std::ofstream &logStream);

	/// @brief ImGuiの開始
	static void Begin();
	
	/// @brief ImGuiの終了
	/// @param commandList コマンドリスト
	static void Render(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> &commandList);

	/// @brief ImGuiの終了
	static void Finalize();

	/// @brief ImGuiのComboBoxを作成
	/// @param label ラベル
	/// @param current_item 現在のアイテム
	/// @param items アイテム名リスト
	/// @return 選択されたらtrueを返す
	static bool Combo(const std::string &label, uint32_t &current_item, const std::vector<std::string> &items);
};