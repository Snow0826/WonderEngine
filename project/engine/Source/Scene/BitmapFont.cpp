#include "BitmapFont.h"
#include "EntityComponentSystem.h"
#include "BlendMode.h"
#include "Object.h"
#include "Sprite.h"
#include "Transform.h"
#include "Material.h"

void BitmapFont::Initialize() {
	for (size_t i = 0; i < kDigitCount; i++) {
		// スプライトの設定
		Sprite sprite = spriteManager_->CreateSprite("0.png");
		sprite.spriteData.position = { static_cast<float>(i * 64), 0.0f };

		// エンティティの生成
		uint32_t entity = registry_->GenerateEntity();
		registry_->AddComponent(entity, BlendMode::kBlendModeNormal);
		registry_->AddComponent(entity, objectManager_->CreateObject(entity));
		registry_->AddComponent(entity, sprite);
		registry_->AddComponent(entity, Transform{});
		registry_->AddComponent(entity, Material{ .enableLighting = false });
		digitEntities_.emplace_back(entity);
	}
}

void BitmapFont::ToSprite(uint32_t value) {
	uint32_t temp = value;
	uint32_t div = static_cast<uint32_t>(std::pow(10, kDigitCount - 1));
	for (uint32_t i = 0; i < kDigitCount; ++i) {
		uint32_t index = temp / div;
		temp %= div;
		div /= 10;
		Sprite *sprite = registry_->GetComponent<Sprite>(digitEntities_[i]);
		if (sprite) {
			sprite->textureHandle = textureHandles_[index];
		}
	}
}

void BitmapFont::Clear() {
	for (size_t i = 0; i < kDigitCount; i++) {
		registry_->RemoveAllComponents(digitEntities_[i]);
	}
}