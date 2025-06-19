#pragma once
#include <memory>
#include <Windows.h>
#include <gdiplus.h>
#include <vector>
#include "Element.h"
#include "RenderContext.h"

using namespace Gdiplus;

enum class Direction
{
	Left,
	Right,
	Up,
	Down
};

class Player
{
private:
	Player() {}
	Player(const Player& ref) {}
	~Player();
	Player& operator=(const Player& ref) {}

	int x = 0, y = 0;
	float pos_x, pos_y;
	float speed = 0.2f;
	bool isMoving = false;
	int target_x, target_y;
	
public:
	static Player& getInstance();
	void Update();
	void Move(Direction direction);
};