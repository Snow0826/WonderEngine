#include "InstanceAllocator.h"
#include "EntityComponentSystem.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

InstanceHandle InstanceAllocator::Allocate(uint32_t entity) {
	InstanceHandle instanceHandle{ .value = static_cast<uint32_t>(entities_.size()) };
	entities_.emplace_back(entity);
	return instanceHandle;
}

void InstanceAllocator::Free(uint32_t entity) {
    auto instanceHandle = registry_->GetComponent<InstanceHandle>(entity);
    if (!instanceHandle) {
        return;
    }

    uint32_t removeIndex = instanceHandle->value;
    uint32_t lastIndex = static_cast<uint32_t>(entities_.size() - 1);
    if (removeIndex != lastIndex) {
        uint32_t movedEntity = entities_[lastIndex];
        entities_[removeIndex] = movedEntity;
        if (auto *movedInstanceData = registry_->GetComponent<InstanceHandle>(movedEntity)) {
            movedInstanceData->value = removeIndex;
        }
    }

    entities_.pop_back();
    instanceHandle->value = UINT32_MAX;
}

void InstanceAllocator::Debug() const {
#ifdef USE_IMGUI
    ImGui::Text("Instance Count: %zu", entities_.size());
#endif // USE_IMGUI
}