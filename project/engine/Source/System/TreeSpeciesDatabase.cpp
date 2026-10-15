#include "TreeSpeciesDatabase.h"

TreeConfig MakeTreeConfig(TreeSpecies species, const Vector3 &rootPosition) {
	TreeConfig config;
	config.rootPosition = rootPosition;
	config.crown.center = rootPosition;
	config.leafCount = 5000;
	config.randomSeed = 0;
	config.maxIterations = 180;
	config.maxBranches = 30000;
	config.branchRadius = 0.18f;
	config.tipRadius = 0.018f;
	config.useRadiusTaper = false;

	switch (species) {
	case TreeSpecies::Zelkova:
		config.trunkHeight = 3.0f;
		config.crown.radius = { 8.5f, 7.0f, 7.5f };
		config.crown.shape = CrownShape::Ellipsoid;
		config.crown.surfaceBias = 0.55f;
		config.crown.topBias = 0.20f;
		config.crown.upwardBias = 0.36f;
		config.crown.outwardBias = 0.70f;
		config.influenceRadius = 2.55f;
		config.killRadius = 0.77f;
		config.branchLength = 0.65f;
		config.minRadius = 0.018f;
		config.gamma = 3.0f;
		break;
	case TreeSpecies::Kunugi:
		config.trunkHeight = 2.6f;
		config.crown.radius = { 9.0f, 5.8f, 8.5f };
		config.crown.shape = CrownShape::Dome;
		config.crown.surfaceBias = 0.62f;
		config.crown.topBias = 0.12f;
		config.crown.upwardBias = 0.22f;
		config.crown.outwardBias = 0.75f;
		config.influenceRadius = 3.06f;
		config.killRadius = 0.92f;
		config.branchLength = 0.61f;
		config.minRadius = 0.018f;
		config.gamma = 2.6f;
		break;
	case TreeSpecies::Konara:
		config.trunkHeight = 2.8f;
		config.crown.radius = { 7.5f, 6.5f, 7.0f };
		config.crown.shape = CrownShape::Dome;
		config.crown.surfaceBias = 0.58f;
		config.crown.topBias = 0.18f;
		config.crown.upwardBias = 0.30f;
		config.crown.outwardBias = 0.68f;
		config.influenceRadius = 2.38f;
		config.killRadius = 0.71f;
		config.branchLength = 0.68f;
		config.minRadius = 0.018f;
		config.gamma = 2.8f;
		break;
	case TreeSpecies::Beech:
		config.trunkHeight = 3.2f;
		config.crown.radius = { 8.0f, 8.0f, 7.0f };
		config.crown.shape = CrownShape::Ellipsoid;
		config.crown.surfaceBias = 0.50f;
		config.crown.topBias = 0.24f;
		config.crown.upwardBias = 0.40f;
		config.crown.outwardBias = 0.55f;
		config.influenceRadius = 2.38f;
		config.killRadius = 0.71f;
		config.branchLength = 0.74f;
		config.minRadius = 0.018f;
		config.gamma = 3.2f;
		break;
	case TreeSpecies::Cedar:
		config.trunkHeight = 5.0f;
		config.crown.radius = { 5.5f, 12.0f, 5.5f };
		config.crown.shape = CrownShape::Cone;
		config.crown.surfaceBias = 0.40f;
		config.crown.topBias = 0.32f;
		config.crown.upwardBias = 0.58f;
		config.crown.outwardBias = 0.42f;
		config.influenceRadius = 1.87f;
		config.killRadius = 0.56f;
		config.branchLength = 0.58f;
		config.minRadius = 0.018f;
		config.gamma = 5.0f;
		break;
	case TreeSpecies::Pine:
		config.trunkHeight = 4.0f;
		config.crown.radius = { 8.0f, 9.0f, 7.0f };
		config.crown.shape = CrownShape::Cylinder;
		config.crown.surfaceBias = 0.50f;
		config.crown.topBias = 0.27f;
		config.crown.upwardBias = 0.40f;
		config.crown.outwardBias = 0.58f;
		config.influenceRadius = 2.38f;
		config.killRadius = 0.71f;
		config.branchLength = 0.74f;
		config.minRadius = 0.018f;
		config.gamma = 4.0f;
		break;
	case TreeSpecies::Willow:
		config.trunkHeight = 3.0f;
		config.crown.radius = { 9.0f, 7.0f, 8.0f };
		config.crown.shape = CrownShape::Vase;
		config.crown.surfaceBias = 0.52f;
		config.crown.topBias = 0.20f;
		config.crown.upwardBias = 0.26f;
		config.crown.outwardBias = 0.58f;
		config.crown.downwardBias = 0.34f;
		config.influenceRadius = 2.72f;
		config.killRadius = 0.82f;
		config.branchLength = 0.74f;
		config.minRadius = 0.018f;
		config.gamma = 3.0f;
		break;
	}
	config.crown.center.y += config.trunkHeight + config.crown.radius.y * 0.5f;
	return config;
}
