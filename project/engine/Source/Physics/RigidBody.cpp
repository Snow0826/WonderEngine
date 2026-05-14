#define NOMINMAX
#include "RigidBody.h"
#include "EntityComponentSystem.h"
#include "Transform.h"
#include "Collision.h"

void PhysicalSystem::Update(float deltaTime) {
	TransformSystem transformSystem{ registry_ };
	registry_->ForEach<RigidBody, Transform>([&](uint32_t entity, RigidBody *rigidBody, Transform *transform) {
		// 力から加速度を計算
		Vector3 acceleration = rigidBody->force / rigidBody->mass;
		// 速度の更新
		rigidBody->velocity += acceleration * deltaTime;
		// 位置の更新
		transform->translate += rigidBody->velocity * deltaTime;
		transformSystem.MarkDirty(entity);

		Vector3 r = Vector3{ 0.0f, rigidBody->radius, 0.0f };	// 力の作用点ベクトル
		Vector3 torque = r.cross(rigidBody->force);				// トルクを計算
		if (!torque.isZero()) {
			Matrix3x3 inertiaTensor = MakeSphereInertiaTensor(rigidBody->mass, rigidBody->radius);	// 慣性テンソルを計算
			rigidBody->angularMomentum += torque * deltaTime;										// 角運動量を更新
			if (inertiaTensor.inverse(inertiaTensor)) {
				rigidBody->angularVelocity = inertiaTensor * rigidBody->angularMomentum;	// 角速度を計算
			}
		}

		// 力をリセット
		rigidBody->force = Vector3{ 0.0f, 0.0f, 0.0f };
		}, exclude<Disabled>());
}

void PhysicalSystem::Reflect(float deltaTime, uint32_t entityA, uint32_t entityB) {
	Collision::Sphere *sphere = registry_->GetComponent<Collision::Sphere>(entityA);
	Collision::Plane *plane = registry_->GetComponent<Collision::Plane>(entityB);
	RigidBody *rigidBody = registry_->GetComponent<RigidBody>(entityA);
	Transform *transform = registry_->GetComponent<Transform>(entityA);
	if (sphere && plane && rigidBody && transform) {
		if (IsCollision(*sphere, *plane)) {
			// 貫通深度の計算
			float penetration = PenetrationDepth(*sphere, *plane);

			// 貫通していなければ何もしない
			if (penetration <= 0.0f) {
				return;
			}

			// 貫通を修正
			transform->translate += plane->normal * penetration;

			// 反射ベクトルの計算
			Vector3 closestPoint = ClosestPoint(sphere->center, *plane);
			Vector3 r = closestPoint - sphere->center;
			Vector3 velocity = rigidBody->velocity + rigidBody->angularVelocity.cross(r);
			float reflected = velocity.dot(plane->normal);

			// 速度と法線が同じ向きなら衝突応答しない
			if (reflected > 0.0f) {
				return;
			}

			constexpr float restitution = 0.5f; // 反発係数（0から1の範囲）
			rigidBody->velocity = velocity - (1.0f + restitution) * reflected * plane->normal;
		}
	}
}

void PhysicalSystem::ReflectImpulse(float deltaTime, uint32_t entityA, uint32_t entityB) {
	Collision::Sphere *sphere = registry_->GetComponent<Collision::Sphere>(entityA);
	Collision::Plane *plane = registry_->GetComponent<Collision::Plane>(entityB);
	RigidBody *rigidBody = registry_->GetComponent<RigidBody>(entityA);
	Transform *transform = registry_->GetComponent<Transform>(entityA);
	if (sphere && plane && rigidBody && transform) {
		if (IsCollision(*sphere, *plane)) {
			// 貫通深度の計算
			float penetration = PenetrationDepth(*sphere, *plane);

			// 貫通していなければ何もしない
			if (penetration <= 0.0f) {
				return;
			}

			// 貫通を修正
			transform->translate += plane->normal * penetration;

			// 反射ベクトルの計算
			Vector3 closestPoint = ClosestPoint(sphere->center, *plane);
			Vector3 r = closestPoint - sphere->center;
			Vector3 velocity = rigidBody->velocity + rigidBody->angularVelocity.cross(r);
			float reflected = velocity.dot(plane->normal);

			// 速度と法線が同じ向きなら衝突応答しない
			if (reflected > 0.0f) {
				return;
			}

			constexpr float restitution = 0.5f; // 反発係数（0から1の範囲）
			Vector3 impulse = -(1.0f + restitution) * reflected * plane->normal * 60.0f;
			rigidBody->force += impulse;
		}
	}
}

Matrix3x3 PhysicalSystem::MakeSphereInertiaTensor(float mass, float radius) {
	Matrix3x3 inertiaTensor = MakeIdentity3x3();			// 単位行列で初期化
	float inertia = (2.0f / 5.0f) * mass * radius * radius;	// 球の慣性テンソル
	inertiaTensor *= inertia;								// 対角成分に慣性テンソルを設定
	return inertiaTensor;
}

void RigidBodyInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("RigidBody")) {
		RigidBody *rigidBody = registry_->GetComponent<RigidBody>(entity);
		if (rigidBody) {
			ImGui::DragFloat3("Force", &rigidBody->force.x, 0.1f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("Velocity", &rigidBody->velocity.x, 0.1f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("AngularMomentum", &rigidBody->angularMomentum.x, 0.1f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("AngularVelocity", &rigidBody->angularVelocity.x, 0.1f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat("Mass", &rigidBody->mass, 0.1f, 0.01f, std::numeric_limits<float>::max());
			ImGui::DragFloat("Radius", &rigidBody->radius, 0.1f, 0.01f, std::numeric_limits<float>::max());

			if (ImGui::Button("Reset")) {
				*rigidBody = RigidBody{};
			}
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}