#include "Text.h"
#include "EntityComponentSystem.h"
#include "Renderer.h"
#include "Object.h"
#include "Sprite.h"
#include "Transform.h"
#include "Material.h"
#include "Quaternion.h"

Text TextManager::CreateText(const std::string &text, float x, float y) {
	Text textData;
	for (size_t i = 0; i < text.size(); ++i) {
		Sprite sprite = spriteManager_->CreateSprite("debugfont.png");
		int32_t ascii = static_cast<uint8_t>(text[i]);
		int32_t index = ascii - 32;   // 画像はスペース(32)から始まっている
		int32_t col = index % kFontLineCount;
		int32_t row = index / kFontLineCount;
		sprite.spriteData.textureLeftTop.x = static_cast<float>(col * kFontWidth);
		sprite.spriteData.textureLeftTop.y = static_cast<float>(row * kFontHeight);
		sprite.spriteData.textureSize.x = static_cast<float>(kFontWidth);
		sprite.spriteData.textureSize.y = static_cast<float>(kFontHeight);
		sprite.spriteData.size.x = static_cast<float>(kFontWidth);
		sprite.spriteData.size.y = static_cast<float>(kFontHeight);
		sprite.spriteData.position.x = x + static_cast<float>(kFontWidth) * static_cast<float>(i); // 次の文字の位置に移動
		sprite.spriteData.position.y = y;
		uint32_t entity = registry_->GenerateEntity();
		registry_->AddComponent(entity, BlendMode::kBlendModeNormal);
		registry_->AddComponent(entity, objectManager_->CreateObject(entity));
		registry_->AddComponent(entity, sprite);
		registry_->AddComponent(entity, Transform{});
		registry_->AddComponent(entity, Material{ .enableLighting = false });
		textData.entities.emplace_back(entity);
	}
	return textData;
}

void TextManager::CreateMatrix4x4Text(const std::string &label, const Matrix4x4 &matrix, float x, float y) {
	CreateText(label, x, y);
	for (size_t row = 0; row < 4; row++) {
		for (size_t column = 0; column < 4; column++) {
			CreateText(std::to_string(matrix.m[row][column]), x + static_cast<float>(column) * static_cast<float>(kColumnWidth), y + static_cast<float>(kRowHeight) + static_cast<float>(row) * static_cast<float>(kRowHeight));
		}
	}
}

void TextManager::CreateVector3Text(const std::string &label, const Vector3 &vector, float x, float y) {
	CreateText(label, x, y);
	CreateText(std::to_string(vector.x), x, y + static_cast<float>(kRowHeight));
	CreateText(std::to_string(vector.y), x + static_cast<float>(kColumnWidth), y + static_cast<float>(kRowHeight));
	CreateText(std::to_string(vector.z), x + static_cast<float>(kColumnWidth) * 2.0f, y + static_cast<float>(kRowHeight));
}

void TextManager::CreateQuaternionText(const std::string &label, const Quaternion &quaternion, float x, float y) {
	CreateText(label, x, y);
	CreateText(std::to_string(quaternion.x), x, y + static_cast<float>(kRowHeight));
	CreateText(std::to_string(quaternion.y), x + static_cast<float>(kColumnWidth), y + static_cast<float>(kRowHeight));
	CreateText(std::to_string(quaternion.z), x + static_cast<float>(kColumnWidth) * 2.0f, y + static_cast<float>(kRowHeight));
	CreateText(std::to_string(quaternion.w), x + static_cast<float>(kColumnWidth) * 3.0f, y + static_cast<float>(kRowHeight));
}

void TextManager::SetText(const Text &textData, const std::string &text) {
	for (size_t i = 0; i < textData.entities.size() && i < text.size(); ++i) {
		Sprite *sprite = registry_->GetComponent<Sprite>(textData.entities[i]);
		if (sprite) {
			int32_t ascii = static_cast<uint8_t>(text[i]);
			int32_t index = ascii - 32;   // 画像はスペース(32)から始まっている
			int32_t col = index % kFontLineCount;
			int32_t row = index / kFontLineCount;
			sprite->spriteData.textureLeftTop.x = static_cast<float>(col * kFontWidth);
			sprite->spriteData.textureLeftTop.y = static_cast<float>(row * kFontHeight);
		}
	}
}

void TextManager::SetTextPosition(const Text &textData, float x, float y) {
	for (size_t i = 0; i < textData.entities.size(); ++i) {
		Sprite *sprite = registry_->GetComponent<Sprite>(textData.entities[i]);
		if (sprite) {
			sprite->spriteData.position.x = x + static_cast<float>(kFontWidth) * static_cast<float>(i);
			sprite->spriteData.position.y = y;
		}
	}
}