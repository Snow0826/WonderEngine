#include "ParticleObject.h"
#include "EntityComponentSystem.h"
#include "BlendMode.h"
#include "Particle.h"

void ParticleObject::Create() {
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, MeshType::kPlane);
	registry_->AddComponent(entity, BlendMode::kBlendModeAdditive);
	registry_->AddComponent(entity, particleManager_->FindParticleGroup("Default"));
	registry_->AddComponent(entity, Relationship{});

	EmitterSphere emitterSphere{
		.translate = { 0.0f, 0.0f, 0.0f },
		.radius = 1.0f,
		.count = 10,
		.frequency = 0.5f,
		.frequencyTime = 0.0f,
		.emit = 0
	};

	entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, emitterSphere);
	registry_->AddComponent(entity, Relationship{});
}