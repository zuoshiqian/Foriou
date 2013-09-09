#include "Shell/Main_win.h"
#include "Shell/Shell.h"
#include <gdiplus.h>
#include <stdexcept>
#include <iostream>

int main()
try {
    using namespace Gdiplus;

    ULONG_PTR gdiplusToken_;

    CPaintManagerUI::SetInstance(::GetModuleHandle(nullptr));
    CPaintManagerUI::SetResourcePath(L"UIRes");

    GdiplusStartupInput   gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken_, &gdiplusStartupInput, NULL);

    if (FAILED(::CoInitialize(NULL))) return 1;

    Foriou::Shell shell("Resource/shell.config");
    shell.show();

    CPaintManagerUI::MessageLoop();

    ::CoUninitialize();
    GdiplusShutdown(gdiplusToken_);
    return 0;
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
}
