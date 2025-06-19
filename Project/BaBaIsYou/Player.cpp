#include <sstream>
#include "Player.h"
#include "GameManager.h"

Player& Player::getInstance()
{
	static Player player;
	return player;
}

Player::~Player()
{
	
}

void Player::Update() {

}


void Player::Move(Direction dir) {
	if (isMoving) return;
	if (GameManager::getInstance().GetGameState() == GameState::Clear) return;

	int dx = 0, dy = 0;
	switch (dir) {
	case Direction::Left:  dx = -1; break;
	case Direction::Right: dx = 1; break;
	case Direction::Up:    dy = -1; break;
	case Direction::Down:  dy = 1; break;
	}

	
	// ��� ���� ���� ��Ҹ� �̵���Ŵ
	for (auto& subject : GameManager::getInstance().GetCurrentStage()->GetElements())
	{
		if (!subject->HasStatus(ElementStatus::You)) continue;

		int newX = subject->GetX() + dx;
		int newY = subject->GetY() + dy;

		//if (subject->CanMove(dx, dy))
		subject->Move(dx, dy); // ���� ��ġ
		// element->SetTarget(newX, newY); // �ִϸ��̼ǿ� ���� ��ġ�� ���� �ִٸ�
	}

	GameManager::getInstance().GetCurrentStage()->UpdateRule();

}
