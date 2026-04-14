#include "Audio.h"
#include "StringConverter.h"
#include "Logger.h"
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <cassert>

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

Audio::Audio(std::ofstream *logStream) : logStream_(logStream) {}

Audio::~Audio() {
	for (size_t i = 0; i < soundDataList_.size(); i++) {
		if (soundDataList_[i].sourceVoice) {
			soundDataList_[i].sourceVoice->DestroyVoice();
			soundDataList_[i].sourceVoice = nullptr;
		}
	}
	xAudio2_.Reset();
	for (size_t i = 0; i < soundDataList_.size(); i++) {
		Unload(static_cast<uint32_t>(i));
	}
	HRESULT hr = MFShutdown();
	assert(SUCCEEDED(hr));
}

void Audio::Initialize() {
	HRESULT hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(hr));

	hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));

	IXAudio2MasteringVoice *masteringVoice = nullptr;
	hr = xAudio2_->CreateMasteringVoice(&masteringVoice);
	assert(SUCCEEDED(hr));
}

uint32_t Audio::Load(const std::string &fileName) {
	// 音声ハンドルを取得する
	uint32_t soundHandle = static_cast<uint32_t>(soundDataList_.size());

	// 読み込まれているかチェックする
	for (uint32_t i = 0; i < soundDataList_.size(); ++i) {
		if (soundDataList_[i].name == fileName) {
			Logger::Log(*logStream_, "SoundData already loaded: " + fileName + " with handle: " + std::to_string(i) + "\n");
			return i;
		}
	}

	Microsoft::WRL::ComPtr<IMFSourceReader> pMFSourceReader = nullptr;
	HRESULT hr = MFCreateSourceReaderFromURL(StringConverter::ConvertString("resources/audio/" + fileName).c_str(), nullptr, &pMFSourceReader);
	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<IMFMediaType> pMFMediaType = nullptr;
	hr = MFCreateMediaType(&pMFMediaType);
	assert(SUCCEEDED(hr));

	hr = pMFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	assert(SUCCEEDED(hr));

	hr = pMFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	assert(SUCCEEDED(hr));

	hr = pMFSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pMFMediaType.Get());
	assert(SUCCEEDED(hr));

	pMFMediaType.Reset();
	pMFMediaType = nullptr;
	hr = pMFSourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pMFMediaType);
	assert(SUCCEEDED(hr));

	SoundData soundData;
	soundData.name = fileName;
	hr = MFCreateWaveFormatExFromMFMediaType(pMFMediaType.Get(), &soundData.format, nullptr);
	assert(SUCCEEDED(hr));

	while (true) {
		Microsoft::WRL::ComPtr<IMFSample> pMFSample = nullptr;
		DWORD dwStreamFlags = 0;
		hr = pMFSourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &dwStreamFlags, nullptr, &pMFSample);
		assert(SUCCEEDED(hr));

		if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
			break;
		}

		Microsoft::WRL::ComPtr<IMFMediaBuffer> pMFMediaBuffer = nullptr;
		hr = pMFSample->ConvertToContiguousBuffer(&pMFMediaBuffer);
		assert(SUCCEEDED(hr));

		BYTE *pBuffer = nullptr;
		DWORD cbCurrentLength{ 0 };
		hr = pMFMediaBuffer->Lock(&pBuffer, nullptr, &cbCurrentLength);
		assert(SUCCEEDED(hr));

		soundData.mediaData.resize(soundData.mediaData.size() + cbCurrentLength);
		memcpy(soundData.mediaData.data() + soundData.mediaData.size() - cbCurrentLength, pBuffer, cbCurrentLength);

		hr = pMFMediaBuffer->Unlock();
		assert(SUCCEEDED(hr));
	}

	// 音声データリストへ格納
	soundDataList_.emplace_back(soundData);
	Logger::Log(*logStream_, "Loaded soundData: " + fileName + " with handle: " + std::to_string(soundHandle) + "\n");
	return soundHandle;
}

void Audio::Unload(uint32_t soundHandle) {
	soundDataList_[soundHandle].mediaData.clear();
	soundDataList_[soundHandle].format = {};
}

void Audio::Play(uint32_t soundHandle, bool loop, float volume) {
	HRESULT hr = xAudio2_->CreateSourceVoice(&soundDataList_[soundHandle].sourceVoice, soundDataList_[soundHandle].format);
	assert(SUCCEEDED(hr));

	XAUDIO2_BUFFER buffer{};
	buffer.AudioBytes = static_cast<UINT32>(soundDataList_[soundHandle].mediaData.size()) * sizeof(BYTE);
	buffer.pAudioData = soundDataList_[soundHandle].mediaData.data();
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : XAUDIO2_NO_LOOP_REGION;

	hr = soundDataList_[soundHandle].sourceVoice->SubmitSourceBuffer(&buffer);
	assert(SUCCEEDED(hr));
	hr = soundDataList_[soundHandle].sourceVoice->SetVolume(volume);
	assert(SUCCEEDED(hr));
	hr = soundDataList_[soundHandle].sourceVoice->Start();
	assert(SUCCEEDED(hr));
}

void Audio::Stop(uint32_t soundHandle) {
	HRESULT hr = soundDataList_[soundHandle].sourceVoice->Stop();
	assert(SUCCEEDED(hr));
	hr = soundDataList_[soundHandle].sourceVoice->FlushSourceBuffers();
	assert(SUCCEEDED(hr));
}

void Audio::Pause(uint32_t soundHandle) {
	HRESULT hr = soundDataList_[soundHandle].sourceVoice->Stop();
	assert(SUCCEEDED(hr));
}

void Audio::Resume(uint32_t soundHandle) {
	HRESULT hr = soundDataList_[soundHandle].sourceVoice->Start();
	assert(SUCCEEDED(hr));
}

void Audio::SetVolume(uint32_t soundHandle, float volume) {
	HRESULT hr = soundDataList_[soundHandle].sourceVoice->SetVolume(volume);
	assert(SUCCEEDED(hr));
}

void Audio::SetPitch(uint32_t soundHandle, float pitch) {
	HRESULT hr = soundDataList_[soundHandle].sourceVoice->SetFrequencyRatio(pitch);
	assert(SUCCEEDED(hr));
}

void Audio::SetPan(uint32_t soundHandle, float pan) {
	// パンの範囲を-1.0f～1.0fに制限
	if (pan < -1.0f) {
		pan = -1.0f;
	} else if (pan > 1.0f) {
		pan = 1.0f;
	}

	// 左右の音量を計算
	float leftVolume = (pan <= 0.0f) ? 1.0f : 1.0f - pan;
	float rightVolume = (pan >= 0.0f) ? 1.0f : 1.0f + pan;

	// ボイスの音量を設定
	HRESULT hr = soundDataList_[soundHandle].sourceVoice->SetOutputMatrix(nullptr, 2, 2, new float[4] { leftVolume, 0.0f, 0.0f, rightVolume });
	assert(SUCCEEDED(hr));
}

bool Audio::IsPlaying(uint32_t soundHandle) const {
	XAUDIO2_VOICE_STATE state;
	soundDataList_[soundHandle].sourceVoice->GetState(&state);
	return state.BuffersQueued > 0;
}