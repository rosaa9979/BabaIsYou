#include <Windows.h>
#include <gdiplus.h>
#include <nlohmann/json.hpp>
#include <memory>
#include <sstream>
#include <fstream>
#include <string>
#include "Stage.h"
#include "GameManager.h"
#include "Element.h"
#include "RenderContext.h"
#include "Tile.h"
#include "BasicRule.h"

using namespace std;
using namespace Gdiplus;
using json = nlohmann::json;

Stage::Stage(string _stage_name)
{
	stage_name = _stage_name;

	if (stage_name.empty())
		OutputDebugString(L"Stage name is empty\n");

	wstring w_stage_name(stage_name.begin(), stage_name.end());
	info_path = L"stages/" + w_stage_name + L".json";

	ifstream file(info_path);
	if (!file.is_open()) {
		OutputDebugString(L"Failed to open stage info file\n");
		return;
	}

	try {
		json stage_data;
		file >> stage_data;

		rows = stage_data["stage_size"]["rows"];
		cols = stage_data["stage_size"]["cols"];
	}
	catch (const std::exception& e) {
		OutputDebugString(L"Failed to parse stage info JSON\n");
		rows = 10;
		cols = 10;
	}

	file.close();

	for (int row = 0; row < rows + 2; row++)
	{
		vector<Tile*> tileMapCol;
		for (int col = 0; col < cols + 2; col++)
		{
			tileMapCol.push_back(new Tile(col, row));
		}

		tileMap.push_back(tileMapCol);
	}
}

Stage::~Stage()
{
	for (Element* element : elements) {
		if (element != nullptr) {
			delete element;
		}
	}
	elements.clear();

	for (int row = 0; row < tileMap.size(); row++) {
		for (int col = 0; col < tileMap[row].size(); col++) {
			if (tileMap[row][col] != nullptr) {
				delete tileMap[row][col];
			}
		}
		tileMap[row].clear();
	}
	tileMap.clear();
}

void Stage::Init()
{
	GameManager::getInstance().SetState(GameState::InStage);

	for (int row = 0; row < rows + 2; row++)
	{
		for (int col = 0; col < cols + 2; col++)
		{
			if (row == 0 || row == rows + 1)
			{
				Element* border = GameManager::getInstance().CopyFromCatalog("Border");
				border->SetPosition(col, row);
				tileMap[row][col]->AddElement(border);
				elements.push_back(border);
			}

			else
			{
				if (col == 0 || col == cols + 1)
				{
					Element* border = GameManager::getInstance().CopyFromCatalog("Border");
					border->SetPosition(col, row);
					tileMap[row][col]->AddElement(border);
					elements.push_back(border);
				}
			}
		}
	}

	ifstream file(info_path);
	if (!file.is_open()) {
		OutputDebugString(L"Failed to open stage info file\n");
		return;
	}

	try {
		json stage_data;
		file >> stage_data;

		if (stage_data.contains("elements") && stage_data["elements"].is_array()) {
			for (const auto& element_data : stage_data["elements"]) {
				string e_name = element_data["name"];

				int e_x = element_data["position"]["x"];
				int e_y = element_data["position"]["y"];

				Element* element = GameManager::getInstance().CopyFromCatalog(e_name);

				if (element != nullptr) {
					element->SetPosition(e_x, e_y);
					tileMap[e_y][e_x]->AddElement(element);
					elements.push_back(element);
				}
			}
		}
	}

	catch (const std::exception& e) {
		OutputDebugString(L"Failed to parse stage info JSON\n");
	}

	file.close();

	for (Element* ele : elements)
		ele->SnapToCurrent();

	UpdateRule();
	
}

void Stage::Update()
{
	for (auto& ele : elements)
	{
		if (auto* e = dynamic_cast<Element*>(ele)) {
			e->Update();
		}
	}
}

void Stage::UpdateRule()
{
	if (GameManager::getInstance().GetGameState() == GameState::Clear) return;

	rules.clear();

	for (Element* ele : elements)
	{
		if (auto* elem = dynamic_cast<Element*>(ele)) {
			elem->Reset();
		}
	}

	SearchRules();
	ApplyRules();

	if (GameManager::getInstance().GetCurrentStage()->CheckWinCondition() && GameManager::getInstance().GetGameState() != GameState::Clear) {
		GameManager::getInstance().SetState(GameState::Clear);
		OutputDebugString(L"Game Clear\n");
	}
}

void Stage::SearchRules()
{
	rules.clear();

	for (Element* e : elements)
	{
		if (!e->IsText()) continue;

		Text* verb = static_cast<Text*>(e);
		if (verb->GetTextType() != TextType::Verb) continue;

		int x = verb->GetX();
		int y = verb->GetY();

		Text* left = GetTextAt(x - 1, y);
		Text* right = GetTextAt(x + 1, y);

		if (left && right &&
			left->GetTextType() == TextType::Noun &&
			(right->GetTextType() == TextType::Noun || right->GetTextType() == TextType::Property))
		{
			RuleType type = (right->GetTextType() == TextType::Property)
				? RuleType::Property : RuleType::Identity;
			rules.push_back(make_unique<BasicRule>(left, verb, right, type));
		}

		Text* up = GetTextAt(x, y - 1);
		Text* down = GetTextAt(x, y + 1);

		if (up && down &&
			up->GetTextType() == TextType::Noun &&
			(down->GetTextType() == TextType::Noun || down->GetTextType() == TextType::Property))
		{
			RuleType type = (down->GetTextType() == TextType::Property)
				? RuleType::Property : RuleType::Identity;
			rules.push_back(make_unique<BasicRule>(up, verb, down, type));
		}
	}
}

void Stage::ApplyRules()
{
	for (const auto& rule : rules)
		rule->Apply(*this);
}

Text* Stage::GetTextAt(int x, int y)
{
	for (Element* e : elements)
	{
		if (e->IsText() && e->GetX() == x && e->GetY() == y) {
			return static_cast<Text*>(e);
		}
	}
	return nullptr;
}

Tile* Stage::GetTileAt(int x, int y)
{
	if (y < 0 || y >= tileMap.size()) return nullptr;
	if (x < 0 || x >= tileMap[y].size()) return nullptr;

	return tileMap[y][x];
}


void Stage::Render(const RenderContext& ctx)
{
    int tileAreaW = ctx.tileSize * (cols + 2);
    int tileAreaH = ctx.tileSize * (rows + 2);

	SolidBrush* bgBrush = GameManager::getInstance().GetBackgroundBrush();
    SolidBrush tileBg(Color(0, 0, 0));
    ctx.graphics->FillRectangle(&tileBg,
        ctx.offsetX, ctx.offsetY,
        tileAreaW, tileAreaH);

    for (int row = 0; row < rows + 2; row++)
    {
		for (int col = 0; col < cols + 2; col++)
		{
            if ((col == 0 || col == cols + 1) || (row == 0 || row == rows + 1))
            {
				ctx.graphics->FillRectangle(bgBrush,
					ctx.ToScreenX(col), ctx.ToScreenY(row),
					ctx.tileSize, ctx.tileSize);
			}

            else
            {
				ctx.graphics->FillRectangle(&tileBg,
					ctx.ToScreenX(col), ctx.ToScreenY(row),
					ctx.tileSize, ctx.tileSize);
            }
		}
    }

	for (auto& ele : elements)
	{
		ele->Render(ctx);
	}

	for (auto& ele : elements)
	{
		if (ele->HasStatus(ElementStatus::You))
		{
			ele->Render(ctx);
		}
	}
}

void Stage::RemoveElement(Element* element)
{
	auto it = std::remove(elements.begin(), elements.end(), element);
	if (it != elements.end()) {
		elements.erase(it, elements.end());
	}
}

void Stage::AddElement(Element* element)
{
	elements.push_back(element);
}

Element* Stage::ReplaceElementWithType(Element* oldElem, MaterialType newType)
{
	int x = oldElem->GetX();
	int y = oldElem->GetY();

	Tile* tile = GetTileAt(x, y);
	if (!tile) return nullptr;

	tile->RemoveElement(oldElem);
	RemoveElement(oldElem);

	Element* newElem = GameManager::getInstance().CopyFromCatalog(newType);
	newElem->SetPosition(x, y);
	newElem->SnapToCurrent();
	tile->AddElement(newElem);
	AddElement(newElem);

	return newElem;
}

bool Stage::CheckWinCondition()
{
	vector<Element*> win_elements;
	vector<Element*> you_elements;

	for (auto& ele : elements)
	{
		if (ele->HasStatus(ElementStatus::Win))
			win_elements.push_back(ele);
		if (ele->HasStatus(ElementStatus::You))
			you_elements.push_back(ele);
	}

	wstring win_str = L"Win Element Size: " + to_wstring(win_elements.size()) + L"\n";
	wstring you_str = L"You Element Size: " + to_wstring(you_elements.size()) + L"\n";
	OutputDebugString(win_str.c_str());
	OutputDebugString(you_str.c_str());

	if (!you_elements.empty() && !win_elements.empty())
	{
		for (Element* you : you_elements)
		{
			for (Element* win : win_elements)
			{
				wstringstream ss;
				ss << L"YOU: (" << you->GetX() << L", " << you->GetY() << L")\n";
				ss << L"WIN: (" << win->GetX() << L", " << win->GetY() << L")\n";
				OutputDebugString(ss.str().c_str());

				if (you->GetX() == win->GetX() && you->GetY() == win->GetY())
				{
					OutputDebugString(L"Game Clear\n");
					return true;
				}
			}
		}
	}

	OutputDebugString(L"Game Not Clear\n");
	return false;
}
