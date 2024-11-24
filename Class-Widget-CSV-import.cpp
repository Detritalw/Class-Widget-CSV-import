#pragma comment(lib, "shlwapi.lib")
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <fcntl.h>
#include <io.h>
#include <bits/stdc++.h>
#include <shlwapi.h>

using namespace std;

// 全局变量
HINSTANCE hInst;
HWND hEdit;

// 声明回调函数
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// 获取编辑框中的文本
wstring GetEditText(HWND hEdit) {
    int len = GetWindowTextLengthW(hEdit) + 1;
    wchar_t* buffer = new wchar_t[len];
    GetWindowTextW(hEdit, buffer, len);
    wstring result(buffer);
    delete[] buffer;
    return result;
}

// 检查文件是否为 CSV 文件
bool IsCsvFile(const wstring& filePath) {
    return PathFileExistsW(filePath.c_str()) && !PathIsDirectoryW(filePath.c_str()) && 
           _wcsicmp(PathFindExtensionW(filePath.c_str()), L".csv") == 0;
}

// 扫描 CSV 文件
wstring scancsv(HWND hWnd, HWND hEdit) {
    wstring filePath = GetEditText(hEdit);

    if (filePath.empty()) {
        MessageBoxW(hWnd, L"文本框为空", L"请输入文件路径", MB_OK | MB_ICONWARNING);
        return L"";
    }

    if (!PathFileExistsW(filePath.c_str())) {
        MessageBoxW(hWnd, L"文件不存在", L"请检查文件路径", MB_OK | MB_ICONSTOP);
        return L"";
    }

    if (!IsCsvFile(filePath)) {
        MessageBoxW(hWnd, L"文件不是.csv格式", L"请选择.csv文件", MB_OK | MB_ICONSTOP);
        return L"";
    }

    return filePath;
}

void NumCsv2Json(HWND hWnd, HWND hEdit) {
    wstring filePath = GetEditText(hEdit);
    int timekc = 40;  // 保留这一行
    string kindkc, text;
    bool kc = false;  // 移动到这里
    scancsv(hWnd, hEdit);

    // 构建输出文件路径
    wstring outputFilePath = filePath.substr(0, filePath.find_last_of(L"\\/") + 1) + L"课表.json";

    // 重定向标准输入到 CSV 文件
    FILE *inputFile = _wfreopen(filePath.c_str(), L"r", stdin);
    if (inputFile == NULL) {
        MessageBoxW(hWnd, L"无法打开输入文件", L"请检查文件路径", MB_OK | MB_ICONERROR);
        return;
    }

    // 重定向标准输出到 JSON 文件
    FILE *outputFile = _wfreopen(outputFilePath.c_str(), L"w", stdout);
    if (outputFile == NULL) {
        MessageBoxW(hWnd, L"无法创建输出文件", L"请检查文件路径", MB_OK | MB_ICONERROR);
        fclose(inputFile);
        return;
    }

    // 这里可以进行 CSV 到 JSON 的转换操作
    // 例如：
    int time = 0;
    // int timekc = 0;  // 删除这一行
    // bool kc = false;  // 移除这一行

    while (true) {
        // 读取时间
        if (scanf("%d,", &time) != 1) {
            printf("读取时间失败\n");
            break;
        }

        if (time == 0) {
            break;  // 如果读到的数字是 0，结束循环
        }

        // 读取时间kc
        if (scanf("%d,", &timekc) != 1) {
            printf("读取时间kc失败\n");
            break;
        }

        // 读取两个字的文本
        if (scanf("%2s,", &text) != 1) {
            printf("读取文本失败\n");
            break;
        }

        // 根据文本设置 kc 的值
        if (strcmp(text.c_str(), "课程") == 0) kc = true;
        else kc = false;

        // 处理读取的数据
        printf("time: %d, timekc: %d, kc: %s\n", time, timekc, kc ? "true" : "false");
    }

    // 关闭文件，恢复标准输入和输出
    fclose(inputFile);
    fclose(outputFile);
    freopen("CONIN$", "r", stdin);  // 恢复标准输入到控制台
    freopen("CONOUT$", "w", stdout);  // 恢复标准输出到控制台

    return;
}

HFONT CreateSmoothFont(int height = 20, int weight = FW_NORMAL, const wchar_t* faceName = L"Segoe UI") {
    LOGFONTW lf = {0};  // 使用 LOGFONTW 而不是 LOGFONT
    lf.lfHeight = height;
    lf.lfWeight = weight;
    lf.lfQuality = CLEARTYPE_QUALITY;
    wcscpy_s(lf.lfFaceName, LF_FACESIZE, faceName);  // 使用 wcscpy_s

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
    // 设置窗口的初始大小
    hWnd = CreateWindowExW(0, L"SampleWindowClass", L"csv文件 转 ClassWidgets json 课表文件", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, 0, 550, 300, NULL, NULL, hInstance, NULL);

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
                                  10, 10, 390, 25, hWnd, NULL, hInst, NULL);
            if (hEdit) {
                SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            }

            CreateButton(hWnd, hInst, hFont, L"选择文件", 410, 10, 100, 25, 1);
            CreateButton(hWnd, hInst, hFont, L"确认并从.csv文件(第一列为单个时间段的时间长度)转换到.json课表文件", 10, 40, 500, 25, 2);
            CreateButton(hWnd, hInst, hFont, L"确认并从.csv文件(第一列为单个时间段的起始时间)转换到.json课表文件", 10, 70, 500, 25, 3);
            CreateButton(hWnd, hInst, hFont, L"确认并从.json课表文件转换到.csv文件", 10, 100, 500, 25, 4);
            CreateButton(hWnd, hInst, hFont, L"需要帮助？", 100, 130, 100, 25, 5);
            CreateButton(hWnd, hInst, hFont, L"退出程序", 200, 130, 100, 25, 6);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                OPENFILENAMEW ofn;       // 使用宽字符版本的结构体
                wchar_t szFile[260];    // 缓冲区用于存储文件名
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hWnd;
                ofn.lpstrFile = szFile; // 正确赋值
                ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
                ofn.lpstrFilter = L"CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrFileTitle = NULL;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = NULL;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                if (GetOpenFileNameW(&ofn)) { // 使用宽字符版本的函数
                    SetWindowTextW(hEdit, ofn.lpstrFile);
                }
            }
            // 其他命令处理
            if (LOWORD(wParam) == 2) {
                scancsv(hWnd, hEdit);
            }
            if (LOWORD(wParam) == 3) {
                scancsv(hWnd, hEdit);
            }
            if (LOWORD(wParam) == 4) {
                scancsv(hWnd, hEdit);
            }
            if (LOWORD(wParam) == 5) MessageBoxW(hWnd, L"其实我也不知道帮助是什么", L"谢谢", MB_OK | MB_ICONINFORMATION);
            if (LOWORD(wParam) == 6) exit(0);
            break;
        }
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