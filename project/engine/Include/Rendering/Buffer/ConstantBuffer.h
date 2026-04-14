#pragma once
#include <d3d12.h>
#include <memory>
#include <string>

class Device;
class Resource;

/// @brief 定数バッファ
class ConstantBuffer final {
public:
	/// @brief デフォルトコンストラクタ
	ConstantBuffer();

	/// @brief デフォルトデストラクタ
	~ConstantBuffer();

	/// @brief 初期化
	/// @param device デバイス
	/// @param size データの大きさ
	/// @param ppData データポインタ
	void Initialize(Device *device, size_t size, uint32_t count);

	/// @brief データのコピー
	/// @param data コピーするデータ
	/// @param size データの大きさ
	/// @param index コピーするインデックス
	void CopyData(const void *data, size_t size, uint32_t index);

	/// @brief 描画用のCBVを設定する
	/// @param rootParameterIndex ルートパラメータのインデックス
	/// @param bufferLocationIndex バッファの位置インデックス
	void BindToGraphics(UINT rootParameterIndex, uint32_t bufferLocationIndex) const;

	/// @brief Compute用のCBVを設定する
	/// @param rootParameterIndex ルートパラメータのインデックス
	/// @param bufferLocationIndex バッファの位置インデックス
	void BindToCompute(UINT rootParameterIndex, uint32_t bufferLocationIndex) const;

	/// @brief 名前を設定する
	/// @param name 名前
	void SetName(const std::string &name);

	/// @brief GPU仮想アドレスを取得する
	/// @param index インデックス
	/// @return GPU仮想アドレス
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress(uint32_t index) const;

private:
	ID3D12GraphicsCommandList *commandList_ = nullptr;	// コマンドリスト
	std::unique_ptr<Resource> resource_ = nullptr;		// リソース
	UINT8 *bufferGPUAddress_ = 0;						// バッファのGPUアドレス
	size_t alignedSize_ = 0;							// アライメント済みのサイズ
	size_t bufferSize_ = 0;								// バッファの大きさ
};