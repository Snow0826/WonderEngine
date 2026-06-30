#include "TreeGenerator.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "Cylinder.h"
#include "Object.h"
#include "Transform.h"
#include "Material.h"
#include "Random.h"

uint32_t TreeGenerator::Generate(float leafRadius, uint32_t leafCount, float influenceRadius, float killRadius, float branchLength) {
	GenerateLeaves(leafRadius, leafCount);
	GenerateRootBranch(influenceRadius, branchLength);
	while (!leaves_.empty()) {
		FindClosestBranch(influenceRadius, killRadius);
		size_t oldBranchCount = branches_.size();
		GrowBranches(branchLength);
		RemoveLeaves();
		if (branches_.size() == oldBranchCount) {
			break;
		}
	}
	return registry_->GenerateEntity();
}

void TreeGenerator::Delete(uint32_t entity) {
	if (auto relationship = registry_->GetComponent<Relationship>(entity)) {
		for (uint32_t child : relationship->children) {
			Delete(child);
		}
	}
	indirectCommandManager_->RemoveIndirectCommand(entity);
	objectManager_->RemoveObject(entity);
	registry_->RemoveAllComponents(entity);
}

void TreeGenerator::GenerateLeaves(float leafRadius, uint32_t leafCount) {
	leaves_.clear();
	for (uint32_t i = 0; i < leafCount; i++) {
		Leaf leaf;
		do {
			leaf.position = Random::generate({ -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f }) * leafRadius;
		} while (leaf.position.length() > leafRadius);
		leaf.position.y += leafRadius * 2.0f;
		leaves_.emplace_back(leaf);
	}
}

void TreeGenerator::GenerateRootBranch(float influenceRadius, float branchLength) {
	branches_.clear();
	std::unique_ptr<Branch> root = std::make_unique<Branch>();
	root->position = { 0.0f, 0.0f, 0.0f };
	root->direction = { 0.0f, 1.0f, 0.0f };
	Branch *current = root.get();
	branches_.emplace_back(std::move(root));
	while (true) {
		bool found = false;
		for (const Leaf &leaf : leaves_) {
			if (leaf.position.distanceFrom(current->position) < influenceRadius) {
				found = true;
				break;
			}
		}

		if (found) {
			break;
		}

		auto child = std::make_unique<Branch>();
		child->position = current->position + current->direction * branchLength;
		child->direction = current->direction;
		child->parent = current;
		current->children.emplace_back(child.get());
		current = child.get();
		branches_.emplace_back(std::move(child));
	}
}

void TreeGenerator::FindClosestBranch(float influenceRadius, float killRadius) {
	uint32_t foundCount = 0;
	for (Leaf &leaf : leaves_) {
		Branch *closestBranch = nullptr;
		float minDistance = influenceRadius;
		for (const auto &branch : branches_) {
			float distance = leaf.position.distanceFrom(branch->position);
			if (distance < killRadius) {
				leaf.reached = true;
				closestBranch = nullptr;
				break;
			}

			if (distance < minDistance) {
				closestBranch = branch.get();
				minDistance = distance;
			}
		}

		if (closestBranch) {
			foundCount++;
			Vector3 newDirection = closestBranch->position.normalized(leaf.position);
			closestBranch->growDirection += newDirection;
			closestBranch->growCount++;
		}
	}
}

void TreeGenerator::GrowBranches(float branchLength) {
	std::vector<std::unique_ptr<Branch>> newBranches;
	for (const auto &branch : branches_) {
		if (branch->growCount == 0) continue;
		Vector3 newDirection = (branch->growDirection / static_cast<float>(branch->growCount)).normalized();
		std::unique_ptr<Branch> child = std::make_unique<Branch>();
		child->position = branch->position + newDirection * branchLength;
		child->direction = newDirection;
		child->parent = branch.get();
		branch->children.emplace_back(child.get());
		newBranches.emplace_back(std::move(child));
		branch->growDirection = { 0.0f, 0.0f, 0.0f };
		branch->growCount = 0;
	}
	branches_.insert(branches_.end(), std::make_move_iterator(newBranches.begin()), std::make_move_iterator(newBranches.end()));
}

void TreeGenerator::RemoveLeaves() {
	leaves_.erase(std::remove_if(leaves_.begin(), leaves_.end(), [](const Leaf &leaf) { return leaf.reached; }), leaves_.end());
}