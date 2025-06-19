#pragma once
#include <vector>
#include <memory>

using namespace std;

class Tile
{
private:
	int x = 0, y = 0;
	vector<Element*> elements;

public:
	Tile(int _x, int _y) : x(_x), y(_y) {}
	void AddElement(Element* element);
	void RemoveElement(Element* element);
	vector<Element*> GetElements() { return elements; }
	bool CanMove(int dx, int dy);
	bool TryPushAll(int dx, int dy);
	bool Move(int dx, int dy);

};