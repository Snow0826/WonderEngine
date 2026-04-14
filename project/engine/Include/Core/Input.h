#pragma once
#include <wrl/client.h>
#include <Xinput.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <array>

class Window;
struct Vector3;

/// @brief 入力
class Input final {
public:
	template <class T> 	using ComPtr = Microsoft::WRL::ComPtr<T>;

	/// @brief 初期化
	/// @param window ウィンドウ
	void Initialize(const Window &window);

	/// @brief 更新
	void Update();

	/// @brief キーの押されている状態をチェックする
	/// @param keyCode DirectInputのキーコード
	/// @return キーの押されている状態かどうか
	bool IsPressKey(uint8_t keyCode) const;

	/// @brief キーの押した瞬間をチェックする
	/// @param keyCode DirectInputのキーコード
	/// @return キーの押した瞬間かどうか
	bool IsTriggerKey(uint8_t keyCode) const;

	/// @brief キーの離した瞬間をチェックする
	/// @param keyCode DirectInputのキーコード
	/// @return キーの離した瞬間かどうか
	bool IsReleaseKey(uint8_t keyCode) const;

	/// @brief マウスの押されいる状態をチェックする
	/// @param mouseCode DirectInputのマウスコード
	/// @return マウスの押されている状態かどうか
	bool IsPressMouse(uint8_t mouseCode) const;

	/// @brief マウスの押した瞬間をチェックする
	/// @param mouseCode DirectInputのマウスコード
	/// @return マウスの押した瞬間かどうか
	bool IsTriggerMouse(uint8_t mouseCode) const;

	/// @brief マウスの離した瞬間をチェックする
	/// @param mouseCode DirectInputのマウスコード
	/// @return マウスの離した瞬間かどうか
	bool IsReleaseMouse(uint8_t mouseCode) const;

	/// @brief ボタンの押されている状態をチェックする
	/// @param buttonCode XInputのボタンコード
	/// @return ボタンの押されている状態かどうか
	bool IsPressButton(int32_t buttonCode) const;

	/// @brief ボタンの押した瞬間をチェックする
	/// @param buttonCode XInputのボタンコード
	/// @return ボタンの押した瞬間かどうか
	bool IsTriggerButton(int32_t buttonCode) const;

	/// @brief ボタンの離した瞬間をチェックする
	/// @param buttonCode XInputのボタンコード
	/// @return ボタンの離した瞬間かどうか
	bool IsReleaseButton(int32_t buttonCode) const;

	/// @brief マウスの移動量を取得する
	/// @return マウスの移動量
	Vector3 GetMouseMove() const;

	/// @brief コントローラーの振動を設定する
	/// @param dwUserIndex ユーザーインデックス
	/// @param leftMotor 左モーター
	/// @param rightMotor 右モーター
	void SetVibration(DWORD dwUserIndex, WORD leftMotor, WORD rightMotor);

	/// @brief 軸の値を正規化する
	/// @param value 軸の値
	/// @param deadzone デッドゾーン
	/// @return 正規化した軸の値
	float NormalizeAxis(SHORT value, int deadzone);

	/// @brief トリガーの値を正規化する
	/// @param value トリガーの値
	/// @param deadzone デッドゾーン
	/// @return 正規化したトリガーの値
	float NormalizeTrigger(BYTE value, int deadzone);

	/// @brief 現在フレームのジョイスティックの状態を取得する
	/// @return 現在フレームのジョイスティックの状態
	const XINPUT_STATE &GetJoyStickState() const { return joyStickState_; }

	/// @brief 前フレームのジョイスティックの状態を取得する
	/// @return 前フレームのジョイスティックの状態
	const XINPUT_STATE &GetJoyStickStatePrevious() const { return preJoyStickState_; }

	/// @brief コントローラーの接続フラグを取得する
	/// @return コントローラーの接続フラグ
	bool Connected() const { return connected_; }

private:
	ComPtr<IDirectInput8> directInput_ = nullptr;		// DirectInput
	ComPtr<IDirectInputDevice8> keyboard_ = nullptr;	// キーボードデバイス
	ComPtr<IDirectInputDevice8> mouse_ = nullptr;		// マウスデバイス
	std::array<BYTE, 256> key_{};						// 現在フレームのキーの状態
	std::array<BYTE, 256> preKey_{};					// 前フレームのキーの状態
	DIMOUSESTATE2 mouseState_{};						// 現在フレームのマウスの状態
	DIMOUSESTATE2 preMouseState_{};						// 前フレームのマウスの状態
	XINPUT_STATE joyStickState_{};						// 現在フレームのジョイスティックの状態
	XINPUT_STATE preJoyStickState_{};					// 前フレームのジョイスティックの状態
	bool connected_ = false;							// コントローラーの接続フラグ
};