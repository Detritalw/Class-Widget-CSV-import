#define UNICODE
#define _UNICODE

#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <bits/stdc++.h>

using namespace std;

// 全局变量
HINSTANCE hInst;
HWND hEdit;

// 声明回调函数
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void scancsv(HWND hWnd){
    MessageBoxW(hWnd, L"文件打开失败", L"请重试", MB_OK | MB_ICONSTOP);
}

HFONT CreateSmoothFont(int height = 20, int weight = FW_NORMAL, const wchar_t* faceName = L"Segoe UI") {
    LOGFONT lf = {0};
    lf.lfHeight = height;
    lf.lfWeight = weight;
    lf.lfQuality = CLEARTYPE_QUALITY;
    wcsncpy_s(lf.lfFaceName, LF_FACESIZE, faceName, _TRUNCATE);

    HFONT hFont = CreateFontIndirectW(&lf);

    if (hFont == NULL) {
        return NULL;
    }

    return hFont;
}

void CreateButton(HWND hWnd, HINSTANCE hInst, HFONT hFont, const wchar_t* text, int x, int y, int width, int height, int id);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEXW wcex;
    HWND hWnd;
    MSG msg;

    hInst = hInstance;

    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"SampleWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    RegisterClassExW(&wcex);
    hWnd = CreateWindowExW(0, L"SampleWindowClass", L"Updata", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit;
    static HINSTANCE hInst;

    switch (message) {
        case WM_CREATE: {
            HFONT hFont = CreateSmoothFont();
            hEdit = CreateWindowW(L"EDIT", L"选择 Class Widgets 文件夹",
                                  WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                  10, 10, 300, 25, hWnd, NULL, hInst, NULL);
            if (hEdit) {
                SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            }

            CreateButton(hWnd, hInst, hFont, L"选择文件夹", 320, 10, 100, 25, 1);
            CreateButton(hWnd, hInst, hFont, L"确认并从.csv文件(第一列为单个时间段的时间长度)转换到.json课表文件", 10, 40, 500, 25, 2);
            CreateButton(hWnd, hInst, hFont, L"确认并从.csv文件(第一列为单个时间段的起始时间)转换到.json课表文件", 10, 70, 500, 25, 3);
            CreateButton(hWnd, hInst, hFont, L"确认并从.json课表文件转换到.csv文件", 10, 100, 500, 25, 4);
            CreateButton(hWnd, hInst, hFont, L"需要帮助？", 200, 130, 100, 25, 5);

            break;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                BROWSEINFOW bi = { 0 };
                bi.hwndOwner = hWnd;
                bi.pszDisplayName = new WCHAR[MAX_PATH];
                bi.lpszTitle = L"选择一个文件夹";
                LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);

                if (pidl != nullptr) {
                    WCHAR path[MAX_PATH];
                    if (SHGetPathFromIDListW(pidl, path)) {
                        SetWindowTextW(hEdit, path);
                    }
                    CoTaskMemFree(pidl);
                }
                delete[] bi.pszDisplayName;
                scancsv(hWnd);
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void CreateButton(HWND hWnd, HINSTANCE hInst, HFONT hFont, const wchar_t* text, int x, int y, int width, int height, int id) {
    HWND hButton = CreateWindowW(L"BUTTON", text,
                                 WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                 x, y, width, height, hWnd, (HMENU)id, hInst, NULL);
    if (hButton) {
        SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
}