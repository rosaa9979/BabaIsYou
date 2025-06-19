#include <vector>
#include <algorithm>
#include "GameManager.h"
#include "Element.h"
#include "Tile.h"

using namespace std;

void Tile::AddElement(Element* element)
{
	elements.push_back(element);
}

void Tile::RemoveElement(Element* element)
{
    for (auto iter = elements.begin(); iter != elements.end(); ++iter)
    {
        if (*iter == element)
        {
            elements.erase(iter);
            return;
        }
    }
}

bool Tile::CanMove(int dx, int dy) {
    if (elements.empty())
        return true;

    bool hasPush = false;

    for (auto& element : elements) {
        const auto& statuses = element->GetStatus();  // vector<ElementStatus>

        // Stop이 하나라도 있으면 이동 불가
        if (std::find(statuses.begin(), statuses.end(), ElementStatus::Stop) != statuses.end()) {
            return false;
        }

        // Push가 하나라도 있으면 hasPush 플래그 설정
        if (std::find(statuses.begin(), statuses.end(), ElementStatus::Push) != statuses.end()) {
            hasPush = true;
        }
    }

    const auto& tileMap = GameManager::getInstance().GetCurrentStage()->GetTileMap();
    int newX = x + dx;
    int newY = y + dy;

    // 경계 체크
    if (newX < 0 || newY < 0 || newY >= tileMap.size() || newX >= tileMap[newY].size())
        return false;

    Tile* nextTile = tileMap[newY][newX];
    if (!nextTile)
        return false;

    // Push 요소가 있으면 다음 타일로 재귀 검사
    if (hasPush)
        return nextTile->CanMove(dx, dy);

    return true;
}


bool Tile::Move(int dx, int dy)
{
    vector<vector<Tile*>>& current_tileMap = GameManager::getInstance().GetCurrentStage()->GetTileMap();

    int newX = x + dx;
    int newY = y + dy;

	int prevX = x - dx;
	int prevY = y - dy;

    Tile* nextTile = current_tileMap[newY][newX];
	Tile* prevTile = current_tileMap[prevY][prevX];
    if (!nextTile) return false;

    bool hasStop = false;
    bool hasPush = false;

    /*
    for (Element* e : elements)
    {
        if (e->IsText())
        {
            hasText = true;
            break;
        }

    }
    

    if (hasText) {
        for (Element* e : elements)
            e->SetTempStatus(ElementStatus::Push);
    }
    */

    // 현재 타일의 element들 검사
    for (Element* ele : elements) {
        const auto& statuses = ele->GetStatus();

        if (std::find(statuses.begin(), statuses.end(), ElementStatus::Stop) != statuses.end()) {
            hasStop = true;
            break;
        }

        if (std::find(statuses.begin(), statuses.end(), ElementStatus::Push) != statuses.end()) {
            hasPush = true;
        }
    }
    /*
	if (hasText) {
        for (Element* e : elements)
            e->SetTempStatus(ElementStatus::Push);
	}
    */

    if (hasStop) {
        return false;
    }

    // PUSH 요소가 있으면 다음 타일로 밀어야 하므로 재귀 이동
    if (hasPush) {
        //if (!nextTile->Move(dx, dy, hasText)) {
        if (!nextTile->Move(dx, dy)) {
            return false;
        }
    }

    // PUSH 요소를 다음 타일로 실제로 이동
    vector<Element*> toMove;
    for (Element* ele : elements) {
        const auto& statuses = ele->GetStatus();
        if (std::find(statuses.begin(), statuses.end(), ElementStatus::Push) != statuses.end()) {
            toMove.push_back(ele);
        }
    }

    for (Element* ele : toMove) {
        ele->SetPosition(nextTile->x, nextTile->y);
        nextTile->AddElement(ele);
        this->RemoveElement(ele);
    }

    return true;
}

bool Tile::TryPushAll(int dx, int dy)
{
    auto& tileMap = GameManager::getInstance().GetCurrentStage()->GetTileMap();
    int newX = x + dx;
    int newY = y + dy;

    if (newY < 0 || newY >= tileMap.size() || newX < 0 || newX >= tileMap[newY].size())
        return false;

    Tile* nextTile = tileMap[newY][newX];
    if (!nextTile) return false;

    // 이 타일의 모든 밀릴 수 있는 애들을 재귀적으로 밀기
    for (Element* ele : elements) {
        if (!ele->IsPushable()) return false;
        if (!ele->TryMove(dx, dy)) return false;
    }

    return true;
}