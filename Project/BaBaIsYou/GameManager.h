#pragma once
#include <Windows.h>
#include <gdiplus.h>
#include <unordered_map>
#include <memory>
#include <chrono>
#include "RenderContext.h"
#include "Player.h"
#include "Stage.h"
#include "Material.h"
#include "ClearEffect.h"

using namespace std;
using namespace Gdiplus;

enum class GameState
{
	Title,
	InStage,
	Clear
};

class GameManager
{
private:
	GameManager() {}
	GameManager(const GameManager& ref) {}
	GameManager& operator=(const GameManager& ref) {}
	~GameManager() {}

	HWND hWnd = nullptr;
	ULONG_PTR gdiplusToken = 0;
	const int TILESIZE = 32;
	const int MAX_TILESIZE = 32;
	const int MARGIN_WIDTH = 64;
	const int MARGIN_HEIGHT = 32;
	unique_ptr<SolidBrush> bgBrush;

	float totalTime = 0.0f;
	float clearTimer = 0.0f;
	chrono::steady_clock::time_point prevTime;

	set<Element*> all_elements;

	vector<string> all_stages;
	Stage* current_stage = nullptr;
	GameState current_state;

	Player& player = Player::getInstance();

	ClearEffect clearEffect;

public:
	static GameManager& getInstance();
	SolidBrush* GetBackgroundBrush() { return bgBrush.get(); }
	void SetWindowHandle(HWND _hWnd);
	void Init();
	void LoadStage();
	void LoadElements();
	void ClearElements();
	void Run();
	void Shutdown();
	void Exit();
	void Update(float deltaTime = 0.08f);
	void SetState(GameState state);
	void Render(HDC hdc, int m, int n);
	void ClearRender(Graphics& graphics, int screenWidth, int screenHeight);
	void HandleInput(WPARAM wParam);
	Element* CopyFromCatalog(MaterialType type);
	Element* CopyFromCatalog(const string& id);
	inline Stage* GetCurrentStage() { return current_stage; }
	inline float GetTime() const { return totalTime; }
	inline GameState GetGameState() const { return current_state; }
	inline ClearEffect GetClearEffect() { return clearEffect; }
	void ChangeStage(const string& stageId);
	void TriggerClearEffect();
};