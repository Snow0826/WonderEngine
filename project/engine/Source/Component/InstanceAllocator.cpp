#include "InstanceAllocator.h"
#include "EntityComponentSystem.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

InstanceData InstanceAllocator::Allocate(uint32_t entity) {
	InstanceData instanceData{ .instanceIndex = static_cast<uint32_t>(entities_.size()) };
	entities_.emplace_back(entity);
	return instanceData;
}

void InstanceAllocator::Free(uint32_t entity) {
    auto instanceData = registry_->GetComponent<InstanceData>(entity);
    if (!instanceData) {
        return;
    }

    uint32_t removeIndex = instanceData->instanceIndex;
    uint32_t lastIndex = static_cast<uint32_t>(entities_.size() - 1);
    if (removeIndex != lastIndex) {
        uint32_t movedEntity = entities_[lastIndex];
        entities_[removeIndex] = movedEntity;
        if (auto *movedInstanceData = registry_->GetComponent<InstanceData>(movedEntity)) {
            movedInstanceData->instanceIndex = removeIndex;
        }
    }

    entities_.pop_back();
    instanceData->instanceIndex = UINT32_MAX;
}

void InstanceAllocator::Debug() const {
#ifdef USE_IMGUI
    ImGui::Text("Instance Count: %zu", entities_.size());
#endif // USE_IMGUI
}