#define NOMINMAX
#include "Sprite.h"
#include "Mesh.h"
#include "Texture.h"
#include "EntityComponentSystem.h"
#include "Transform.h"
#include "Material.h"

SpriteManager::SpriteManager(TextureManager *textureManager, MeshManager *meshManager, Registry *registry)
	: textureManager_(textureManager)
	, meshManager_(meshManager)
	, registry_(registry) {}

Sprite SpriteManager::CreateSprite(const std::string &textureFileName) {
	D3D12_RESOURCE_DESC resourceDesc = textureManager_->GetResourceDesc(textureFileName);
	Vector2 size = { static_cast<float>(resourceDesc.Width), static_cast<float>(resourceDesc.Height) };
	Sprite sprite;
	sprite.spriteData.size = size;
	sprite.spriteData.textureSize = size;
	sprite.initialSpriteData = sprite.spriteData;
	sprite.meshHandle = meshManager_->CreateSprite();
	sprite.textureHandle = textureManager_->GetTextureReadHandle(textureFileName);
	sprite.textureFileName = textureFileName;
	return sprite;
}

void SpriteManager::UpdateSprite() {
	registry_->ForEach<Sprite, Transform>([this](uint32_t entity, Sprite *sprite, Transform *transform) {
		VertexData *vertexData = meshManager_->GetVertexData(sprite->meshHandle);

		// 頂点座標の計算
		float left = -sprite->spriteData.anchorPoint.x;
		float top = -sprite->spriteData.anchorPoint.y;
		float right = 1.0f - sprite->spriteData.anchorPoint.x;
		float bottom = 1.0f - sprite->spriteData.anchorPoint.y;

		if (sprite->spriteData.isFlipX) {
			left = -left;
			right = -right;
		}

		if (sprite->spriteData.isFlipY) {
			top = -top;
			bottom = -bottom;
		}

		vertexData[0].position = { left, bottom, 0.0f, 1.0f };	// 左下
		vertexData[1].position = { left, top, 0.0f, 1.0f };		// 左上
		vertexData[2].position = { right, bottom, 0.0f, 1.0f };	// 右下
		vertexData[3].position = { right, top, 0.0f, 1.0f };	// 右上

		// テクスチャ座標の計算
		D3D12_RESOURCE_DESC resourceDesc = textureManager_->GetResourceDesc(sprite->textureFileName);
		float texLeft = sprite->spriteData.textureLeftTop.x / static_cast<float>(resourceDesc.Width);
		float texTop = sprite->spriteData.textureLeftTop.y / static_cast<float>(resourceDesc.Height);
		float texRight = (sprite->spriteData.textureLeftTop.x + sprite->spriteData.textureSize.x) / static_cast<float>(resourceDesc.Width);
		float texBottom = (sprite->spriteData.textureLeftTop.y + sprite->spriteData.textureSize.y) / static_cast<float>(resourceDesc.Height);

		vertexData[0].texcoord = { texLeft, texBottom };	// 左下
		vertexData[1].texcoord = { texLeft, texTop };		// 左上
		vertexData[2].texcoord = { texRight, texBottom };	// 右下
		vertexData[3].texcoord = { texRight, texTop };		// 右上

		// SRTデータの設定
		transform->translate = { sprite->spriteData.position.x, sprite->spriteData.position.y, 0.0f };
		transform->scale = { sprite->spriteData.size.x, sprite->spriteData.size.y, 1.0f };
		transform->rotate = { 0.0f, 0.0f, sprite->spriteData.rotation };
		registry_->AddComponent<DirtyTransform>(entity, DirtyTransform{});
		registry_->AddComponent<DirtyMaterial>(entity, DirtyMaterial{});
		}, exclude<Disabled>());
}

void SpriteInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Sprite")) {
		Sprite *sprite = registry_->GetComponent<Sprite>(entity);
		if (sprite) {
			ImGui::DragFloat2("Position", &sprite->spriteData.position.x, 0.1f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), "%.1f");
			ImGui::DragFloat2("Size", &sprite->spriteData.size.x, 1.0f, 0.0f, std::numeric_limits<float>::max());
			ImGui::SliderAngle("Rotation", &sprite->spriteData.rotation);
			ImGui::DragFloat2("AnchorPoint", &sprite->spriteData.anchorPoint.x, 0.01f, -1.0f, 1.0f);
			ImGui::Checkbox("FlipX", &sprite->spriteData.isFlipX);
			ImGui::Checkbox("FlipY", &sprite->spriteData.isFlipY);
			ImGui::DragFloat2("TextureLeftTop", &sprite->spriteData.textureLeftTop.x, 1.0f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat2("TextureSize", &sprite->spriteData.textureSize.x, 1.0f, 0.0f, std::numeric_limits<float>::max());
			sprite->textureHandle = textureManager_->EditTexture("Texture", sprite->textureFileName);

			if (ImGui::Button("Reset")) {
				sprite->spriteData = sprite->initialSpriteData;
			}
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}