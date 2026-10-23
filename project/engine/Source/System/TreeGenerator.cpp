#define NOMINMAX
#include "TreeGenerator.h"
#include "TreeSpeciesDatabase.h"
#include "EntityComponentSystem.h"
#include "World.h"
#include "InstanceAllocator.h"
#include "Primitive.h"
#include "Transform.h"
#include "Material.h"
#include "Interpolation.h"
#include "Random.h"
#include "Logger.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

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
	treeCounter_++;
	timer_.Begin();
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, Name{ .value = "Tree " + std::to_string(treeCounter_) });
	registry_->AddComponent(entity, MeshType::kTree);
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, EulerTransform{});
	registry_->AddComponent(entity, Material{ .environmentCoefficient = 0.0f });
	registry_->AddComponent(entity, DirtyTransform{});
	registry_->AddComponent(entity, DirtyRelationshipTransform{});
	registry_->AddComponent(entity, DirtyMaterial{});
	registry_->AddComponent(entity, DirtyTextureData{});
	registry_->AddComponent(entity, DirtyMeshLOD{});
	registry_->AddComponent(entity, DirtyCullingData{});
	registry_->AddComponent(entity, instanceAllocator_->Allocate(entity));
	registry_->AddComponent(entity, primitiveGenerator_->CreateTree("TreeMesh" + std::to_string(treeCounter_), CreateBranchMesh(), 2, 4, 1.0f, "Bark001_1K-JPG_Color.jpg"));
	registry_->AddComponent(entity, UseCulling{});
	timer_.End();
	Logger::Log(*logStream_, "Create branch time: " + std::to_string(timer_.GetMs()) + " ms\n");
	return entity;
}

void TreeGenerator::Delete(uint32_t entity) {
	if (auto relationship = registry_->GetComponent<Relationship>(entity)) {
		for (uint32_t child : relationship->children) {
			Delete(child);
		}
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

BranchMesh TreeGenerator::CreateBranchMesh() {
	BranchMesh result;
	if (branches_.empty()) {
		return result;
	}
	Branch *root = branches_.front().get();
	TraceBranchPath(root, nullptr, result);
	return result;
}

void TreeGenerator::TraceBranchPath(Branch *start, Branch *firstChild, BranchMesh &result) {
	BranchPath path;
	path.points.emplace_back(MakeBranchPathPoint(start));
	Branch *current = start;
	if (firstChild) {
		path.points.emplace_back(MakeBranchPathPoint(firstChild));
		current = firstChild;
	}

	// 子が1つしかない場合は、分岐点まで経路を伸ばす
	while (current->children.size() == 1) {
		current = current->children.front();
		path.points.emplace_back(MakeBranchPathPoint(current));
	}

	if (path.points.size() >= 2) {
		// 累積距離を計算する
		path.points[0].distance = 0.0f;
		for (size_t i = 0; i + 1 < path.points.size(); ++i) {
			const auto &p0 = path.points[i];
			const auto &p1 = path.points[i + 1];
			const float segmentLength = (p1.position - p0.position).length();
			const Vector3 tangent0 = p0.direction.normalized() * segmentLength;
			const Vector3 tangent1 = p1.direction.normalized() * segmentLength;
			path.points[i + 1].distance = p0.distance + ApproximateHermiteLength(p0.position, p1.position, tangent0, tangent1);
		}

		result.paths.emplace_back(std::move(path));
	}

	// 分岐点から先は子ごとに別の経路を作る
	for (Branch *child : current->children) {
		TraceBranchPath(current, child, result);
	}
}

BranchPathPoint TreeGenerator::MakeBranchPathPoint(const Branch *branch) {
	return  BranchPathPoint{ .position = branch->position, .direction = branch->direction, .radius = branch->radius, .distance = 0.0f };
}

float TreeGenerator::ApproximateHermiteLength(const Vector3 &startPosition, const Vector3 &endPosition, const Vector3 &startTangent, const Vector3 &endTangent, uint32_t samples) {
	if (samples == 0) {
		return (endPosition - startPosition).length();
	}

	Vector3 previous = Interpolation<Vector3>::Hermite(startPosition, endPosition, startTangent, endTangent, 0.0f);
	float length = 0.0f;
	for (uint32_t i = 1; i <= samples; ++i) {
		const float t = static_cast<float>(i) / static_cast<float>(samples);
		const Vector3 current = Interpolation<Vector3>::Hermite(startPosition, endPosition, startTangent, endTangent, t);
		length += (current - previous).length();
		previous = current;
	}
	return length;
}

void TreeConfigInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("TreeConfig")) {
		if (auto treeConfig = registry_->GetComponent<TreeConfig>(entity)) {
			int32_t treeSpeciesIndex = static_cast<int32_t>(treeConfig->species);
			if (ImGui::Combo("TreeSpecies", &treeSpeciesIndex, "Zelkova\0Kunugi\0Konara\0Beech\0Cedar\0Pine\0Willow\0")) {
				treeConfig->species = static_cast<TreeSpecies>(treeSpeciesIndex);
				MakeTreeConfig(*treeConfig);
			}
			ImGui::DragFloat3("RootPosition", &treeConfig->rootPosition.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("RootDirection", &treeConfig->rootDirection.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			if (ImGui::TreeNode("Crown")) {
				ImGui::DragFloat3("Center", &treeConfig->crown.center.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
				ImGui::DragFloat3("Radius", &treeConfig->crown.radius.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
				int32_t shapeIndex = static_cast<int32_t>(treeConfig->crown.shape);
				if (ImGui::Combo("Shape", &shapeIndex, "Sphere\0Ellipsoid\0Dome\0Cone\0RoundedCone\0Cylinder\0Vase\0")) {
					treeConfig->crown.shape = static_cast<CrownShape>(shapeIndex);
				}
				ImGui::DragFloat("SurfaceBias", &treeConfig->crown.surfaceBias, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("TopBias", &treeConfig->crown.topBias, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("UpwardBias", &treeConfig->crown.upwardBias, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("OutwardBias", &treeConfig->crown.outwardBias, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("DownwardBias", &treeConfig->crown.downwardBias, 0.01f, 0.0f, 1.0f);
				ImGui::TreePop();
			}
			constexpr uint32_t minValue = 0;
			constexpr uint32_t maxValue = std::numeric_limits<uint32_t>::max();
			ImGui::DragScalar("LeafCount", ImGuiDataType_U32, &treeConfig->leafCount, 1, &minValue, &maxValue);
			ImGui::DragFloat("MinRadius", &treeConfig->minRadius, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat("BranchRadius", &treeConfig->branchRadius, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat("TipRadius", &treeConfig->tipRadius, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::Checkbox("UseRadiusTaper", &treeConfig->useRadiusTaper);
			ImGui::DragFloat("Gamma", &treeConfig->gamma, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat("InfluenceRadius", &treeConfig->influenceRadius, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat("KillRadius", &treeConfig->killRadius, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat("BranchLength", &treeConfig->branchLength, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat("TrunkHeight", &treeConfig->trunkHeight, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragScalar("RandomSeed", ImGuiDataType_U32, &treeConfig->randomSeed, 1, &minValue, &maxValue);
			ImGui::DragScalar("MaxIterations", ImGuiDataType_U32, &treeConfig->maxIterations, 1, &minValue, &maxValue);
			ImGui::DragScalar("MaxBranches", ImGuiDataType_U32, &treeConfig->maxBranches, 1, &minValue, &maxValue);
			treeConfig->rootDirection = treeConfig->rootDirection.normalized();

			if (ImGui::Button("Generate")) {
				uint32_t treeEntity = treeGenerator_->Generate(*treeConfig);
				treeEntities_.emplace_back(treeEntity);
			}

			if (ImGui::Button("Delete") && !treeEntities_.empty()) {
				treeGenerator_->Delete(treeEntities_.back());
				treeEntities_.pop_back();
			}
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}