#include <PoissonDiskSampler.h>
#include <Random.h>
#include <numbers>

std::vector<Vector2> PoissonDiskSampler::Generate(float width, float height, float radius, uint32_t attempts) {
	width_ = width;
	height_ = height;
	radius_ = radius;
	cellSize_ = radius_ / std::sqrt(2.0f);
	gridWidth_ = static_cast<int32_t>(std::ceil(width_ / cellSize_));
	gridHeight_ = static_cast<int32_t>(std::ceil(height_ / cellSize_));
	grid_.assign(gridWidth_ * gridHeight_, -1);
	samples_.clear();

	// 最初の点
	Vector2 first = Random::Generate(Vector2{ 0.0f, 0.0f }, Vector2{ width_, height_ });
	AddSample(first);
	std::vector<uint32_t> active{ 0 };

	while (!active.empty()) {
		// Active Listからランダム選択
		uint32_t activeIndex = Random::Generate<uint32_t>(0, static_cast<uint32_t>(active.size()) - 1);
		uint32_t sampleIndex = active[activeIndex];
		const Vector2 center = samples_[sampleIndex];
		bool found = false;
		for (size_t i = 0; i < attempts; ++i) {
			Vector2 candidate = GenerateCandidate(center);
			if (IsValid(candidate)) {
				AddSample(candidate);
				active.emplace_back(static_cast<uint32_t>(samples_.size()) - 1);
				found = true;
				break;
			}
		}

		if (!found) {
			active[activeIndex] = active.back();
			active.pop_back();
		}
	}

	return samples_;
}

Vector2 PoissonDiskSampler::GenerateCandidate(const Vector2 &center) {
	float angle = Random::Generate(0.0f, 2.0f * std::numbers::pi_v<float>);
	float distance = Random::Generate(radius_, 2.0f * radius_);

	return {
		.x = center.x + std::cos(angle) * distance,
		.y = center.y + std::sin(angle) * distance
	};
}

bool PoissonDiskSampler::IsValid(const Vector2 &candidate) {
	// 範囲外
	if (candidate.x < 0.0f || candidate.x >= width_ ||
		candidate.y < 0.0f || candidate.y >= height_) {
		return false;
	}

	int32_t cellX = static_cast<int32_t>(candidate.x / cellSize_);
	int32_t cellY = static_cast<int32_t>(candidate.y / cellSize_);

	// 周辺セルだけ確認
	for (int32_t y = cellY - 2; y <= cellY + 2; ++y) {
		for (int32_t x = cellX - 2; x <= cellX + 2; ++x) {
			if (x < 0 || x >= gridWidth_ ||
				y < 0 || y >= gridHeight_) {
				continue;
			}

			int32_t index = grid_[y * gridWidth_ + x];
			if (index == -1) {
				continue;
			}

			const Vector2 &p = samples_[index];
			float dx = candidate.x - p.x;
			float dy = candidate.y - p.y;
			float distanceSquared = dx * dx + dy * dy;
			if (distanceSquared < radius_ * radius_) {
				return false;
			}
		}
	}

	return true;
}

void PoissonDiskSampler::AddSample(const Vector2 &point) {
	int32_t index = static_cast<int32_t>(samples_.size());
	samples_.emplace_back(point);

	int32_t cellX = static_cast<int32_t>(point.x / cellSize_);
	int32_t cellY = static_cast<int32_t>(point.y / cellSize_);
	grid_[cellY * gridWidth_ + cellX] = index;
}