#define NOMINMAX
#include "Fade.h"
#include "EntityComponentSystem.h"
#include "BlendMode.h"
#include "Object.h"
#include "Sprite.h"
#include "Material.h"
#include "Transform.h"
#include <algorithm>

void Fade::Add(const std::string &textureFileName, const Vector4 &color) {
	// スプライトの設定
	Sprite sprite = spriteManager_->CreateSprite(textureFileName);
	sprite.spriteData.size = { 1280.0f, 720.0f };

	// エンティティの生成
	entity_ = registry_->GenerateEntity();
	registry_->AddComponent(entity_, BlendMode::kBlendModeNormal);
	registry_->AddComponent(entity_, objectManager_->CreateObject(entity_));
	registry_->AddComponent(entity_, sprite);
	registry_->AddComponent(entity_, Transform{});
	registry_->AddComponent(entity_, Material{ .color = color, .enableLighting = false });
}

void Fade::Remove() {
	registry_->RemoveAllComponents(entity_);
}

void Fade::Update() {
	Material *material = registry_->GetComponent<Material>(entity_);
	if (!material) {
		return;
	}

	switch (status_) {
		case Fade::Status::None:
			break;
		case Fade::Status::FadeIn:
			// 1フレーム分の秒数をカウントアップ
			counter_ += kDeltaTime;

			// フェード継続時間に達したら打ち止め
			counter_ = std::min(counter_, duration_);

			// 0.0f~1.0fで経過時間が継続時間に近づくほどアルファ値を小さくする
			material->color.w = 1.0f - std::clamp(counter_ / duration_, 0.0f, 1.0f);
			break;
		case Fade::Status::FadeOut:
			// 1フレーム分の秒数をカウントアップ
			counter_ += kDeltaTime;

			// フェード継続時間に達したら打ち止め
			counter_ = std::min(counter_, duration_);

			// 0.0f~1.0fで経過時間が継続時間に近づくほどアルファ値を大きくする
			material->color.w = std::clamp(counter_ / duration_, 0.0f, 1.0f);
			break;
		default:
			break;
	}
}

void Fade::Start(Status status, float duration) {
	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;
	registry_->RemoveComponent<Disabled>(entity_);
}

void Fade::Stop() {
	status_ = Status::None;
	registry_->AddComponent(entity_, Disabled{});
}

bool Fade::IsFinished() const {
	switch (status_) {
		case Fade::Status::None:
			break;
		case Fade::Status::FadeIn:
			return counter_ >= duration_ ? true : false;
			break;
		case Fade::Status::FadeOut:
			return counter_ >= duration_ ? true : false;
			break;
		default:
			break;
	}
	return true;
}