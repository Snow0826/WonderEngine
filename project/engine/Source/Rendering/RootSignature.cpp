#include "RootSignature.h"
#include "Logger.h"
#include <cassert>

Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature::Create(std::ofstream &logStream, const Microsoft::WRL::ComPtr<ID3D12Device> &device) {
	// RootSignatureDescの設定
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descriptionRootSignature.pParameters = rootParameters_.data();							// RootParameterの配列
	descriptionRootSignature.NumParameters = static_cast<UINT>(rootParameters_.size());		// RootParameterの数
	descriptionRootSignature.pStaticSamplers = staticSamplers_.data();						// StaticSamplerの配列
	descriptionRootSignature.NumStaticSamplers = static_cast<UINT>(staticSamplers_.size());	// StaticSamplerの数

	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(logStream, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// RootSignatureを作成する
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	return rootSignature;
}

RootSignature RootSignature::Add32BitConstant(D3D12_SHADER_VISIBILITY visibility, UINT shaderRegister, int num32BitValues) {
	D3D12_ROOT_PARAMETER rootParameter{};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;	// パラメータの種類
	rootParameter.ShaderVisibility = visibility;								// シェーダーの可視性
	rootParameter.Constants.ShaderRegister = shaderRegister;					// シェーダーのレジスタ番号
	rootParameter.Constants.Num32BitValues = num32BitValues;					// 32ビット定数の数
	rootParameters_.push_back(rootParameter);									// メンバ配列に追加
	return *this;
}

RootSignature RootSignature::AddCBuffer(D3D12_SHADER_VISIBILITY visibility, UINT shaderRegister) {
	D3D12_ROOT_PARAMETER rootParameter{};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	// パラメータの種類
	rootParameter.ShaderVisibility = visibility;					// シェーダーの可視性
	rootParameter.Descriptor.ShaderRegister = shaderRegister;		// シェーダーのレジスタ番号
	rootParameters_.push_back(rootParameter);						// メンバ配列に追加
	return *this;
}

RootSignature RootSignature::AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility, UINT shaderRegister) {
	// DescriptorRangeの設定
	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.BaseShaderRegister = shaderRegister;										// シェーダーのレジスタ番号
	descriptorRange.NumDescriptors = numDescriptors;											// ディスクリプタの数
	descriptorRange.RangeType = rangeType;														// ディスクリプタの種類
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;	// ディスクリプタテーブルのオフセット
	descriptorRanges_.push_back(descriptorRange);												// DescriptorRangeの配列を追加

	// DescriptorTableの設定
	D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable{};
	descriptorTable.NumDescriptorRanges = 1;								// ディスクリプタテーブルの数
	descriptorTable.pDescriptorRanges = &descriptorRanges_[rangeCounter_];	// ディスクリプタテーブルの配列
	rangeCounter_++;														// ディスクリプタのカウンターをインクリメント

	// RootParameterの設定
	D3D12_ROOT_PARAMETER rootParameter{};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	// パラメータの種類
	rootParameter.ShaderVisibility = visibility;								// シェーダーの可視性
	rootParameter.DescriptorTable = descriptorTable;							// デスクリプタテーブルの設定
	rootParameters_.push_back(rootParameter);									// RootParameterの配列を追加

	return *this;
}

RootSignature RootSignature::AddSampler(D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE mode, D3D12_COMPARISON_FUNC comparison, FLOAT maxLOD, UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility) {
	D3D12_STATIC_SAMPLER_DESC staticSampler{};
	staticSampler.Filter = filter;					// フィルタリング
	staticSampler.AddressU = mode;					// U座標のアドレスモード
	staticSampler.AddressV = mode;					// V座標のアドレスモード
	staticSampler.AddressW = mode;					// W座標のアドレスモード
	staticSampler.ComparisonFunc = comparison;		// 比較関数
	staticSampler.MaxLOD = maxLOD;					// 最大LOD
	staticSampler.ShaderRegister = shaderRegister;	// シェーダーのレジスタ番号
	staticSampler.ShaderVisibility = visibility;	// シェーダーの可視性
	staticSamplers_.push_back(staticSampler);		// StaticSamplerの配列を追加
	return *this;
}