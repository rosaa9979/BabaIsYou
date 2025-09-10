#pragma once
#include <string>
#include <set>
#include <vector>
#include <memory>
#include <Windows.h>
#include <gdiplus.h>
#include "RenderContext.h"

using namespace std;
using namespace Gdiplus;

class Stage;

enum class ElementStatus
{
	None,
	You,
	Win,
	Push,
	Stop
};

class Element
{
protected:
	string id = "None";
	int current_x = 0, current_y = 0;
	float draw_x = 0.0f, draw_y = 0.0f;
	int target_x = 0, target_y = 0;
	float speed = 18.0f;
	wstring image_path = L"";
	Image* image = nullptr;
	Color color = Color(0, 0, 0, 0);
	vector<ElementStatus> status = { ElementStatus::Push };
	ElementStatus temp_status = ElementStatus::Push;
	static Image* glow_effect;

public:
	Element(string _id, wstring _image_path, Color _color, ElementStatus _status = ElementStatus::None);
	Element(const Element& other);
	virtual ~Element();
	virtual void Update();
	virtual void UpdatePosition(float deltaTime = 0.008f);
	virtual void Reset() { status = { ElementStatus::None }; }
	virtual Element* Clone() const = 0;
	static void LoadGlowEffect();
	string GetId() const;
	inline Gdiplus::Image* GetImage() const { return image; }
	inline vector<ElementStatus> GetStatus() const { return status; }
	inline ElementStatus GetTempStatus() const { return temp_status; }
	inline int GetX() const { return current_x; }
	inline int GetY() const { return current_y; }
	virtual bool IsText() const { return false; }
	virtual bool IsMaterial() const { return false; }
	void Render(const RenderContext& ctx);
	void SetPosition(int _x, int _y);
	void SetTarget(int _x, int _y);
	void SetStatus(ElementStatus _status) { status.push_back(_status); }
	void SetTempStatus(ElementStatus _status) { temp_status = _status; }
	bool HasStatus(ElementStatus _status) const;
	void SnapToCurrent();
	virtual bool IsAlwaysPushable() const { return false; }
	virtual bool IsPushable() const;
	bool CanMove(int dx, int dy);
	bool TryMove(int dx, int dy);
	bool Move(int dx, int dy);
};