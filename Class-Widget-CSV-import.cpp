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

    if (filePath.empty()) {
        MessageBoxW(hWnd, L"文本框为空", L"请输入文件路径", MB_OK | MB_ICONWARNING);
        return;
    }

    if (!PathFileExistsW(filePath.c_str())) {
        MessageBoxW(hWnd, L"文件不存在", L"请检查文件路径", MB_OK | MB_ICONSTOP);
        return;
    }

    if (!IsCsvFile(filePath)) {
        MessageBoxW(hWnd, L"文件不是.csv格式", L"请选择.csv文件", MB_OK | MB_ICONSTOP);
        return;
    }

    // 构建输出文件路径
    wstring outputFilePath = filePath.substr(0, filePath.find_last_of(L"\\/") + 1) + L"课表.json";

    wifstream inputFile(filePath.c_str());
    if (!inputFile.is_open()) {
        MessageBoxW(hWnd, L"无法打开输入文件", L"请检查文件路径", MB_OK | MB_ICONERROR);
        return;
    }

    wofstream outputFile(outputFilePath.c_str());
    if (!outputFile.is_open()) {
        MessageBoxW(hWnd, L"无法创建输出文件", L"请检查文件路径", MB_OK | MB_ICONERROR);
        return;
    }

    int time = 0;
    int itime = 0;  // 引入行号变量

    outputFile << L"{\n    \"timeline\": {\n        \"default\": {\n";

    wstring line;
    while (getline(inputFile, line)) {
        ++itime;  // 行号递增

        wistringstream iss(line);
        if (!(iss >> time)) {
            MessageBoxW(hWnd, L"无法读取时间", L"请检查文件内容", MB_OK | MB_ICONERROR);
            continue;
        }

        if (time == 0) {
            break;  // 如果读到的数字是 0，结束循环
        }

        wstring text;
        if (!(iss >> text)) {
            MessageBoxW(hWnd, L"无法读取文本", L"请检查文件内容", MB_OK | MB_ICONERROR);
            continue;
        }

        // 根据文本设置 kc 的值
        bool kc = (text == L"课程");

        // 处理读取的数据
        if (kc) {
            if (itime != 1) outputFile << L",\n    ";
            outputFile << L"\"a0" << itime << L"\": \"" << time << L"\"";
        } else {
            if (itime != 1) outputFile << L",\n    ";
            outputFile << L"\"f" << itime << L"\": \"" << time << L"\"";
        }
    }

    outputFile << L"\n        }\n    }\n}";

    // 关闭文件
    inputFile.close();
    outputFile.close();
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

    if (!RegisterClassExW(&wcex)) {
        MessageBoxW(NULL, L"窗口类注册失败", L"错误", MB_OK | MB_ICONERROR);
        return 0;
    }

    // 设置窗口的初始大小
    const wchar_t* windowTitle = L"csv 转 ClassWidgets json 课表转换器";
    hWnd = CreateWindowExW(0, L"SampleWindowClass", windowTitle, WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, 0, 550, 300, NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        MessageBoxW(NULL, L"窗口创建失败", L"错误", MB_OK | MB_ICONERROR);
        return 0;
    }

    // 显示窗口
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // 消息循环
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
            hEdit = CreateWindowW(L"EDIT", L"选择 CSV文件 或 Class Widgets json课表",
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
                NumCsv2Json(hWnd, hEdit);
            }
            if (LOWORD(wParam) == 3) {
                NumCsv2Json(hWnd, hEdit);
            }
            if (LOWORD(wParam) == 4) {
                NumCsv2Json(hWnd, hEdit);
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