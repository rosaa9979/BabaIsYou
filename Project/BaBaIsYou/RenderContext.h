#pragma once
#include <Windows.h>
#include <gdiplus.h>

using namespace Gdiplus;

struct RenderContext {
	HDC hdc;
	Graphics* graphics;
	int tileSize;
	int offsetX;
	int offsetY;

	inline int ToScreenX(int tileX) const {
		return offsetX + tileX * tileSize;
	}

	inline int ToScreenY(int tileY) const {
		return offsetY + tileY * tileSize;
	}
};