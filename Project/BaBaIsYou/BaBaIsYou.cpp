#include <windows.h>
#include <gdiplus.h>
#include "GameManager.h"

#pragma comment(lib, "gdiplus.lib")

// ������ ���ν��� �Լ� ����
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// WinMain: WinAPI ������
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // ������ Ŭ���� ����
    const wchar_t CLASS_NAME[] = L"BabaWindowClass";

    //ULONG_PTR gdiplusToken;
    //Gdiplus::GdiplusStartupInput gdiInput;
    //Gdiplus::GdiplusStartup(&gdiplusToken, &gdiInput, nullptr);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;              // �޽��� ó�� �Լ�
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = NULL;
    //wc.hbrBackground = CreateSolidBrush(RGB(0, 255, 0)); // ���� ���

    RegisterClass(&wc);

    GameManager& gm = GameManager::getInstance();

    // ������ ����
    HWND hWnd = CreateWindowEx(
        0,                          // Ȯ�� ��Ÿ��
        CLASS_NAME,                 // Ŭ���� �̸�
        L"Baba Is You",             // Ÿ��Ʋ
        WS_OVERLAPPEDWINDOW,        // â ��Ÿ��
        CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800, // ��ġ & ũ��
		nullptr, nullptr, hInstance, nullptr
    );

	gm.SetWindowHandle(hWnd);

    if (hWnd == nullptr) {
        return 0;
    }

    // â �����ֱ�
    ShowWindow(hWnd, nCmdShow);
    gm.Init();
    gm.Run();
	gm.Shutdown();

    //Gdiplus::GdiplusShutdown(gdiplusToken);
    return 0;
}

// �޽��� ó�� �Լ�
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    GameManager& gm = GameManager::getInstance();
    switch (message)
    {
	case WM_CREATE:

		break;

    case WM_KEYDOWN:
        gm.HandleInput(wParam);
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        gm.Render(hdc, 100, 80);
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0); // ���α׷� ����
        break;

    case WM_ERASEBKGND:
        return 1; // �ý��� ��� ����� ���� (������ ���� + BitBlt �� ������)

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}