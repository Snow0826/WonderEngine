#pragma once
#include <wrl/client.h>
#include <xaudio2.h>
#include <string>
#include <vector>

/// @brief オーディオ
class Audio final {

	/// @brief 音声データ
	struct SoundData final {
		std::string name;							// 音声名
		WAVEFORMATEX *format = {};					// 波形フォーマット
		std::vector<uint8_t> mediaData;				// メディアバッファデータ
		IXAudio2SourceVoice *sourceVoice = nullptr;	// ソースボイス
	};

public:
	/// @brief コンストラクタ
	/// @param logStream ログ出力用ストリーム
	Audio(std::ofstream *logStream);

	/// @brief デストラクタ
	~Audio();

	/// @brief 初期化
	void Initialize();

	/// @brief 音声データの読み込み
	/// @param filePath 音声データのファイル名
	/// @return 音声ハンドル
	uint32_t Load(const std::string &fileName);

	/// @brief 音声データのアンロード
	/// @param soundHandle 音声ハンドル
	void Unload(uint32_t soundHandle);

	/// @brief 音声を再生する
	/// @param soundHandle 音声ハンドル
	/// @param loop ループするかどうか
	/// @param volume 音量(0.0f～1.0f)
	void Play(uint32_t soundHandle, bool loop = true, float volume = 1.0f);

	/// @brief 音声を停止する
	/// @param soundHandle 音声ハンドル
	void Stop(uint32_t soundHandle);

	/// @brief 音声を一時停止する
	/// @param soundHandle 音声ハンドル
	void Pause(uint32_t soundHandle);

	/// @brief 音声を再開する
	/// @param soundHandle 音声ハンドル
	void Resume(uint32_t soundHandle);

	/// @brief 音量を設定する
	/// @param soundHandle 音声ハンドル
	/// @param volume 音量(0.0f～1.0f)
	void SetVolume(uint32_t soundHandle, float volume);

	/// @brief 音声のピッチを設定する
	/// @param soundHandle 音声ハンドル
	/// @param pitch ピッチ(0.0f～1.0f)
	void SetPitch(uint32_t soundHandle, float pitch);

	/// @brief 音声のパンを設定する
	/// @param soundHandle 音声ハンドル
	/// @param pan パン(-1.0f～1.0f)
	void SetPan(uint32_t soundHandle, float pan);

	/// @brief 音声が再生中かどうか取得する
	/// @param soundHandle 音声ハンドル
	/// @return 再生中かどうか
	bool IsPlaying(uint32_t soundHandle) const;

private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_ = nullptr;	// XAudio2インターフェース
	std::ofstream *logStream_;								// ログ出力用ストリーム
	std::vector<SoundData> soundDataList_;					// 音声データリスト
};