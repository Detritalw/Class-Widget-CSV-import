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

// 创建平滑字体
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

// 扫描 CSV 文件
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

    int itime = 0;  // 引入行号变量
    wstring line;
    wstring timelineJson = L"{\n    \"timeline\": {\n        \"default\": {\n";
    wstring scheduleJson = L"    \"schedule\": {\n";  // 声明并初始化 scheduleJson
    bool firstTimelineEntry = true;
    bool firstScheduleEntry = true;

    while (getline(inputFile, line)) {
        wistringstream iss(line);
        int time;
        wstring dummy; // 用于跳过前两列
        if (!(iss >> time >> dummy)) {
            continue;  // 跳过不符合格式的行
        }

        // 处理时间戳
        if (!firstTimelineEntry) timelineJson += L",\n";  // 如果不是第一行，则需要添加逗号
        else firstTimelineEntry = false;
        wstring key = (itime % 2 == 0) ? L"f" : L"a";
        timelineJson += L"            \"" + key + L"0" + to_wstring(itime + 1) + L"\": \"" + to_wstring(time) + L"\"";

        // 添加课程名称到 schedule
        wstring courseName;
        vector<wstring> scheduleItems;
        for (int i = 0; i < 7; ++i) { // 从第3列开始读取7个元素
            if (!(iss >> quoted(courseName))) { // 使用 quoted 以处理可能的引号
                break; // 如果没有更多的列，跳出循环
            }
            scheduleItems.push_back(courseName);
        }

        // 构建当前时间段的 schedule 部分
        if (!scheduleItems.empty()) {
            if (!firstScheduleEntry) scheduleJson += L",\n";  // 如果不是第一行，则需要添加逗号
            else firstScheduleEntry = false;
            scheduleJson += L"        \"" + to_wstring(itime) + L"\": [\n";
            for (size_t i = 0; i < scheduleItems.size(); ++i) {
                if (i > 0) scheduleJson += L", ";
                scheduleJson += L"\"" + scheduleItems[i] + L"\"";
            }
            scheduleJson += L"\n        ]";
        }

        ++itime;  // 行号递增
    }

    // 结束 timeline 和 schedule 部分
    timelineJson += L"\n        }\n    },\n";
    scheduleJson += L"\n    }\n}";

    // 合并 timeline 和 schedule 部分
    wstring finalJson = timelineJson + scheduleJson;

    // 写入输出文件
    outputFile << finalJson;

    // 关闭文件
    inputFile.close();
    outputFile.close();
    MessageBoxW(hWnd, L"任务已完成", L"完成", MB_OK | MB_ICONINFORMATION);
    return;
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