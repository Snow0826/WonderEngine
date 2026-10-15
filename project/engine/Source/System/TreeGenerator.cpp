#define NOMINMAX
#include "TreeGenerator.h"
#include "EntityComponentSystem.h"
#include "World.h"
#include "InstanceAllocator.h"
#include "Primitive.h"
#include "Transform.h"
#include "Material.h"
#include "Random.h"
#include "Logger.h"

namespace {
	uint32_t treeCounter = 0;	// 木のカウンター
}

uint32_t TreeGenerator::Generate(const TreeConfig &tree) {
	currentCrown_ = tree.crown;
	if (tree.randomSeed == 0) {
		randomEngine_.seed(std::random_device{}());
	} else {
		randomEngine_.seed(tree.randomSeed);
	}
	leaves_.clear();
	branches_.clear();
	branchGrid_.clear();
	gridCellSize_ = tree.influenceRadius;
	GenerateLeaves(tree.crown.center, tree.crown.radius, tree.leafCount);
	GenerateRootBranch(tree.rootPosition, tree.rootDirection);
	GenerateTrunk(tree.trunkHeight, tree.branchLength, tree.rootDirection);
	uint32_t noProgressCount = 0;
	size_t previousLeafCount = leaves_.size();
	uint32_t iteration = 0;
	timer_.Begin();
	while (!leaves_.empty() && (tree.maxIterations == 0 || iteration < tree.maxIterations) && (tree.maxBranches == 0 || branches_.size() < tree.maxBranches)) {
		++iteration;
		FindClosestBranch(tree.influenceRadius, tree.killRadius);
		GrowBranches(tree.branchLength, tree.maxBranches);
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
	timer_.End();
	Logger::Log(*logStream_, "Branch generation time: " + std::to_string(timer_.GetMs()) + " ms\n");
	Logger::Log(*logStream_, "Number of branches: " + std::to_string(branches_.size()) + "\n");
	timer_.Begin();
	CalculateRadius(branches_.front().get(), tree);
	timer_.End();
	Logger::Log(*logStream_, "Calculate radius time: " + std::to_string(timer_.GetMs()) + " ms\n");
	Vector3 boundsMin = branches_.front()->position;
	Vector3 boundsMax = branches_.front()->position;
	for (const auto &branch : branches_) {
		// 各枝の円柱は節点から branchLength だけ伸びるため、その長さも保守的に含める。
		const float margin = branch->radius + tree.branchLength + 1.0f;
		boundsMin.x = std::min(boundsMin.x, branch->position.x - margin);
		boundsMin.y = std::min(boundsMin.y, branch->position.y - margin);
		boundsMin.z = std::min(boundsMin.z, branch->position.z - margin);
		boundsMax.x = std::max(boundsMax.x, branch->position.x + margin);
		boundsMax.y = std::max(boundsMax.y, branch->position.y + margin);
		boundsMax.z = std::max(boundsMax.z, branch->position.z + margin);
	}
	uint32_t treeId = treeCounter;
	treeCounter++;
	timer_.Begin();
	Primitive branchPrimitive = primitiveGenerator_->CreateCylinder("Branch" + std::to_string(treeCounter), 32, 1.0f, 1.0f, tree.branchLength, false, "Bark001_1K-JPG_Color.jpg");
	uint32_t treeEntity = CreateBranchRecursive(branches_.front().get(), std::numeric_limits<uint32_t>::max(), Quaternion::IdentityQuaternion(), tree.branchLength, branchPrimitive, treeId);
	registry_->AddComponent(treeEntity, TreeBounds{ .treeId = treeId, .min = boundsMin, .max = boundsMax });
	timer_.End();
	Logger::Log(*logStream_, "Create branch time: " + std::to_string(timer_.GetMs()) + " ms\n");
	return treeEntity;
}

void TreeGenerator::Delete(uint32_t entity) {
	std::vector<uint32_t> children;
	if (auto relationship = registry_->GetComponent<Relationship>(entity)) {
		children = relationship->children;
	}
	for (uint32_t child : children) {
		Delete(child);
	}
	instanceAllocator_->Free(entity);
	registry_->RemoveAllComponents(entity);
	registry_->AddComponent(entity, DirtyMeshLOD{});
	registry_->AddComponent(entity, DirtyCullingData{});
}

void TreeGenerator::GenerateLeaves(const Vector3 &crownCenter, const Vector3 &crownRadius, uint32_t leafCount) {
	timer_.Begin();
	leaves_.clear();
	leaves_.reserve(leafCount);
	for (uint32_t i = 0; i < leafCount; i++) {
		Leaf leaf;
		while (true) {
			Vector3 distance = Random::Generate(randomEngine_, -crownRadius, crownRadius);
			float x = distance.x / crownRadius.x;
			float y = distance.y / crownRadius.y;
			float z = distance.z / crownRadius.z;
			float radial = std::sqrt(x * x + z * z);
			float envelope = 1.0f;
			bool signedEnvelope = false;
			switch (currentCrown_.shape) {
				case CrownShape::Sphere:
				case CrownShape::Ellipsoid:
					envelope = x * x + y * y + z * z;
					break;
				case CrownShape::Dome:
					envelope = radial * radial + std::pow((y - 0.15f) / 0.85f, 2.0f);
					if (y < -0.30f) {
						envelope += 4.0f;
					}
					break;
				case CrownShape::Cone:
					envelope = radial - (0.98f - (y + 0.5f) * 0.92f);
					signedEnvelope = true;
					break;
				case CrownShape::RoundedCone:
					envelope = radial - (0.35f + 0.65f * (0.5f - y));
					signedEnvelope = true;
					break;
				case CrownShape::Cylinder:
					envelope = std::max(radial - 0.95f, std::abs(y) - 0.5f);
					signedEnvelope = true;
					break;
				case CrownShape::Vase:
					envelope = radial - (0.30f + 0.65f * (y + 0.5f));
					signedEnvelope = true;
					break;
			}
			if ((signedEnvelope ? envelope <= 0.0f : envelope <= 1.0f) && Random::Generate(randomEngine_, 0.0f, 1.0f) <= 1.0f - currentCrown_.topBias * (1.0f - (y + 0.5f))) {
				if (Random::Generate(randomEngine_, 0.0f, 1.0f) < currentCrown_.surfaceBias) {
					Vector3 normal{ distance.x / (crownRadius.x * crownRadius.x), distance.y / (crownRadius.y * crownRadius.y), distance.z / (crownRadius.z * crownRadius.z) };
					distance += normal.normalized() * (Random::Generate(randomEngine_, 0.0f, 0.1f) * std::min(crownRadius.x, crownRadius.z));
				}
				leaf.position = crownCenter + distance;
				break;
			}
		};
		leaves_.emplace_back(leaf);
	}
	timer_.End();
	Logger::Log(*logStream_, "Leaf generation time: " + std::to_string(timer_.GetMs()) + " ms\n");
	Logger::Log(*logStream_, "Number of leaves: " + std::to_string(leaves_.size()) + "\n");
}

void TreeGenerator::GenerateRootBranch(const Vector3 &rootPosition, const Vector3 &rootDirection) {
	auto root = std::make_unique<Branch>();
	root->position = rootPosition;
	root->direction = rootDirection;
	AddBranchToGrid(root.get());
	branches_.emplace_back(std::move(root));
}

void TreeGenerator::GenerateTrunk(float trunkHeight, float branchLength, const Vector3 &rootDirection) {
	if (trunkHeight <= 0.0f || branchLength <= 0.0f) {
		return;
	}

	Vector3 direction = rootDirection.isZero() ? Vector3{ 0.0f, 1.0f, 0.0f } : rootDirection.normalized();
	Branch *parent = branches_.front().get();
	uint32_t segmentCount = static_cast<uint32_t>(std::ceil(trunkHeight / branchLength));
	branches_.reserve(branches_.size() + segmentCount);
	for (uint32_t i = 0; i < segmentCount; ++i) {
		auto trunkSegment = std::make_unique<Branch>();
		trunkSegment->position = parent->position + direction * branchLength;
		trunkSegment->direction = direction;
		trunkSegment->parent = parent;
		trunkSegment->generation = parent->generation + 1;
		parent->children.emplace_back(trunkSegment.get());
		AddBranchToGrid(trunkSegment.get());
		parent = trunkSegment.get();
		branches_.emplace_back(std::move(trunkSegment));
	}
}

void TreeGenerator::FindClosestBranch(float influenceRadius, float killRadius) {
	for (auto &branch : branches_) {
		branch->growDirection = { 0.0f, 0.0f, 0.0f };
		branch->growCount = 0;
	}

	const float killRadiusSquared = killRadius * killRadius;
	const float influenceRadiusSquared = influenceRadius * influenceRadius;
	for (auto &leaf : leaves_) {
		Branch *closestBranch = FindClosestBranchInGrid(leaf.position, influenceRadius, killRadius);
		if (!closestBranch) {
			continue;
		}

		Vector3 difference = leaf.position - closestBranch->position;
		float distanceSquared = difference.lengthSquare();

		if (distanceSquared < killRadiusSquared) {
			leaf.reached = true;
			continue;
		}

		if (distanceSquared > influenceRadiusSquared) {
			continue;
		}

		Vector3 radial = closestBranch->position - Vector3{ currentCrown_.center.x, closestBranch->position.y, currentCrown_.center.z };
		Vector3 biasedDirection = difference.normalized() + Vector3{ 0.0f, currentCrown_.upwardBias - currentCrown_.downwardBias, 0.0f } + radial.normalized() * currentCrown_.outwardBias;
		closestBranch->growDirection += biasedDirection.normalized();
		closestBranch->growCount++;
	}
}

Branch *TreeGenerator::FindClosestBranchInGrid(const Vector3 &position, float influenceRadius, float killRadius) {
	Branch *closestBranch = nullptr;
	float minDistanceSquared = std::numeric_limits<float>::max();
	GridCell centerCell = GetGridCell(position);
	float searchRadius = std::max(influenceRadius, killRadius);
	int32_t cellRange = static_cast<int32_t>(std::ceil(searchRadius / gridCellSize_));
	for (int32_t x = -cellRange; x <= cellRange; ++x) {
		for (int32_t y = -cellRange; y <= cellRange; ++y) {
			for (int32_t z = -cellRange; z <= cellRange; ++z) {
				GridCell cell = { centerCell.x + x, centerCell.y + y, centerCell.z + z };
				auto it = branchGrid_.find(cell);
				if (it == branchGrid_.end()) {
					continue;
				}

				for (Branch *branch : it->second) {
					Vector3 difference = position - branch->position;
					float distanceSquared = difference.lengthSquare();
					if (distanceSquared < minDistanceSquared) {
						closestBranch = branch;
						minDistanceSquared = distanceSquared;
					}
				}
			}
		}
	}
	return closestBranch;
}

void TreeGenerator::GrowBranches(float branchLength, uint32_t maxBranches) {
	std::vector<std::unique_ptr<Branch>> newBranches;
	for (const auto &branch : branches_) {
		if (maxBranches != 0 && branches_.size() + newBranches.size() >= maxBranches) {
			break;
		}
		if (branch->growCount == 0) {
			continue;
		}

		auto child = std::make_unique<Branch>();
		child->direction = (branch->direction + branch->growDirection).normalized();
		child->position = branch->position + child->direction * branchLength;
		child->parent = branch.get();
		child->generation = branch->generation + 1;
		branch->children.emplace_back(child.get());
		AddBranchToGrid(child.get());
		newBranches.emplace_back(std::move(child));
	}
	branches_.insert(branches_.end(), std::make_move_iterator(newBranches.begin()), std::make_move_iterator(newBranches.end()));
}

void TreeGenerator::RemoveLeaves() {
	leaves_.erase(std::remove_if(leaves_.begin(), leaves_.end(), [](const Leaf &leaf) { return leaf.reached; }), leaves_.end());
}

float TreeGenerator::CalculateRadius(Branch *branch, const TreeConfig &tree) {
	if (tree.useRadiusTaper) {
		float sum = 0.0f;
		for (Branch *child : branch->children) {
			float radius = CalculateRadius(child, tree);
			sum += std::pow(radius, 4.0f);
		}
		float derivedRadius = branch->children.empty() ? 0.0f : std::pow(sum, 1.0f / std::max(tree.gamma, 0.1f));
		float taperRadius = std::max(tree.tipRadius, tree.branchRadius * std::exp(-static_cast<float>(branch->generation) * 0.045f));
		branch->radius = std::max(taperRadius, derivedRadius);
		return branch->radius;
	}

	if (branch->children.empty()) {
		branch->radius = tree.minRadius;
		return branch->radius;
	}

	float sum = 0.0f;
	for (Branch *child : branch->children) {
		float radius = CalculateRadius(child, tree);
		sum += std::pow(radius, tree.gamma);
	}

	branch->radius = std::pow(sum, 1.0f / tree.gamma);
	return branch->radius;
}

GridCell TreeGenerator::GetGridCell(const Vector3 &position) const {
	return {
		static_cast<int32_t>(std::floor(position.x / gridCellSize_)),
		static_cast<int32_t>(std::floor(position.y / gridCellSize_)),
		static_cast<int32_t>(std::floor(position.z / gridCellSize_))
	};
}

void TreeGenerator::AddBranchToGrid(Branch *branch) {
	GridCell cell = GetGridCell(branch->position);
	branchGrid_[cell].emplace_back(branch);
}

uint32_t TreeGenerator::CreateBranchRecursive(Branch *branch, uint32_t parentEntity, const Quaternion &parentWorldRotation, float branchLength, const Primitive &branchPrimitive, uint32_t treeId) {
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
	registry_->AddComponent(currentEntity, DirtyRelationshipTransform{});
	registry_->AddComponent(currentEntity, DirtyMaterial{});
	registry_->AddComponent(currentEntity, DirtyTextureData{});
	registry_->AddComponent(currentEntity, DirtyMeshLOD{});
	registry_->AddComponent(currentEntity, DirtyCullingData{});
	registry_->AddComponent(currentEntity, instanceAllocator_->Allocate(currentEntity));
	registry_->AddComponent(currentEntity, branchPrimitive);
	registry_->AddComponent(currentEntity, BranchData{ .bottomRadius = bottomRadius, .topRadius = branch->radius, .inverseLength = 1.0f / branchLength });
	registry_->AddComponent(currentEntity, Cylinder{ .radius = std::max(bottomRadius, branch->radius), .height = branchLength });
	registry_->AddComponent(currentEntity, UseCulling{});
	registry_->AddComponent(currentEntity, TreeMember{ .treeId = treeId });

	//-----------------------------------
	// 子生成
	//-----------------------------------

	Relationship relationship;
	relationship.parent = parentEntity;
	for (Branch *child : branch->children) {
		relationship.children.emplace_back(CreateBranchRecursive(child, currentEntity, worldRotation, branchLength, branchPrimitive, treeId));
	}

	/*if (branch->children.empty()) {
		relationship.children.emplace_back(CreateLeaf(currentEntity, treeId));
		relationship.children.emplace_back(CreateLeaf(currentEntity, treeId));
	}*/

	registry_->AddComponent(currentEntity, relationship);
	return currentEntity;
}

uint32_t TreeGenerator::CreateLeaf(uint32_t parentEntity, uint32_t treeId) {
	Relationship relationship;
	relationship.parent = parentEntity;
	Quaternion rotate = Quaternion::MakeRotateAxisAngleQuaternion(Random::Generate(randomEngine_, Vector3{ -1.0f, -1.0f, -1.0f }, Vector3{ 1.0f, 1.0f, 1.0f }).normalized(), Random::Generate(randomEngine_, 0.0f, 360.0f));
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, MeshType::kPlane);
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, QuaternionTransform{ .rotate = rotate });
	registry_->AddComponent(entity, Material{ .environmentCoefficient = 0.0f });
	registry_->AddComponent(entity, DirtyTransform{});
	registry_->AddComponent(entity, DirtyRelationshipTransform{});
	registry_->AddComponent(entity, DirtyMaterial{});
	registry_->AddComponent(entity, DirtyTextureData{});
	registry_->AddComponent(entity, DirtyMeshLOD{});
	registry_->AddComponent(entity, DirtyCullingData{});
	registry_->AddComponent(entity, instanceAllocator_->Allocate(entity));
	registry_->AddComponent(entity, primitiveGenerator_->CreatePlane("Leaf" + std::to_string(treeCounter), "oak.png"));
	registry_->AddComponent(entity, relationship);
	return entity;
}
