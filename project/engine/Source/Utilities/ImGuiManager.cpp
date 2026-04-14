#include "ImGuiManager.h"
#include "DescriptorHeap.h"
#include "Logger.h"

void ImGuiManager::Initialize([[maybe_unused]] HWND hwnd, [[maybe_unused]] const Microsoft::WRL::ComPtr<ID3D12Device> &device, [[maybe_unused]] const DXGI_SWAP_CHAIN_DESC1 &swapChainDesc, const D3D12_RENDER_TARGET_VIEW_DESC &rtvDesc, [[maybe_unused]] DescriptorHeap &cbvSrvUavDescriptorHeap, [[maybe_unused]] std::ofstream &logStream) {
#ifdef USE_IMGUI
	uint32_t srvIndex = cbvSrvUavDescriptorHeap.AllocateDescriptor();
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(device.Get(),
        swapChainDesc.BufferCount,
        rtvDesc.Format,
        cbvSrvUavDescriptorHeap.GetDescriptorHeap(),
        cbvSrvUavDescriptorHeap.GetCPUDescriptorHandle(srvIndex),
        cbvSrvUavDescriptorHeap.GetGPUDescriptorHandle(srvIndex));
	Logger::Log(logStream, "ImGui SRV Descriptor Index: " + std::to_string(srvIndex) + "\n");
#endif // USE_IMGUI
}

void ImGuiManager::Begin() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif // USE_IMGUI
}

void ImGuiManager::Render([[maybe_unused]] const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> &commandList) {
#ifdef USE_IMGUI
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
#endif // USE_IMGUI
}

void ImGuiManager::Finalize() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // USE_IMGUI
}

bool ImGuiManager::Combo([[maybe_unused]] const std::string &label, [[maybe_unused]] uint32_t &current_item, [[maybe_unused]] const std::vector<std::string> &items) {
    bool changed = false;
#ifdef USE_IMGUI
    if (ImGui::BeginCombo(label.c_str(), items[current_item].c_str())) {
        for (uint32_t i = 0; i < items.size(); i++) {
            bool selected = (items[current_item] == items[i]);
            if (ImGui::Selectable(items[i].c_str(), selected)) {
				current_item = i;
                changed = true;
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
#endif // USE_IMGUI
    return changed;
}