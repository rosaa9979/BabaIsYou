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
	bgBrush = make_unique<SolidBrush>(Color(50, 50, 50)); // 배경색 설정

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
    all_elements.clear();

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

        Sleep(8); // 120 FPS
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
        // ✅ clearEffect.Start()는 단 한 번만 실행
        if (!clearEffect.IsRendering()) {
            clearEffect.Start();
        }

        clearEffect.Update(deltaTime);
        clearTimer += deltaTime;

        // ✅ 연출이 끝났을 때만 stage 삭제
        if (clearEffect.IsFinished() && current_stage != nullptr) {
            delete current_stage;
            current_stage = nullptr;
        }

        // 5초 후 종료
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

    Gdiplus::GdiplusShutdown(gdiplusToken);
}

void GameManager::Exit()
{
    SendMessage(hWnd, WM_CLOSE, 0, 0);
}

void GameManager::Render(HDC hdc, int m, int n) {
    // 1. 화면 크기 가져오기
    RECT rect;
    GetClientRect(hWnd, &rect);
    int screenWidth = rect.right - rect.left;
    int screenHeight = rect.bottom - rect.top;

    // 2. tileSize 계산
    if (current_stage != nullptr)
    {
        m = current_stage->GetCols();
        n = current_stage->GetRows();
    }
    int tileW = (screenWidth - 2 * MARGIN_WIDTH) / m;
    int tileH = (screenHeight - 2 * MARGIN_HEIGHT) / n;
    int tileSize = min(min(tileW, tileH), MAX_TILESIZE);

    // 3. 중앙 정렬용 오프셋
    int offsetX = (screenWidth - (tileSize * (m + 2))) / 2;
    int offsetY = (screenHeight - (tileSize * (n + 2))) / 2;

    // 4. 더블 버퍼링 준비
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, screenWidth, screenHeight);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
    Graphics g(memDC);
    g.Clear(Color(50, 50, 50)); // 배경 검정

    // 5. 렌더링 컨텍스트 생성
    RenderContext ctx = { memDC, &g, tileSize, offsetX, offsetY };

    // 6. 현재 스테이지 렌더링
    if (current_stage)
		current_stage->Render(ctx);
    ClearRender(g, screenWidth, screenHeight);
    //player.Render(ctx);

    // 7. 화면에 복사
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
        // 중심 타원 제외한 외곽만 덮기
        GraphicsPath path;
        path.AddEllipse(
            static_cast<REAL>(cx - r),
            static_cast<REAL>(cy - r),
            static_cast<REAL>(r * 2),
            static_cast<REAL>(r * 2)
        );

        Region excludeRegion(&path);
        graphics.SetClip(&excludeRegion, CombineModeExclude);

        SolidBrush brush(Color(50, 50, 50));  // 테두리색
        graphics.FillRectangle(&brush, 0, 0, screenWidth, screenHeight);

        graphics.ResetClip();
    }
    else
    {
        // 덮기 완료 상태
        SolidBrush brush(Color(50, 50, 50));
        graphics.FillRectangle(&brush, 0, 0, screenWidth, screenHeight);
    }

    // 텍스트 출력 (완전히 덮인 후)
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
                return mat->Clone(); // 복제된 객체 반환
            }
        }

    }

    return nullptr; // 못 찾았을 경우
}

Element* GameManager::CopyFromCatalog(const std::string& id)
{
    // 소문자 버전 만들기
    string lowerId = id;
    transform(lowerId.begin(), lowerId.end(), lowerId.begin(),
        [](unsigned char c) { return tolower(c); });

    for (auto& e : all_elements)
    {
        if (e->GetId() == lowerId) {
            return e->Clone(); // 복제된 객체 반환
        }
    }

    return nullptr; // 못 찾았을 경우
}

void GameManager::ChangeStage(const string& stageId)
{

}

void GameManager::TriggerClearEffect()
{
    clearEffect.Start();
}
