#include "pch.h"
#include "include/App.h"
#include <exception>
#include <string>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    try {
        auto& app = App::getInstance();
        if (!app.initialize(hInstance)) {
            MessageBoxW(nullptr, L"Failed to initialize application", L"Error", MB_OK | MB_ICONERROR);
            return 1;
        }
        int result = app.run();
        app.shutdown();
        return result;
    } catch (const std::exception& e) {
        std::string msg = "Exception caught: ";
        msg += e.what();
        MessageBoxA(nullptr, msg.c_str(), "Runtime Exception", MB_OK | MB_ICONERROR);
        return 1;
    } catch (...) {
        MessageBoxW(nullptr, L"Unknown Exception caught in wWinMain", L"Runtime Exception", MB_OK | MB_ICONERROR);
        return 1;
    }
}

