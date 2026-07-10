#include "ParticleObject.h"
#include "EntityComponentSystem.h"
#include "BlendMode.h"
#include "Particle.h"

void ParticleObject::Create() {
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, MeshType::kPlane);
	registry_->AddComponent(entity, BlendMode::kBlendModeAdditive);
	registry_->AddComponent(entity, Relationship{});
	registry_->AddComponent(entity, particleManager_->FindParticleGroup("Default"));
}