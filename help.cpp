#include <windows.h>
int main() {
    const char* url = "https://bloretcrew.feishu.cn/wiki/BGXsw2TTUiqvREk1QLkc7nuCnvT?from=from_copylink";
    ShellExecute(0, 0, url, 0, 0, SW_SHOW);
    return 0;
}