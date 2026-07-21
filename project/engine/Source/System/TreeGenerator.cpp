#define NOMINMAX
#include "TreeGenerator.h"
#include "EntityComponentSystem.h"
#include "World.h"
#include "InstanceAllocator.h"
#include "Primitive.h"
#include "Transform.h"
#include "Material.h"
#include "Random.h"

namespace {
	uint32_t treeCounter = 0;	// 木のカウンター
}

uint32_t TreeGenerator::Generate(const Vector3 &rootPosition, const Vector3 &rootDirection, const Vector3 &crownCenter, const Vector3 &crownRadius, uint32_t leafCount, float minRadius, float gamma, float influenceRadius, float killRadius, float branchLength) {
	GenerateLeaves(crownCenter, crownRadius, leafCount);
	GenerateRootBranch(rootPosition, rootDirection);
	uint32_t noProgressCount = 0;
	size_t previousLeafCount = leaves_.size();
	while (!leaves_.empty()) {
		FindClosestBranch(influenceRadius, killRadius);
		GrowBranches(branchLength);
		RemoveLeaves();
		if (leaves_.size() == previousLeafCount) {
			noProgressCount++;
		} else {
			noProgressCount = 0;
			previousLeafCount = leaves_.size();
		}

		if (noProgressCount >= 60) {
			break;
		}
	}
	CalculateRadius(branches_.front().get(), minRadius, gamma);
	treeCounter++;
	return CreateBranchRecursive(branches_.front().get(), std::numeric_limits<uint32_t>::max(), Quaternion::IdentityQuaternion(), branchLength);
}

void TreeGenerator::Delete(uint32_t entity) {
	std::vector<uint32_t> children;
	if (auto relationship = registry_->GetComponent<Relationship>(entity)) {
		children = relationship->children;
	}
	for (uint32_t child : children) {
		Delete(child);
	}
	registry_->RemoveAllComponents(entity);
}

void TreeGenerator::GenerateLeaves(const Vector3 &crownCenter, const Vector3 &crownRadius, uint32_t leafCount) {
	leaves_.clear();
	for (uint32_t i = 0; i < leafCount; i++) {
		Leaf leaf;
		while (true) {
			Vector3 distance{
				Random::generate(-crownRadius.x, crownRadius.x),
				Random::generate(-crownRadius.y, crownRadius.y),
				Random::generate(-crownRadius.z, crownRadius.z),
			};

			float value =
				(distance.x * distance.x) / (crownRadius.x * crownRadius.x) +
				(distance.y * distance.y) / (crownRadius.y * crownRadius.y) +
				(distance.z * distance.z) / (crownRadius.z * crownRadius.z);

			if (value <= 1.0f) {
				leaf.position = crownCenter + distance;
				break;
			}
		};
		leaves_.emplace_back(leaf);
	}
}

void TreeGenerator::GenerateRootBranch(const Vector3 &rootPosition, const Vector3 &rootDirection) {
	branches_.clear();
	auto root = std::make_unique<Branch>();
	root->position = rootPosition;
	root->direction = rootDirection;
	branches_.emplace_back(std::move(root));
}

void TreeGenerator::FindClosestBranch(float influenceRadius, float killRadius) {
	for (auto &branch : branches_) {
		branch->growDirection = { 0.0f, 0.0f, 0.0f };
		branch->growCount = 0;
	}

	for (auto &leaf : leaves_) {
		Branch *closestBranch = nullptr;
		float minDistance = std::numeric_limits<float>::max();
		for (const auto &branch : branches_) {
			float distance = leaf.position.distanceFrom(branch->position);
			if (distance < minDistance) {
				closestBranch = branch.get();
				minDistance = distance;
			}
		}

		if (!closestBranch) {
			continue;
		}

		if (minDistance < killRadius) {
			leaf.reached = true;
			continue;
		}

		if (minDistance > influenceRadius) {
			continue;
		}

		closestBranch->growDirection += (leaf.position - closestBranch->position).normalized();
		closestBranch->growCount++;
	}
}

void TreeGenerator::GrowBranches(float branchLength) {
	std::vector<std::unique_ptr<Branch>> newBranches;
	for (const auto &branch : branches_) {
		if (branch->growCount == 0) {
			continue;
		}

		auto child = std::make_unique<Branch>();
		child->direction = (branch->direction + branch->growDirection).normalized();
		child->position = branch->position + child->direction * branchLength;
		child->parent = branch.get();
		branch->children.emplace_back(child.get());
		newBranches.emplace_back(std::move(child));
	}
	branches_.insert(branches_.end(), std::make_move_iterator(newBranches.begin()), std::make_move_iterator(newBranches.end()));
}

void TreeGenerator::RemoveLeaves() {
	leaves_.erase(std::remove_if(leaves_.begin(), leaves_.end(), [](const Leaf &leaf) { return leaf.reached; }), leaves_.end());
}

float TreeGenerator::CalculateRadius(Branch *branch, float minRadius, float gamma) {
	if (branch->children.empty()) {
		branch->radius = minRadius;
		return branch->radius;
	}

	float sum = 0.0f;
	for (Branch *child : branch->children) {
		float radius = CalculateRadius(child, minRadius, gamma);
		sum += std::pow(radius, gamma);
	}

	branch->radius = std::pow(sum, 1.0f / gamma);
	return branch->radius;
}

uint32_t TreeGenerator::CreateBranchRecursive(Branch *branch, uint32_t parentEntity, const Quaternion &parentWorldRotation, float branchLength) {
	//-----------------------------------
	// 枝の位置と回転を計算
	//-----------------------------------
	Quaternion worldRotation;
	Quaternion localRotation;
	Vector3 localPosition;
	float bottomRadius;
	if (branch->parent) {
		worldRotation = Quaternion::DirectionToDirection({ 0.0f, 1.0f, 0.0f }, (branch->position - branch->parent->position).normalized());
		localRotation = parentWorldRotation.Inverse() * worldRotation;
		localPosition = { 0.0f, branchLength, 0.0f };
		bottomRadius = branch->parent->radius;
	} else {
		worldRotation = Quaternion::IdentityQuaternion();
		localRotation = worldRotation;
		localPosition = branch->position;
		bottomRadius = branch->radius;
	}

	//-----------------------------------
	// Entity生成
	//-----------------------------------

	uint32_t currentEntity = registry_->GenerateEntity();
	registry_->AddComponent(currentEntity, MeshType::kCylinder);
	registry_->AddComponent(currentEntity, BlendMode::kBlendModeNone);
	registry_->AddComponent(currentEntity, QuaternionTransform{ .rotate = localRotation, .translate = localPosition });
	registry_->AddComponent(currentEntity, Material{ .environmentCoefficient = 0.0f });
	registry_->AddComponent(currentEntity, DirtyTransform{});
	registry_->AddComponent(currentEntity, DirtyMaterial{});
	registry_->AddComponent(currentEntity, DirtyTextureData{});
	registry_->AddComponent(currentEntity, DirtyMeshLOD{});
	registry_->AddComponent(currentEntity, DirtyCullingData{});
	registry_->AddComponent(currentEntity, instanceAllocator_->Allocate(currentEntity));
	registry_->AddComponent(currentEntity, primitiveGenerator_->CreateCylinder("Branch" + std::to_string(treeCounter), 32, branch->radius, bottomRadius, branchLength, true, "Bark001_1K-JPG_Color.jpg"));
	registry_->AddComponent(currentEntity, UseCulling{});

	//-----------------------------------
	// 子生成
	//-----------------------------------

	Relationship relationship;
	relationship.parent = parentEntity;
	for (Branch *child : branch->children) {
		uint32_t childEntity = CreateBranchRecursive(child, currentEntity, worldRotation, branchLength);
		if (childEntity != std::numeric_limits<uint32_t>::max()) {
			relationship.children.emplace_back(childEntity);
		}
	}

	registry_->AddComponent(currentEntity, relationship);
	return currentEntity;
}