#include "Object.h"
#include "EntityComponentSystem.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

Object ObjectManager::CreateObject(uint32_t entity) {
	Object object{ static_cast<uint32_t>(entities_.size()) };
	entities_.emplace_back(entity);
	return object;
}

void ObjectManager::RemoveObject(uint32_t entity) {
    Object *object = registry_->GetComponent<Object>(entity);
    if (!object) {
        return;
    }

    uint32_t removeIndex = object->handle;
    uint32_t lastIndex = static_cast<uint32_t>(entities_.size() - 1);
    if (removeIndex != lastIndex) {
        uint32_t movedEntity = entities_[lastIndex];
        entities_[removeIndex] = movedEntity;
        if (Object *movedObject = registry_->GetComponent<Object>(movedEntity)) {
            movedObject->handle = removeIndex;
        }
    }

    entities_.pop_back();
    object->handle = UINT32_MAX;
}

void ObjectManager::Debug() const {
#ifdef USE_IMGUI
	ImGui::Text("Object Count: %zu", entities_.size());
#endif // USE_IMGUI
}