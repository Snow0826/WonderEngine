#pragma once
#include <wrl/client.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <vector>
#include <string>

/// @brief パイプラインステート
class PipelineState final {
public:
	/// @brief PSOの作成
	/// @param device デバイス
	/// @param rootSignature ルートシグネチャ
	/// @return パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> Create(ID3D12Device *device, ID3D12RootSignature *rootSignature);
	
	/// @brief InputElementの追加
	/// @param name セマンティクス名
	/// @param index セマンティックインデックス
	/// @param format 要素データの形式
	/// @param alignedByteOffset 頂点の先頭からこの要素へのオフセット
	/// @return パイプラインステート
	PipelineState AddInput(LPCSTR name, UINT index, DXGI_FORMAT format, UINT alignedByteOffset);

	/// @brief レンダーターゲットフォーマットの追加
	/// @param format フォーマット
	/// @return パイプラインステート
	PipelineState AddRenderTargetFormat(DXGI_FORMAT format);

	/// @brief ブレンドの設定
	/// @param blendState ブレンドの状態
	/// @return パイプラインステート
	PipelineState SetBlendState(const D3D12_BLEND_DESC &blend);

	/// @brief ラスタライザーの設定
	/// @param rasterizer ラスタライザーの状態
	/// @return パイプラインステート
	PipelineState SetRasterizer(const D3D12_RASTERIZER_DESC &rasterizer);

	/// @brief 深度ステンシルの設定
	/// @param depthStencil 深度ステンシルの状態
	/// @return パイプラインステート
	PipelineState SetDepthState(const D3D12_DEPTH_STENCIL_DESC &depthStencil);

	/// @brief 頂点シェーダーの設定
	/// @param pShader シェーダーのポインタ
	/// @param size シェーダーのサイズ
	/// @return パイプラインステート
	PipelineState SetVertexShader(LPVOID pShader, SIZE_T size);

	/// @brief ピクセルシェーダーの設定
	/// @param pShader シェーダーのポインタ
	/// @param size シェーダーのサイズ
	/// @return パイプラインステート
	PipelineState SetPixelShader(LPVOID pShader, SIZE_T size);

	/// @brief コンピュートシェーダーの設定
	/// @param pShader シェーダーのポインタ
	/// @param size シェーダーのサイズ
	/// @return パイプラインステート
	PipelineState SetComputeShader(LPVOID pShader, SIZE_T size);

	/// @brief プリミティブトポロジの種類を設定
	/// @param topologyType プリミティブトポロジの種類
	/// @return パイプラインステート
	PipelineState SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType);

	/// @brief シェーダーコンパイル
	/// @param os ログの出力ストリーム
	/// @param filePath CompilerするShaderファイルへのパス
	/// @param profile Compileに使用するProfile
	/// @param dxcUtils DXCのユーティリティ
	/// @param dxcCompiler DXCのコンパイラ
	/// @param includeHandler インクルードハンドラ
	static Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(std::ofstream &logStream, const std::wstring &filePath, const wchar_t *profile, IDxcUtils *dxcUtils, IDxcCompiler3 *dxcCompiler, IDxcIncludeHandler *includeHandler);

private:
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements_{};	// InputAssemblerStageの要素リスト
	std::vector<DXGI_FORMAT> renderTargetFormats_{};		// レンダーターゲットフォーマットリスト
	D3D12_BLEND_DESC blend_{};								// BlendState
	D3D12_RASTERIZER_DESC rasterizer_{};					// RasterizerState
	D3D12_DEPTH_STENCIL_DESC depthStencil_{};				// DepthStencilState
	D3D12_SHADER_BYTECODE vertexShader_{ nullptr, 0 };		// 頂点シェーダー
	D3D12_SHADER_BYTECODE pixelShader_{ nullptr, 0 };		// ピクセルシェーダー
	D3D12_SHADER_BYTECODE computeShader_{ nullptr, 0 };		// コンピュートシェーダー
	D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType_{};			// PrimitiveTopologyType
};

