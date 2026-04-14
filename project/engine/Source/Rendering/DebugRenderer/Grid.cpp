#include "Grid.h"
#include "EntityComponentSystem.h"
#include "DebugRenderer.h"

void Grid::Initialize() {
#ifdef DRAW_LINE
	Rendering::Line line;

	// 手前から奧に向かって描画
	for (float xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		// 座標を計算
		line.start = { -kGridHalfWidth + (kGridEvery * xIndex), 0.0f, -kGridHalfWidth };
		line.end = { -kGridHalfWidth + (kGridEvery * xIndex), 0.0f, kGridHalfWidth };

		// 色を設定
		if (xIndex == kSubdivision / 2) {
			line.color = { 1.0f, 0.0f, 0.0f, 1.0f };
		} else if (std::fmodf(xIndex, 10.0f) == 0.0f) {
			line.color = { 0.1f, 0.1f, 0.1f, 1.0f };
		} else {
			line.color = { 0.6f, 0.6f, 0.6f, 1.0f };
		}

		lines_.emplace_back(line);
	}

	// 左から右に向かって描画
	for (float zIndex = 0.0f; zIndex <= kSubdivision; ++zIndex) {
		// 座標を計算
		line.start = { -kGridHalfWidth, 0.0f, -kGridHalfWidth + (kGridEvery * zIndex) };
		line.end = { kGridHalfWidth, 0.0f, -kGridHalfWidth + (kGridEvery * zIndex) };

		// 色を設定
		if (zIndex == kSubdivision / 2) {
			line.color = { 0.0f, 1.0f, 0.0f, 1.0f };
		} else if (std::fmodf(zIndex, 10.0f) == 0.0f) {
			line.color = { 0.1f, 0.1f, 0.1f, 1.0f };
		} else {
			line.color = { 0.6f, 0.6f, 0.6f, 1.0f };
		}

		lines_.emplace_back(line);
	}
#endif // DRAW_LINE
}

void Grid::Update() {
#ifdef DRAW_LINE
	if (visible_) {
		for (const Rendering::Line &line : lines_) {
			debugRenderer_->AddLine(line);
		}
	}
#endif // DRAW_LINE
}

void Grid::Edit() {
#ifdef USE_IMGUI
	ImGui::Checkbox("Visible Grid", &visible_);
#endif // USE_IMGUI
}