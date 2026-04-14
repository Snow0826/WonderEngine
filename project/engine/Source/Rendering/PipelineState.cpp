#include "PipelineState.h"
#include "Logger.h"
#include "StringConverter.h"
#include <cassert>
#include <format>

Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState::Create(ID3D12Device *device, ID3D12RootSignature *rootSignature) {
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
	if (computeShader_.pShaderBytecode == nullptr) {
		// グラフィックスパイプラインステートの設定
		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
		graphicsPipelineStateDesc.pRootSignature = rootSignature;										// RootSignature
		graphicsPipelineStateDesc.InputLayout.pInputElementDescs = inputElements_.data();				// InputLayout
		graphicsPipelineStateDesc.InputLayout.NumElements = static_cast<UINT>(inputElements_.size());	// InputLayoutの数
		graphicsPipelineStateDesc.BlendState = blend_;													// BlendState
		graphicsPipelineStateDesc.RasterizerState = rasterizer_;										// RasterizerState
		graphicsPipelineStateDesc.VS = vertexShader_;													// VertexShader
		graphicsPipelineStateDesc.PS = pixelShader_;													// PixelShader
		graphicsPipelineStateDesc.NumRenderTargets = static_cast<UINT>(renderTargetFormats_.size());	// 書き込むRTVの数
		for (size_t i = 0; i < renderTargetFormats_.size(); i++) {
			graphicsPipelineStateDesc.RTVFormats[i] = renderTargetFormats_[i];							// 書き込むRTVのフォーマット
		}
		graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;							// 書き込むDSVのフォーマット
		graphicsPipelineStateDesc.DepthStencilState = depthStencil_;									// DepthStencilState
		graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;								// サンプルマスク
		graphicsPipelineStateDesc.SampleDesc.Count = 1;													// サンプル数
		graphicsPipelineStateDesc.PrimitiveTopologyType = topologyType_;								// PrimitiveTopologyType

		// グラフィックスパイプラインステートの生成
		HRESULT hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState));
		assert(SUCCEEDED(hr));
	} else {
		// コンピュートパイプラインステートの設定
		D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc{};
		computePipelineStateDesc.pRootSignature = rootSignature;	// RootSignature
		computePipelineStateDesc.CS = computeShader_;				// ComputeShader

		// コンピュートパイプラインステートの生成
		HRESULT hr = device->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&pipelineState));
		assert(SUCCEEDED(hr));
	}

	return pipelineState;
}

PipelineState PipelineState::AddInput(LPCSTR name, UINT index, DXGI_FORMAT format, UINT alignedByteOffset) {
	D3D12_INPUT_ELEMENT_DESC inputElement{};
	inputElement.SemanticName = name;					// セマンティクス名
	inputElement.SemanticIndex = index;					// セマンティックインデックス
	inputElement.Format = format;						// 要素データの形式
	inputElement.AlignedByteOffset = alignedByteOffset;	// 頂点の先頭からこの要素へのオフセット
	inputElements_.emplace_back(inputElement);
	return *this;
}

PipelineState PipelineState::AddRenderTargetFormat(DXGI_FORMAT format) {
	renderTargetFormats_.emplace_back(format);
	return *this;
}

PipelineState PipelineState::SetBlendState(const D3D12_BLEND_DESC &blend) {
	blend_ = blend;
	return *this;
}

PipelineState PipelineState::SetRasterizer(const D3D12_RASTERIZER_DESC &rasterizer) {
	rasterizer_ = rasterizer;
	return *this;
}

PipelineState PipelineState::SetDepthState(const D3D12_DEPTH_STENCIL_DESC &depthStencil) {
	depthStencil_ = depthStencil;
	return *this;
}

PipelineState PipelineState::SetVertexShader(LPVOID pShader, SIZE_T size) {
	vertexShader_.pShaderBytecode = pShader;
	vertexShader_.BytecodeLength = size;
	return *this;
}

PipelineState PipelineState::SetPixelShader(LPVOID pShader, SIZE_T size) {
	pixelShader_.pShaderBytecode = pShader;
	pixelShader_.BytecodeLength = size;
	return *this;
}

PipelineState PipelineState::SetComputeShader(LPVOID pShader, SIZE_T size) {
	computeShader_.pShaderBytecode = pShader;
	computeShader_.BytecodeLength = size;
	return *this;
}

PipelineState PipelineState::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType) {
	topologyType_ = topologyType;
	return *this;
}

Microsoft::WRL::ComPtr<IDxcBlob> PipelineState::CompileShader(std::ofstream &logStream, const std::wstring &filePath, const wchar_t *profile, IDxcUtils *dxcUtils, IDxcCompiler3 *dxcCompiler, IDxcIncludeHandler *includeHandler) {
	// これからシェーダーをコンパイルする旨をログに出す
	Logger::Log(logStream, StringConverter::ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));

	// hlslファイルを読む
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr));

	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer{};
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	// コンパイルオプションを設定する
	LPCWSTR arguments[] = {
		filePath.c_str(),	// シェーダーのファイル名
		L"-E", L"main",		// エントリーポイント
		L"-T", profile,		// プロファイル
		L"-Zi",				// デバッグ情報を出力する
		L"-Qembed_debug",	// デバッグ情報を埋め込む
		L"-Od",				// 最適化しない
		L"-Zpr",			// パイプラインコンパイラを使用する
	};

	// コンパイルを実行する
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,		// シェーダーのソースコード
		arguments,					// コンパイルオプション
		_countof(arguments),		// コンパイルオプションの数
		includeHandler,				// インクルードハンドラ
		IID_PPV_ARGS(&shaderResult)	// コンパイル結果を受け取る
	);
	assert(SUCCEEDED(hr));

	// 警告・エラーのチェック
	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	assert(SUCCEEDED(hr));

	// エラーがあった場合はログに出力する
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Logger::Log(logStream, shaderError->GetStringPointer());
		assert(false);
	}

	// コンパイル結果を受け取る
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));

	// コンパイル結果をログに出力する
	Logger::Log(logStream, StringConverter::ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));

	// コンパイル結果を返す
	return shaderBlob;
}