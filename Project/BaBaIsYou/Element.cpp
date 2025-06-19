#pragma
#include <Windows.h>
#include <gdiplus.h>
#include <algorithm>
#include <chrono>
#include "GameManager.h"
#include "Element.h"
#include "Tile.h"
#include "Stage.h"

using namespace std;
using namespace Gdiplus;

Image* Element::glow_effect = nullptr;

Element::Element(string _id, wstring _image_path, Color _color, ElementStatus _status)
{
    id = _id;
	image_path = _image_path;
	image = new Image(image_path.c_str());
	color = _color;

    status.push_back(_status);

	if (image->GetLastStatus() != Gdiplus::Ok) {
		OutputDebugString(L"[ERROR] Failed to load image in Generator!\n");
	}
}

Element::~Element()
{
    if (image != nullptr)
	{
		delete image; // 이미지 메모리 해제
		image = nullptr;
	}
}

void Element::Update()
{
    //temp_status = ElementStatus::None;
    //UpdatePosition();
}

void Element::UpdatePosition(float deltaTime)
{
    float targetX = static_cast<float>(current_x);
    float targetY = static_cast<float>(current_y);

    float distanceX = targetX - draw_x;
    float distanceY = targetY - draw_y;

    // Easing factor (EaseOut Cubic)
    float factor = 1.0f - pow(1.0f - deltaTime * speed, 3.0f);

    draw_x += distanceX * factor;
    draw_y += distanceY * factor;

    // 도착 근처면 딱 붙여주기
    if (fabs(draw_x - targetX) < 0.01f) draw_x = targetX;
    if (fabs(draw_y - targetY) < 0.01f) draw_y = targetY;
}

Element::Element(const Element& other)
{
	id = other.id;
    status = other.status;
	current_x = other.current_x;
	current_y = other.current_y;
	image_path = other.image_path;
    image = new Image(image_path.c_str());
    color = other.color;
}

void Element::LoadGlowEffect()
{
	if (glow_effect == nullptr) {
		glow_effect = new Image(L"resources/glow_effect.png");
		if (glow_effect->GetLastStatus() != Gdiplus::Ok) {
			OutputDebugString(L"[ERROR] Failed to load glow effect image!\n");
		}
	}
}

string Element::GetId() const
{
    string lowerId = id;

    transform(lowerId.begin(), lowerId.end(), lowerId.begin(),
        [](unsigned char c) { return tolower(c); });

    return lowerId;
}

//void Element::Render(const RenderContext& ctx)
//{
//    Graphics graphics(ctx.hdc);
//
//    int screenX = ctx.offsetX + static_cast<int>(draw_x * ctx.tileSize);
//    int screenY = ctx.offsetY + static_cast<int>(draw_y * ctx.tileSize);
//
//    if (!image) {
//        OutputDebugString(L"[ERROR] image is nullptr in Element::Render()\n");
//        return;
//    }
//
//    // 기본 색상 추출
//    float r = static_cast<float>(color.GetRed()) / 255.0f;
//    float g = static_cast<float>(color.GetGreen()) / 255.0f;
//    float b = static_cast<float>(color.GetBlue()) / 255.0f;
//    float alpha = 1.0f;  // 기본 투명도
//
//    // 텍스트인지 여부 및 활성 상태 확인
//    bool isText = dynamic_cast<Text*>(this) != nullptr;
//    bool isActive = true;
//
//    if (isText) {
//        Stage* stage = GameManager::getInstance().GetCurrentStage();
//        isActive = static_cast<Text*>(this)->IsActiveInStage(stage);
//
//        if (!isActive) {
//            // 비활성 텍스트: 어둡고 약간 투명하게
//            float dim = 0.7f;
//            float alphaDim = 0.7f;
//
//            r *= dim;
//            g *= dim;
//            b *= dim;
//            alpha = alphaDim;
//        }
//    }
//
//    // Tint 색상 적용용 ColorMatrix
//    ColorMatrix tintMatrix = {
//        r,    0.0f, 0.0f, 0.0f, 0.0f,
//        0.0f, g,    0.0f, 0.0f, 0.0f,
//        0.0f, 0.0f, b,    0.0f, 0.0f,
//        0.0f, 0.0f, 0.0f, alpha, 0.0f,
//        0.0f, 0.0f, 0.0f, 0.0f, 1.0f
//    };
//
//    ImageAttributes attr;
//    attr.SetColorMatrix(&tintMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
//
//    // 이미지 그리기
//    graphics.DrawImage(
//        image,
//        Rect(screenX, screenY, ctx.tileSize, ctx.tileSize),
//        0, 0,
//        image->GetWidth(), image->GetHeight(),
//        UnitPixel,
//        &attr
//    );
//}

void Element::Render(const RenderContext& ctx)
{
    Graphics graphics(ctx.hdc);

    int screenX = ctx.offsetX + static_cast<int>(draw_x * ctx.tileSize);
    int screenY = ctx.offsetY + static_cast<int>(draw_y * ctx.tileSize);

    if (!image) {
        OutputDebugString(L"[ERROR] image is nullptr in Element::Render()\n");
        return;
    }

    // 색상 추출
    float r = static_cast<float>(color.GetRed()) / 255.0f;
    float g = static_cast<float>(color.GetGreen()) / 255.0f;
    float b = static_cast<float>(color.GetBlue()) / 255.0f;
    float alpha = 1.0f;

    // 텍스트 비활성 시 색 어둡게 + 흐릿하게
    bool isText = dynamic_cast<Text*>(this) != nullptr;
    bool isActive = true;

    if (isText) {
        Stage* stage = GameManager::getInstance().GetCurrentStage();
        isActive = static_cast<Text*>(this)->IsActiveInStage(stage);

        if (!isActive) {
            float dim = 0.7f;
            float alphaDim = 0.7f;
            r *= dim;
            g *= dim;
            b *= dim;
            alpha = alphaDim;
        }
    }

    // ⭐ Win 상태이면 Glow 이미지 먼저 출력
    if (HasStatus(ElementStatus::Win) && glow_effect) {
        int glowSize = 64;
        int offset = (glowSize - ctx.tileSize) / 2;

        // 💡 시간 기반으로 alpha 조절
        float time = GameManager::getInstance().GetTime(); // 초 단위 시간
        float glowAlpha = 0.3f + 0.2f * sinf(time * 3.0f); // 0.1 ~ 0.5 범위로 점멸

        ColorMatrix glowMatrix = {
            1, 0, 0, 0, 0,
            0, 1, 0, 0, 0,
            0, 0, 1, 0, 0,
            0, 0, 0, glowAlpha, 0, // 💡 알파에 적용!
            0, 0, 0, 0, 1
        };

        ImageAttributes glowAttr;
        glowAttr.SetColorMatrix(&glowMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);

        graphics.DrawImage(
            glow_effect,
            Rect(screenX - offset, screenY - offset, glowSize, glowSize),
            0, 0,
            glow_effect->GetWidth(), glow_effect->GetHeight(),
            UnitPixel,
            &glowAttr
        );
    }

    // 최종 Tint ColorMatrix
    ColorMatrix tintMatrix = {
        r, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, g, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, b, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, alpha, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };

    ImageAttributes attr;
    attr.SetColorMatrix(&tintMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);

    // 🔷 실제 오브젝트 이미지 출력
    graphics.DrawImage(
        image,
        Rect(screenX, screenY, ctx.tileSize, ctx.tileSize),
        0, 0,
        image->GetWidth(), image->GetHeight(),
        UnitPixel,
        &attr
    );
}


void Element::SetPosition(int x, int y)
{
    vector<vector<Tile*>>& current_tileMap = GameManager::getInstance().GetCurrentStage()->GetTileMap();

    // 현재 타일에서 자신을 제거
    Tile* currentTile = current_tileMap[current_y][current_x];
    currentTile->RemoveElement(this);

    // 좌표 업데이트
    current_x = x;
    current_y = y;

    // 새 타일에 자신을 추가
    Tile* newTile = current_tileMap[current_y][current_x];
    newTile->AddElement(this);
}

void Element::SetTarget(int _x, int _y)
{
    draw_x = static_cast<float>(current_x); // 현재 위치에서 시작
    draw_y = static_cast<float>(current_y);
    target_x = _x;
    target_y = _y;
}

bool Element::HasStatus(ElementStatus _status) const
{
	return find(status.begin(), status.end(), _status) != status.end();
}

void Element::SnapToCurrent()
{
    draw_x = static_cast<float>(current_x);
    draw_y = static_cast<float>(current_y);
}

bool Element::IsPushable() const
{
    const auto& statuses = GetStatus();  // vector<ElementStatus>
    return std::find(statuses.begin(), statuses.end(), ElementStatus::Push) != statuses.end()
        || IsAlwaysPushable();
}

bool Element::CanMove(int dx, int dy) {
    // 현재 타일맵 가져오기
    vector<vector<Tile*>> current_tileMap = GameManager::getInstance().GetCurrentStage()->GetTileMap();

    // 이동하려는 새 좌표 계산
    int newX = current_x + dx;
    int newY = current_y + dy;

    // 맵 경계 체크
    if (newX < 0 || newY < 0 ||
        newY >= current_tileMap.size() ||
        newX >= current_tileMap[newY].size()) {
        return false;
    }

    // 대상 타일 가져오기
    Tile* targetTile = current_tileMap[newY][newX];

    // 타일이 존재하지 않으면 이동 불가
    if (targetTile == nullptr) {
        return false;
    }

    // 타일의 canMove 함수 호출하여 이동 가능 여부 확인
    return targetTile->CanMove(dx, dy);
}

bool Element::TryMove(int dx, int dy)
{
    Tile* tile = GameManager::getInstance().GetCurrentStage()->GetTileAt(current_x, current_y);
    if (!tile) return false;

    return tile->Move(dx, dy);
}


bool Element::Move(int dx, int dy) {
    // 먼저 이동 가능한지 확인
    if (!CanMove(dx, dy))
        return false;

    vector<vector<Tile*>> current_tileMap = GameManager::getInstance().GetCurrentStage()->GetTileMap();
    int newX = current_x + dx;
    int newY = current_y + dy;

    Tile* currentTile = current_tileMap[current_y][current_x];
    Tile* targetTile = current_tileMap[newY][newX];

    if (!currentTile || !targetTile) return false;
    
    // 목표 타일의 Move 함수 호출하여 재귀적으로 이동 처리
    if (!targetTile->Move(dx, dy)) {
        //return false;
    }
    

    // 엘리먼트의 위치 업데이트
    this->SetPosition(newX, newY);

    return true;
}