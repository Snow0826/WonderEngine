#pragma once
#include <wrl/client.h>
#include <d3d12.h>
#include <vector>
#include <fstream>

/// @brief ルートシグネチャ
class RootSignature final {
public:
	/// @brief ルートシグネチャの作成
	/// @param logStream ログストリーム
	/// @param device デバイス
	/// @return ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> Create(std::ofstream &logStream, const Microsoft::WRL::ComPtr<ID3D12Device> &device);

	/// @brief 32BitConstantの追加
	/// @param visibility シェーダーの可視性
	/// @param shaderRegister シェーダーのレジスタ番号
	/// @param num32BitValues 32ビット値の数
	/// @return ルートシグネチャ
	RootSignature Add32BitConstant(D3D12_SHADER_VISIBILITY visibility, UINT shaderRegister, int num32BitValues);

	/// @brief ConstantBufferの追加
	/// @param visibility シェーダーの可視性
	/// @param shaderRegister シェーダーのレジスタ番号
	/// @return ルートシグネチャ
	RootSignature AddCBuffer(D3D12_SHADER_VISIBILITY visibility, UINT shaderRegister);

	/// @brief DescriptorTableの追加
	/// @param rangeType デスクリプタの種類
	/// @param numDescriptors ディスクリプタの数
	/// @param visibility シェーダーの可視性
	/// @param shaderRegister シェーダーのレジスタ番号
	/// @return ルートシグネチャ
	RootSignature AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility, UINT shaderRegister);

	/// @brief StaticSamplerの追加
	/// @param filter フィルタリングモード
	/// @param mode アドレスモード
	/// @param comparison 比較関数
	/// @param maxLOD 最大LOD
	/// @param shaderRegister シェーダーのレジスタ番号
	/// @param visibility シェーダーの可視性
	/// @return ルートシグネチャ
	RootSignature AddSampler(D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE mode, D3D12_COMPARISON_FUNC comparison, FLOAT maxLOD, UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility);

private:
	std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges_{};	// DescriptorRangeの配列
	std::vector<D3D12_ROOT_PARAMETER> rootParameters_{};		// RootParameterの配列
	std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers_{};	// StaticSamplerの配列
	uint32_t rangeCounter_ = 0;	// DescriptorRangeのカウンター
};