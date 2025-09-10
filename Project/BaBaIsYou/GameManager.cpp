#include <Windows.h>
#include <gdiplus.h>
#include <math.h>
#include <algorithm>
#include <memory>
#include "GameManager.h"
#include "Text.h"
#include "Material.h"

using namespace std;
using namespace Gdiplus;


GameManager& GameManager::getInstance()
{
	static GameManager gm;
	return gm;
}

void GameManager::SetWindowHandle(HWND _hWnd)
{
	hWnd = _hWnd;
}

void GameManager::Init()
{
    GdiplusStartupInput input;
    Gdiplus::GdiplusStartup(&gdiplusToken, &input, nullptr);
	bgBrush = make_unique<SolidBrush>(Color(50, 50, 50));

    prevTime = std::chrono::steady_clock::now();

    LoadStage();
    LoadElements();

    current_stage = new Stage(all_stages[0]);
}

void GameManager::LoadStage()
{
	all_stages.push_back("Stage0");
}

void GameManager::LoadElements()
{
    Element::LoadGlowEffect();
    ClearElements();

    all_elements.insert(new Material("Border", L"resources/transparent.png", Color(0, 0, 0), MaterialType::None, ElementStatus::Stop));

    all_elements.insert(new Material("BaBa", L"resources/baba.png", Color(255, 255, 255), MaterialType::BaBa));
    all_elements.insert(new Material("Flag", L"resources/flag.png", Color(237, 226, 133), MaterialType::Flag));
    all_elements.insert(new Material("Wall1", L"resources/wall_1.png", Color(41, 49, 65), MaterialType::Wall));
    all_elements.insert(new Material("Wall2", L"resources/wall_2.png", Color(41, 49, 65), MaterialType::Wall));
    all_elements.insert(new Material("Wall3", L"resources/wall_3.png", Color(41, 49, 65), MaterialType::Wall));
	all_elements.insert(new Material("Rock", L"resources/rock.png", Color(194, 158, 70), MaterialType::Rock));

	all_elements.insert(new Text("Text_BABA", L"resources/text_baba.png", Color(217, 57, 106), TextType::Noun));
	all_elements.insert(new Text("Text_FLAG", L"resources/text_flag.png", Color(237, 226, 133), TextType::Noun));
	all_elements.insert(new Text("Text_WALL", L"resources/text_wall.png", Color(155, 155, 155), TextType::Noun));
	all_elements.insert(new Text("Text_ROCK", L"resources/text_rock.png", Color(144, 103, 62), TextType::Noun));

	all_elements.insert(new Text("Text_IS", L"resources/text_is.png", Color(255, 255, 255), TextType::Verb));

	all_elements.insert(new Text("Text_YOU", L"resources/text_you.png", Color(217, 57, 106), TextType::Property, ElementStatus::You));
	all_elements.insert(new Text("Text_STOP", L"resources/text_stop.png", Color(48, 56, 36), TextType::Property, ElementStatus::Stop));
	all_elements.insert(new Text("Text_PUSH", L"resources/text_push.png", Color(144, 103, 62), TextType::Property, ElementStatus::Push));
	all_elements.insert(new Text("Text_WIN", L"resources/text_win.png", Color(237, 226, 133), TextType::Property, ElementStatus::Win));
}

void GameManager::ClearElements()
{
    for (Element* element : all_elements) {
        if (element) {
            delete element;
        }
    }

    all_elements.clear();
}

void GameManager::Run()
{
    current_stage->Init();
    MSG msg;
    bool running = true;

    while (running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) running = false;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        auto now = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(now - prevTime).count();
        prevTime = now;

        totalTime += deltaTime;

        Update();
        InvalidateRect(hWnd, NULL, FALSE);

        Sleep(8);
    }
}

void GameManager::Update(float deltaTime)
{
    if (current_state == GameState::InStage && current_stage)
    {
        current_stage->Update();
    }

    if (current_state == GameState::Clear)
    {
        if (!clearEffect.IsRendering()) {
            clearEffect.Start();
        }

        clearEffect.Update(deltaTime);
        clearTimer += deltaTime;

        if (clearEffect.IsFinished() && current_stage != nullptr) {
            delete current_stage;
            current_stage = nullptr;
        }

        if (clearTimer > 5.0f)
            PostQuitMessage(0);
    }

}

void GameManager::SetState(GameState state)
{
	current_state = state;
}

void GameManager::Shutdown()
{
    bgBrush.reset();

	if (current_stage != nullptr) {
		delete current_stage;
		current_stage = nullptr;
	}
    ClearElements();
    Gdiplus::GdiplusShutdown(gdiplusToken);
}

void GameManager::Exit()
{
    SendMessage(hWnd, WM_CLOSE, 0, 0);
}

void GameManager::Render(HDC hdc, int m, int n) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    int screenWidth = rect.right - rect.left;
    int screenHeight = rect.bottom - rect.top;

    if (current_stage != nullptr)
    {
        m = current_stage->GetCols();
        n = current_stage->GetRows();
    }
    int tileW = (screenWidth - 2 * MARGIN_WIDTH) / m;
    int tileH = (screenHeight - 2 * MARGIN_HEIGHT) / n;
    int tileSize = min(min(tileW, tileH), MAX_TILESIZE);

    int offsetX = (screenWidth - (tileSize * (m + 2))) / 2;
    int offsetY = (screenHeight - (tileSize * (n + 2))) / 2;

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, screenWidth, screenHeight);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
    Graphics g(memDC);
    g.Clear(Color(50, 50, 50)); 

    RenderContext ctx = { memDC, &g, tileSize, offsetX, offsetY };

    if (current_stage)
		current_stage->Render(ctx);
    ClearRender(g, screenWidth, screenHeight);

    BitBlt(hdc, 0, 0, screenWidth, screenHeight, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

void GameManager::ClearRender(Graphics& graphics, int screenWidth, int screenHeight)
{
    if (!clearEffect.IsRendering())
        return;

    int cx = screenWidth / 2;
    int cy = screenHeight / 2;
    float maxRadius = sqrtf(cx * cx + cy * cy);
    float r = maxRadius * (1.0f - clearEffect.radiusProgress);;


    if (!clearEffect.IsFinished())
    {
        GraphicsPath path;
        path.AddEllipse(
            static_cast<REAL>(cx - r),
            static_cast<REAL>(cy - r),
            static_cast<REAL>(r * 2),
            static_cast<REAL>(r * 2)
        );

        Region excludeRegion(&path);
        graphics.SetClip(&excludeRegion, CombineModeExclude);

        SolidBrush brush(Color(50, 50, 50));
        graphics.FillRectangle(&brush, 0, 0, screenWidth, screenHeight);

        graphics.ResetClip();
    }
    else
    {
        SolidBrush brush(Color(50, 50, 50));
        graphics.FillRectangle(&brush, 0, 0, screenWidth, screenHeight);
    }

    if (clearEffect.IsFinished()) {
        Gdiplus::FontFamily fontFamily(L"Malgun Gothic");
        Gdiplus::Font font(&fontFamily, 32, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
        Gdiplus::SolidBrush textBrush(Color(255, 255, 255));
        Gdiplus::StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);

        graphics.DrawString(
            L"Game Clear",
            -1,
            &font,
            RectF(0, 0, static_cast<REAL>(screenWidth), static_cast<REAL>(screenHeight)),
            &format,
            &textBrush
        );
    }
}

void GameManager::HandleInput(WPARAM wParam)
{
	switch (wParam)
	{
	case VK_LEFT:
		player.Move(Direction::Left);
		break;
	case VK_RIGHT:
		player.Move(Direction::Right);
		break;
	case VK_UP:
		player.Move(Direction::Up);
		break;
	case VK_DOWN:
		player.Move(Direction::Down);
		break;
	}
}

Element* GameManager::CopyFromCatalog(MaterialType type)
{
    for (auto& e : all_elements)
    {
        if (e->IsMaterial())
        {
			Material* mat = static_cast<Material*>(e);
            if (mat->GetMaterialType() == type) {
                return mat->Clone(); 
            }
        }

    }

    return nullptr;
}

Element* GameManager::CopyFromCatalog(const std::string& id)
{
    string lowerId = id;
    transform(lowerId.begin(), lowerId.end(), lowerId.begin(),
        [](unsigned char c) { return tolower(c); });

    for (auto& e : all_elements)
    {
        if (e->GetId() == lowerId) {
            return e->Clone();
        }
    }

    return nullptr;
}

void GameManager::ChangeStage(const string& stageId)
{

}

void GameManager::TriggerClearEffect()
{
    clearEffect.Start();
}
