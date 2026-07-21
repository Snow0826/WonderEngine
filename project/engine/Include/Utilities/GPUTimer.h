#pragma once
#include <wrl/client.h>
#include <d3d12.h>
#include <memory>

class Device;
class Resource;

/// @brief GPUタイマー
class GPUTimer {
public:
	/// @brief コンストラクタ
	/// @param device デバイス
	GPUTimer(Device *device);

	/// @brief デストラクタ
	~GPUTimer();
	
	/// @brief タイマーの開始
	void Begin();
	
	/// @brief タイマーの終了
	void End();

	/// @brief Query結果をReadbackBufferにコピーする
	void Resolve();

	/// @brief 経過時間の取得
	/// @return 経過時間(ミリ秒)
	double GetMs() const;

private:
	Device *device_ = nullptr;										// デバイス
	Microsoft::WRL::ComPtr<ID3D12QueryHeap> queryHeap_ = nullptr;	// クエリヒープ
	std::unique_ptr<Resource> readbackBuffer_ = nullptr;			// 読み取りバッファ
	uint64_t *timestamps = nullptr;									// タイムスタンプ
};