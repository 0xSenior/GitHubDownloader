#include "pch.h"
#include "include/App.h"
#include "utils/StringHelper.h"

#pragma comment(lib, "dwmapi.lib")

void App::enableDPIAwareness() {
    auto hShcore = LoadLibraryW(L"shcore.dll");
    if (hShcore) {
        typedef HRESULT(WINAPI* PFN_SetProcessDpiAwareness)(int);
        auto pFunc = (PFN_SetProcessDpiAwareness)GetProcAddress(hShcore, "SetProcessDpiAwareness");
        if (pFunc) { pFunc(2); m_dpiAware = true; }
        FreeLibrary(hShcore);
    }
}

bool App::initialize(HINSTANCE hInstance) {
    m_hInstance = hInstance;
    enableDPIAwareness();
    INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icex);

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    m_mainWindow = std::make_unique<MainWindow>();
    if (!m_mainWindow->create(hInstance, WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT)) {
        return false;
    }

    const int DWMWA_USE_IMMERSIVE_DARK_MODE_VALUE = 20;
    BOOL dark = TRUE;
    DwmSetWindowAttribute(m_mainWindow->getHwnd(), DWMWA_USE_IMMERSIVE_DARK_MODE_VALUE, &dark, sizeof(dark));

    m_logger.log(LogLevel::Info, L"Application initialized");
    return true;
}

int App::run() {
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void App::shutdown() {
    m_logger.log(LogLevel::Info, L"Application shutting down");
    m_mainWindow.reset();
    CoUninitialize();
}
