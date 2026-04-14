#include "Button.h"
#include "EntityComponentSystem.h"
#include "BlendMode.h"
#include "Sprite.h"
#include "Object.h"
#include "Transform.h"
#include "Material.h"
#include "Easing.h"

void Button::Initialize() {
	// スプライトの初期化
	Sprite sprite = spriteManager_->CreateSprite("T_X_A_Color_3D.png");
	sprite.spriteData.position = { 640.0f, 540.0f };
	sprite.spriteData.size = kMaxButtonSize;
	sprite.spriteData.anchorPoint = { 0.5f, 0.5f };

	// エンティティの生成
	entity_ = registry_->GenerateEntity();
	registry_->AddComponent(entity_, BlendMode::kBlendModeNormal);
	registry_->AddComponent(entity_, sprite);
	registry_->AddComponent(entity_, objectManager_->CreateObject(entity_));
	registry_->AddComponent(entity_, Transform{});
	registry_->AddComponent(entity_, Material{ .enableLighting = false });
}

void Button::Update() {
	// アニメーションの更新
	if (isAnimating_) {
		if (animationTimer_++ < kAnimationDuration) {
			Sprite *sprite = registry_->GetComponent<Sprite>(entity_);
			if (sprite) {
				sprite->spriteData.size = Easing<Vector2>::InOutCirc(kMaxButtonSize, kMinButtonSize, kMaxButtonSize, animationTimer_, kAnimationDuration);
			}
		} else {
			isAnimating_ = false;
			animationTimer_ = 0;
		}
	}
}