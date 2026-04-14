#include "Object.h"
#include "EntityComponentSystem.h"

Object ObjectManager::CreateObject(uint32_t entity) {
	Object object{ static_cast<uint32_t>(entities_.size()) };
	entities_.emplace_back(entity);
	return object;
}

void ObjectManager::RemoveObject(uint32_t entity) {
	Object *object = registry_->GetComponent<Object>(entity);
	Object *lastObject = registry_->GetComponent<Object>(entities_.back());
	if (!object || !lastObject) {
		return;
	}
	// 最後尾のオブジェクトと入れ替える
	if (entity != entities_.back()) {
		entities_[object->handle] = entities_.back();
		lastObject->handle = object->handle;
	}
	entities_.pop_back();
}

void ObjectManager::Debug() const {
#ifdef USE_IMGUI
	ImGui::Text("Object Count: %zu", entities_.size());
#endif // USE_IMGUI
}