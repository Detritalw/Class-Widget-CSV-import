#pragma comment(lib, "shlwapi.lib")
#include <windows.h>
#include <shlwapi.h>
#include <fcntl.h>
#include <io.h>
#include <bits/stdc++.h>
#include <chrono>
#include <thread>

using namespace std;

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

void NumCsv2Json(HWND hWnd) {
    // 显示“正在等待插件写入配置文件...”消息框
    MessageBoxW(hWnd, L"正在等待插件写入配置文件...", L"请稍候", MB_OK | MB_ICONINFORMATION);

    // 等待5秒
    Sleep(500000);

    // 读取 link.ini 文件中的内容
    wifstream linkFile("link.ini");
    if (!linkFile.is_open()) {
        MessageBoxW(NULL, L"无法与插件对接，无法打开link.ini文件，请检查插件是否有权限读取。", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    wstring filePath;
    wstring outputDir;
    getline(linkFile, filePath);
    getline(linkFile, outputDir);
    linkFile.close();

    // 检查文件路径和文件夹路径是否存在
    if (!PathFileExistsW(filePath.c_str()) || !PathFileExistsW(outputDir.c_str())) {
        MessageBoxW(NULL, L"link.ini 文件中的路径无效，请检查路径是否正确。", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

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
    wstring outputFilePath = outputDir + L"\\课表.json";

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

int main() {
    // 运行 NumCsv2Json 函数
    NumCsv2Json(NULL);

    return 0;
}