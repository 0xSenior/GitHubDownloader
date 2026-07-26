#pragma once
#include "pch.h"
#include "ui/D2DRenderer.h"
#include "ui/MainWindow.h"
#include "utils/Logger.h"

class App {
public:
    static App& getInstance() {
        static App instance;
        return instance;
    }

    bool initialize(HINSTANCE hInstance);
    int run();
    void shutdown();

    HINSTANCE getHInstance() const { return m_hInstance; }
    MainWindow* getMainWindow() const { return m_mainWindow.get(); }
    Logger& getLogger() { return m_logger; }

private:
    App() = default;
    HINSTANCE m_hInstance = nullptr;
    std::unique_ptr<MainWindow> m_mainWindow;
    Logger m_logger;
    bool m_dpiAware = false;

    void enableDPIAwareness();
};
