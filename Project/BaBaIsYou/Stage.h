#pragma once
#include <Windows.h>
#include <gdiplus.h>
#include <fstream>
#include <vector>
#include <memory>
#include "RenderContext.h"
#include "Element.h"
#include "Tile.h"
#include "Rule.h"

using namespace std;
using namespace Gdiplus;

class Stage
{
private:
	string stage_name = "";
	wstring info_path = L"";

	int rows;
	int cols;

	vector<vector<Tile*>>tileMap;
	vector<Element*> elements;
	vector<unique_ptr<Rule>> rules;

public:
	Stage(string _stage_name);
	Stage(const Stage&) = delete;
	Stage& operator=(const Stage&) = delete;
	~Stage();
	void Init();
	void Update();
	void UpdateRule();
	void SearchRules();
	void ApplyRules();
	Text* GetTextAt(int x, int y);
	Tile* GetTileAt(int x, int y);
	void Render(const RenderContext& ctx);
	inline vector<vector<Tile*>>& GetTileMap() { return tileMap; }
	inline vector<Element*>& GetElements() { return elements; }
	inline int GetRows() const { return rows; }
	inline int GetCols() const { return cols; }
	inline const vector<unique_ptr<Rule>>& GetRules() const { return rules; }
	void RemoveElement(Element* element);
	void AddElement(Element* element);
	Element* ReplaceElementWithType(Element* oldElem, MaterialType newType);
	bool CheckWinCondition();
};