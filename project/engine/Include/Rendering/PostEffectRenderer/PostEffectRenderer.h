#pragma once

class Device;
class World;

/// @brief ポストエフェクトレンダラー
class PostEffectRenderer {
public:
	/// @brief コンストラクタ
	/// @param device デバイス
	/// @param world ワールド
	PostEffectRenderer(Device *device, World *world) : device_(device), world_(world) {}

	/// @brief デストラクタ
	virtual ~PostEffectRenderer() = default;

	/// @brief 描画
	void Render();

protected:
	/// @brief バインド
	virtual void Bind() = 0;

	/// @brief デバイスを取得する
	/// @return デバイス
	Device *GetDevice() const { return device_; }

	/// @brief ワールドを取得する
	/// @return ワールド
	World *GetWorld() const { return world_; }

private:
	Device *device_ = nullptr;	// デバイス
	World *world_ = nullptr;	// ワールド
};