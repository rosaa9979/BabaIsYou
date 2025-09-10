// 메모리 누수 검사  
#ifdef _DEBUG  
#define _CRTDBG_MAP_ALLOC  
#include <crtdbg.h>  
#include <stdlib.h>  
#endif  

#include <windows.h>  
#include <gdiplus.h>  
#include <iostream>  
#include "GameManager.h"  

#pragma comment(lib, "gdiplus.lib")  

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);  

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {  
    // 메모리 누수 검사 시작  
    #ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);  
    #endif  

    const wchar_t CLASS_NAME[] = L"BabaWindowClass";  

    WNDCLASS wc = {};  
    wc.lpfnWndProc = WndProc;              
    wc.hInstance = hInstance;  
    wc.lpszClassName = CLASS_NAME;  
    wc.hbrBackground = NULL;  

    RegisterClass(&wc);  

    GameManager& gm = GameManager::getInstance();  

    HWND hWnd = CreateWindowEx(  
        0,                          
        CLASS_NAME,                  
        L"Baba Is You",               
        WS_OVERLAPPEDWINDOW,          
        CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800,   
        nullptr, nullptr, hInstance, nullptr  
    );  

    gm.SetWindowHandle(hWnd);  

    if (hWnd == nullptr) {  
        return 0;  
    }  

    ShowWindow(hWnd, nCmdShow);  
    gm.Init();  
    gm.Run();  
    gm.Shutdown();  

    return 0;  
}  

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
        PostQuitMessage(0);  
        break;  

    case WM_ERASEBKGND:  
        return 1;  

    default:  
        return DefWindowProc(hWnd, message, wParam, lParam);  
    }  

    return 0;  
}