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
void Json2NumCsv(HWND hWnd, HWND hEdit){
    // 获取编辑控件中的文本作为文件路径
    wstring filePath = GetEditText(hEdit);

    // 检查文件路径是否为空
    if (filePath.empty()) {
        MessageBoxW(hWnd, L"文本框为空", L"请输入文件路径", MB_OK | MB_ICONWARNING);
        return;
    }

    // 检查文件是否存在
    if (!PathFileExistsW(filePath.c_str())) {
        MessageBoxW(hWnd, L"文件不存在", L"请检查文件路径", MB_OK | MB_ICONSTOP);
        return;
    }

    // 检查文件是否为JSON格式
    if (_wcsicmp(PathFindExtensionW(filePath.c_str()), L".json") != 0) {
        MessageBoxW(hWnd, L"文件不是.json格式", L"请选择.json文件", MB_OK | MB_ICONSTOP);
        return;
    }

    // 构建输出文件路径
    wstring outputFilePath = filePath.substr(0, filePath.find_last_of(L"\\/") + 1) + L"a.csv";

    // 打开输入文件，使用 UTF-8 编码
    wifstream inputFile(filePath.c_str());
    inputFile.imbue(locale(locale(), new codecvt_utf8<wchar_t>));
    if (!inputFile.is_open()) {
        MessageBoxW(hWnd, L"无法打开输入文件", L"请检查文件路径", MB_OK | MB_ICONERROR);
        return;
    }

    // 创建输出文件，使用 UTF-8 编码
    wofstream outputFile(outputFilePath.c_str());
    outputFile.imbue(locale(locale(), new codecvt_utf8<wchar_t>));
    if (!outputFile.is_open()) {
        MessageBoxW(hWnd, L"无法创建输出文件", L"请检查文件路径", MB_OK | MB_ICONERROR);
        return;
    }

    // 读取 JSON 文件内容
    wstring jsonContent((istreambuf_iterator<wchar_t>(inputFile)), istreambuf_iterator<wchar_t>());
    inputFile.close();

    // 解析 JSON 内容
    size_t partStart = jsonContent.find(L"\"part\": {");
    size_t partEnd = jsonContent.find(L"}", partStart) + 1;
    wstring partJson = jsonContent.substr(partStart, partEnd - partStart);

    size_t partNameStart = jsonContent.find(L"\"part_name\": {");
    size_t partNameEnd = jsonContent.find(L"}", partNameStart) + 1;
    wstring partNameJson = jsonContent.substr(partNameStart, partNameEnd - partNameStart);

    // 获取 part 中的项目数量
    size_t partCount = count(partJson.begin(), partJson.end(), L'[');

    // 输出 n+一个逗号，然后换行
    outputFile << partCount << L",\n";

    // 逐行输出 part_name 和 part 中的值
    size_t pos = 0;
    for (size_t i = 0; i < partCount; ++i) {
        // 获取 part_name 中的值
        size_t nameStart = partNameJson.find(L"\"", pos) + 1;
        size_t nameEnd = partNameJson.find(L"\"", nameStart);
        wstring partName = partNameJson.substr(nameStart, nameEnd - nameStart);

        // 获取 part 中的值
        size_t partStart = partJson.find(L"[", pos) + 1;
        size_t partEnd = partJson.find(L"]", partStart);
        wstring partValues = partJson.substr(partStart, partEnd - partStart);

        // 输出 part_name 和 part 中的值
        outputFile << partName << L"," << partValues << L"\n";

        pos = partEnd + 1;
    }

    // 解析 timeline 和 schedule
    size_t timelineStart = jsonContent.find(L"\"timeline\": {");
    size_t timelineEnd = jsonContent.find(L"}", timelineStart) + 1;
    wstring timelineJson = jsonContent.substr(timelineStart, timelineEnd - timelineStart);

    size_t scheduleStart = jsonContent.find(L"\"schedule\": {");
    size_t scheduleEnd = jsonContent.find(L"}", scheduleStart) + 1;
    wstring scheduleJson = jsonContent.substr(scheduleStart, scheduleEnd - scheduleStart);

    // 写入 CSV 文件
    outputFile << L"\n时间戳,课程名称\n";
    pos = 0;
    while ((pos = timelineJson.find(L"\"")) != wstring::npos) {
        size_t keyStart = pos + 1;
        size_t keyEnd = timelineJson.find(L"\"", keyStart);
        wstring key = timelineJson.substr(keyStart, keyEnd - keyStart);

        size_t valueStart = timelineJson.find(L"\"", keyEnd + 1) + 1;
        size_t valueEnd = timelineJson.find(L"\"", valueStart);
        wstring value = timelineJson.substr(valueStart, valueEnd - valueStart);

        size_t scheduleIndex;
        try {
            scheduleIndex = stoi(key.substr(1, 2)) - 1; // 修复索引解析错误
        } catch (const invalid_argument& e) {
            MessageBoxW(hWnd, L"无效的索引值", L"错误", MB_OK | MB_ICONERROR);
            return;
        } catch (const out_of_range& e) {
            MessageBoxW(hWnd, L"索引值超出范围", L"错误", MB_OK | MB_ICONERROR);
            return;
        }

        size_t schedulePos = scheduleJson.find(L"\"" + to_wstring(scheduleIndex) + L"\": [");
        if (schedulePos == wstring::npos) {
            MessageBoxW(hWnd, L"未找到对应的课程安排", L"错误", MB_OK | MB_ICONERROR);
            return;
        }
        size_t scheduleEndPos = scheduleJson.find(L"]", schedulePos);
        wstring schedule = scheduleJson.substr(schedulePos + to_wstring(scheduleIndex).length() + 4, scheduleEndPos - schedulePos - to_wstring(scheduleIndex).length() - 4);

        outputFile << value << L"," << schedule << L"\n";

        pos = valueEnd + 1;
    }

    // 关闭文件
    outputFile.close();

    // 提示任务完成
    MessageBoxW(hWnd, L"任务已完成", L"完成", MB_OK | MB_ICONINFORMATION);
}
void NumCsv2Json(HWND hWnd, HWND hEdit) {
    // 获取编辑控件中的文本作为文件路径
    wstring filePath = GetEditText(hEdit);

    // 检查文件路径是否为空
    if (filePath.empty()) {
        MessageBoxW(hWnd, L"文本框为空", L"请输入文件路径", MB_OK | MB_ICONWARNING);
        return;
    }

    // 检查文件是否存在
    if (!PathFileExistsW(filePath.c_str())) {
        MessageBoxW(hWnd, L"文件不存在", L"请检查文件路径", MB_OK | MB_ICONSTOP);
        return;
    }

    // 检查文件是否为CSV格式
    if (!IsCsvFile(filePath)) {
        MessageBoxW(hWnd, L"文件不是.csv格式", L"请选择.csv文件", MB_OK | MB_ICONSTOP);
        return;
    }

    // 构建输出文件路径
    wstring outputFilePath = filePath.substr(0, filePath.find_last_of(L"\\/") + 1) + L"课表.json";

    // 打开输入文件，使用 UTF-8 编码
    wifstream inputFile(filePath.c_str());
    inputFile.imbue(locale(locale(), new codecvt_utf8<wchar_t>));
    if (!inputFile.is_open()) {
        MessageBoxW(hWnd, L"无法打开输入文件", L"请检查文件路径", MB_OK | MB_ICONERROR);
        return;
    }

    // 创建输出文件，使用 UTF-8 编码
    wofstream outputFile(outputFilePath.c_str());
    outputFile.imbue(locale(locale(), new codecvt_utf8<wchar_t>));
    if (!outputFile.is_open()) {
        MessageBoxW(hWnd, L"无法创建输出文件", L"请检查文件路径", MB_OK | MB_ICONERROR);
        return;
    }

    // 初始化 timeline 和 schedule 的 JSON 字符串
    wstring timelineJson = L"    \"timeline\": {\n        \"default\": {\n";
    wstring scheduleJson = L"    \"schedule\": {\n";

    // 初始化行号
    int itime = 0;

    wstring line;
    // 逐行读取输入文件
    getline(inputFile, line); // 读取第一行，获取节点数量
    int nodeCount = stoi(line);

    wstring partJson = L"    \"part\": {\n";
    wstring partNameJson = L"    \"part_name\": {\n";

    for (int i = 0; i < nodeCount; ++i) {
        getline(inputFile, line);
        wistringstream iss(line);
        wstring token;
        vector<wstring> tokens;

        // 使用逗号作为分隔符
        while (getline(iss, token, L',')) {
            tokens.push_back(token);
        }

        // 检查是否有足够的列
        if (tokens.size() < 3) {
            // 显示消息框，提示发现错误，并提供选项：中止、继续和取消
            int result = MessageBoxW(hWnd, (L"发现格式错误的行: " + to_wstring(i + 1) + L"\n请选择操作").c_str(), L"格式错误", MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION);
            if (result == IDABORT) {
                // 中止操作
                MessageBoxW(hWnd, L"操作已中止", L"中止", MB_OK | MB_ICONINFORMATION);
                return;
            } else if (result == IDRETRY) {
                // 继续处理下一行
                continue;
            } else if (result == IDIGNORE) {
                // 取消操作
                MessageBoxW(hWnd, L"操作已取消", L"取消", MB_OK | MB_ICONINFORMATION);
                return;
            }
        }

        // 处理节点名称、小时和分钟
        wstring nodeName = tokens[0];
        int hour = stoi(tokens[1]);
        int minute = stoi(tokens[2]);

        // 构建 part 和 part_name 的 JSON 字符串
        partJson += L"        \"" + to_wstring(i) + L"\": [" + to_wstring(hour) + L", " + to_wstring(minute) + L"]";
        partNameJson += L"        \"" + to_wstring(i) + L"\": \"" + nodeName + L"\"";

        if (i < nodeCount - 1) {
            partJson += L",\n";
            partNameJson += L",\n";
        }
    }

    partJson += L"\n    },\n";
    partNameJson += L"\n    }\n";

    // 处理剩余的 schedule 数据
    while (getline(inputFile, line)) {
        wistringstream iss(line);
        wstring token;
        vector<wstring> tokens;

        // 使用逗号作为分隔符
        while (getline(iss, token, L',')) {
            tokens.push_back(token);
        }

        // 检查是否有足够的列
        if (tokens.size() < 9) {
            // 显示消息框，提示发现错误，并提供选项：中止、继续和取消
            int result = MessageBoxW(hWnd, (L"发现格式错误的行: " + to_wstring(itime + 1) + L"\n请选择操作").c_str(), L"格式错误", MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION);
            if (result == IDABORT) {
                // 中止操作
                MessageBoxW(hWnd, L"操作已中止", L"中止", MB_OK | MB_ICONINFORMATION);
                return;
            } else if (result == IDRETRY) {
                // 继续处理下一行
                continue;
            } else if (result == IDIGNORE) {
                // 取消操作
                MessageBoxW(hWnd, L"操作已取消", L"取消", MB_OK | MB_ICONINFORMATION);
                return;
            }
        }

        // 处理时间戳
        int time = stoi(tokens[0]);
        wstring key = (itime % 2 == 0) ? L"f" : L"a";
        timelineJson += L"            \"" + key + L"0" + to_wstring(itime + 1) + L"\": \"" + to_wstring(time) + L"\"";
        if (itime < 5) timelineJson += L",\n";  // 如果不是最后一行，则需要添加逗号

        // 添加课程名称到 schedule
        vector<wstring> scheduleItems(tokens.begin() + 2, tokens.end()); // 从第3列开始读取

        // 构建当前时间段的 schedule 部分
        scheduleJson += L"        \"" + to_wstring(itime) + L"\": [\n";
        for (size_t i = 0; i < scheduleItems.size(); ++i) {
            scheduleJson += L"            \"" + scheduleItems[i] + L"\"";
            if (i < scheduleItems.size() - 1) {
                scheduleJson += L",\n";
            } else {
                scheduleJson += L"\n";
            }
        }
        scheduleJson += L"        ]";
        if (itime < 5) scheduleJson += L",\n";  // 如果不是最后一行，则需要添加逗号

        ++itime;  // 行号递增
    }

    // 关闭文件
    inputFile.close();

    // 完成 timeline 和 schedule 的 JSON 字符串
    timelineJson += L"\n        }\n    },\n";
    scheduleJson += L"\n    },";  // 移除最后一个换行符

    // 在倒数第二个 } 符号之前添加换行符
    scheduleJson += L"\n";

    // 构建输出字符串
    wstring outputJson = L"{\n" + timelineJson + scheduleJson + partJson + partNameJson + L"}";

    // 写入输出文件
    outputFile << outputJson;

    // 关闭文件
    outputFile.close();

    // 提示任务完成
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
            CreateButton(hWnd, hInst, hFont, L"确认并从.json课表文件转换到.csv文件", 10, 100, 500, 25, 4);
            CreateButton(hWnd, hInst, hFont, L"需要帮助？", 100, 130, 100, 25, 5);
            CreateButton(hWnd, hInst, hFont, L"退出程序", 200, 130, 100, 25, 6);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                OPENFILENAMEW ofn;       // 使用宽字符版本的结构体
                wchar_t szFile[260] = {0};    // 缓冲区用于存储文件名
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hWnd;
                ofn.lpstrFile = szFile; // 正确赋值
                ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
                ofn.lpstrFilter = L"csv 表格 (*.csv)\0*.csv\0Class Widgets 课表 (*.json)\0*.json\0All Files (*.*)\0*.*\0";
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
                Json2NumCsv(hWnd, hEdit);
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