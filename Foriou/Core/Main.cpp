#include "Core/Core.h"

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE,
                     LPSTR lpszArgument, int nCmdShow)
try {
    Foriou::Core app(hInstance);
    if (!app.init_app()) {
        ::MessageBox(NULL, L"Initialization failed", L"Foriou", MB_OK);
        return -1;
    }

    return app.exec();
}
catch (std::exception& e) {
    ::MessageBoxA(NULL, e.what(), "Foriou", MB_OK);
    return -2;
}
//----------------------------------------------------------------------
// End of File
//----------------------------------------------------------------------
