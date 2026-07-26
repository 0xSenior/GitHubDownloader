#include "pch.h"
#include "include/App.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    auto& app = App::getInstance();
    if (!app.initialize(hInstance)) {
        MessageBoxW(nullptr, L"Failed to initialize application", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    int result = app.run();
    app.shutdown();
    return result;
}
