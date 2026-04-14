#pragma once
#include <d3d12.h>
#include "DirectXTex.h"
#include <string>
#include <map>
#include <memory>

class Device;
class Resource;

/// @brief テクスチャマネージャー
class TextureManager final {

	/// @brief テクスチャ
	struct Texture final {
		uint32_t readHandle = 0;									// 読み取りハンドル
		uint32_t writeHandle = 0;									// 書き込みハンドル
		std::unique_ptr<Resource> resource = nullptr;				// テクスチャリソース
		std::unique_ptr<Resource> intermediateResource = nullptr;	// 中間リソース
	};

public:
	/// @brief コンストラクタ
	/// @param device デバイス
	/// @param logStream ログ出力ストリーム
	TextureManager(Device *device, std::ofstream *logStream);

	/// @brief デストラクタ
	~TextureManager();

	/// @brief テクスチャの読み込み
	/// @param fileName テクスチャ名
	/// @return テクスチャハンドル
	uint32_t LoadTexture(const std::string &fileName);

	/// @brief テクスチャ読み取りハンドルの取得
	/// @param fileName テクスチャ名
	/// @return テクスチャ読み取りハンドル
	uint32_t GetTextureReadHandle(const std::string &fileName) const;

	/// @brief テクスチャ書き込みハンドルの取得
	/// @param fileName テクスチャ名
	/// @return テクスチャ書き込みハンドル
	uint32_t GetTextureWriteHandle(const std::string &fileName) const;

	/// @brief テクスチャを編集する
	/// @param label ラベル
	/// @param fileName ファイル名
	/// @return テクスチャハンドル
	uint32_t EditTexture(const std::string &label, std::string &fileName);

	/// @brief リソースの設定を取得する
	/// @param fileName ファイル名
	/// @return リソースの設定
	D3D12_RESOURCE_DESC GetResourceDesc(const std::string &fileName) const;

	/// @brief テクスチャリソースの取得
	/// @param fileName ファイル名
	/// @return テクスチャリソース
	Resource *GetTextureResource(const std::string &fileName) const;

private:
	Device *device_ = nullptr;												// デバイス
	std::ofstream *logStream_ = nullptr;									// ログ出力用のストリーム
	std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;	// テクスチャリスト
	DirectX::ScratchImage mipImages_;										// ミップマップ付きのデータ

	/// @brief テクスチャの読み込み
	/// @param directoryPath ディレクトリパス
	/// @param fileName ファイル名
	/// @return ミップマップ付きのデータ
	DirectX::ScratchImage LoadTexture(const std::string &directoryPath, const std::string &fileName);
};