#include "ImGuiManager.h"
#include "DescriptorHeap.h"
#include "Logger.h"

#ifdef USE_IMGUI
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <ImGuizmo.h>
#endif // USE_IMGUI

void ImGuiManager::Initialize([[maybe_unused]] HWND hwnd, [[maybe_unused]] const Microsoft::WRL::ComPtr<ID3D12Device> &device, [[maybe_unused]] const Microsoft::WRL::ComPtr<ID3D12CommandQueue> &commandQueue, [[maybe_unused]] const DXGI_SWAP_CHAIN_DESC1 &swapChainDesc, const D3D12_RENDER_TARGET_VIEW_DESC &rtvDesc, const D3D12_DEPTH_STENCIL_VIEW_DESC &dsvDesc, [[maybe_unused]] DescriptorHeap &cbvSrvUavDescriptorHeap, [[maybe_unused]] std::ofstream &logStream) {
#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	static ImGuiDescriptorContext context;
	context.cbvSrvUavDescriptorHeap = &cbvSrvUavDescriptorHeap;
	context.logStream = &logStream;
	ImGui_ImplDX12_InitInfo initInfo = {};
	initInfo.Device = device.Get();
	initInfo.CommandQueue = commandQueue.Get();
	initInfo.NumFramesInFlight = swapChainDesc.BufferCount;
	initInfo.RTVFormat = rtvDesc.Format;
	initInfo.DSVFormat = dsvDesc.Format;
	initInfo.SrvDescriptorHeap = cbvSrvUavDescriptorHeap.GetDescriptorHeap();
	initInfo.UserData = &context;
	initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo *info, D3D12_CPU_DESCRIPTOR_HANDLE *out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE *out_gpu_handle) {
		auto *context = static_cast<ImGuiDescriptorContext *>(info->UserData);
		uint32_t srvIndex = context->cbvSrvUavDescriptorHeap->AllocateDescriptor();
		*out_cpu_handle = context->cbvSrvUavDescriptorHeap->GetCPUDescriptorHandle(srvIndex);
		*out_gpu_handle = context->cbvSrvUavDescriptorHeap->GetGPUDescriptorHandle(srvIndex);
		Logger::Log(*context->logStream, "ImGui SRVDescriptorIndex: " + std::to_string(srvIndex) + "\n");
		};
	initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {};
	ImGui_ImplDX12_Init(&initInfo);
#endif // USE_IMGUI
}

void ImGuiManager::Begin() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGuizmo::BeginFrame();
#endif // USE_IMGUI
}

void ImGuiManager::End() {
#ifdef USE_IMGUI
	ImGui::Render();
#endif // USE_IMGUI
}

void ImGuiManager::Draw([[maybe_unused]] const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> &commandList) {
#ifdef USE_IMGUI
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