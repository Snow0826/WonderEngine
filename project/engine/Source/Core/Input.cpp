#include "Input.h"
#include "Window.h"
#include "Vector3.h"
#include <cassert>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput9_1_0.lib")

void Input::Initialize(const Window &window) {
	// DirectInputの初期化
	HRESULT hr = DirectInput8Create(
		window.GetHInstance(),
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void **)&directInput_, nullptr);
	assert(SUCCEEDED(hr));

	// キーボードデバイスの生成
	hr = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, nullptr);
	assert(SUCCEEDED(hr));

	// マウスデバイスの生成
	hr = directInput_->CreateDevice(GUID_SysMouse, &mouse_, nullptr);
	assert(SUCCEEDED(hr));

	// キーボードの入力データ形式の設定
	hr = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));

	// マウスの入力データ形式の設定
	hr = mouse_->SetDataFormat(&c_dfDIMouse2);
	assert(SUCCEEDED(hr));

	// キーボードの排他制御レベルの設定
	hr = keyboard_->SetCooperativeLevel(
		window.GetHandle(),
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	// マウスの排他制御レベルの設定
	hr = mouse_->SetCooperativeLevel(
		window.GetHandle(),
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(hr));

	// ジョイスティックの状態の初期化
	ZeroMemory(&joyStickState_, sizeof(XINPUT_STATE));
	ZeroMemory(&preJoyStickState_, sizeof(XINPUT_STATE));
}

void Input::Update() {
	// キーボード情報の取得開始
	keyboard_->Acquire();

	// マウス情報の取得開始
	mouse_->Acquire();

	// キーボードの前フレームの状態を保存する
	preKey_ = key_;

	// マウスの前フレームの状態を保存する
	preMouseState_ = mouseState_;

	// ジョイスティックの前フレームの状態を保存する
	preJoyStickState_ = joyStickState_;

	// キーボードの入力状態を取得する
	keyboard_->GetDeviceState(sizeof(key_), key_.data());

	// マウスの入力状態を取得する
	mouse_->GetDeviceState(sizeof(mouseState_), &mouseState_);

	// ジョイスティックの入力状態を取得する
	connected_ = XInputGetState(0, &joyStickState_) == ERROR_SUCCESS;
}

bool Input::IsPressKey(uint8_t keyCode) const {
	return key_[keyCode];
}

bool Input::IsTriggerKey(uint8_t keyCode) const {
	return key_[keyCode] && !preKey_[keyCode];
}

bool Input::IsReleaseKey(uint8_t keyCode) const {
	return !key_[keyCode] && preKey_[keyCode];
}

bool Input::IsPressMouse(uint8_t mouseCode) const {
	return mouseState_.rgbButtons[mouseCode];
}

bool Input::IsTriggerMouse(uint8_t mouseCode) const {
	return mouseState_.rgbButtons[mouseCode] && !preMouseState_.rgbButtons[mouseCode];
}

bool Input::IsReleaseMouse(uint8_t mouseCode) const {
	return !mouseState_.rgbButtons[mouseCode] && preMouseState_.rgbButtons[mouseCode];
}

bool Input::IsPressButton(int32_t buttonCode) const {
	return (joyStickState_.Gamepad.wButtons & buttonCode) != 0;
}

bool Input::IsTriggerButton(int32_t buttonCode) const {
	return (joyStickState_.Gamepad.wButtons & buttonCode) != 0 &&
		(preJoyStickState_.Gamepad.wButtons & buttonCode) == 0;
}

bool Input::IsReleaseButton(int32_t buttonCode) const {
	return (joyStickState_.Gamepad.wButtons & buttonCode) == 0 &&
		(preJoyStickState_.Gamepad.wButtons & buttonCode) != 0;
}

Vector3 Input::GetMouseMove() const {
	return Vector3{
		static_cast<float>(mouseState_.lX),
		static_cast<float>(mouseState_.lY),
		static_cast<float>(mouseState_.lZ)
	};
}

void Input::SetVibration(DWORD dwUserIndex, WORD leftMotor, WORD rightMotor) {
	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
	vibration.wLeftMotorSpeed = leftMotor;   // 0 ～ 65535
	vibration.wRightMotorSpeed = rightMotor;  // 0 ～ 65535
	XInputSetState(dwUserIndex, &vibration);
}

float Input::NormalizeAxis(SHORT value, int deadzone) {
	if (value > deadzone) {
		return static_cast<float>(value - deadzone) / static_cast<float>(32767 - deadzone);
	} else if (value < -deadzone) {
		return static_cast<float>(value + deadzone) / static_cast<float>(32768 - deadzone);
	}
	return 0.0f;
}

float Input::NormalizeTrigger(BYTE value, int deadzone) {
	if (value > deadzone) {
		return static_cast<float>(value - deadzone) / static_cast<float>(255 - deadzone);
	}
	return 0.0f;
}
