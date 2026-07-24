#pragma once

// Windows Headers
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <winhttp.h>
#include <shellscalingapi.h>
#include <dwmapi.h>
#include <shlobj.h>
#include <shlwapi.h>

// C++20 Headers
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <memory>
#include <algorithm>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <future>
#include <chrono>
#include <format>
#include <source_location>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <optional>
#include <variant>
#include <ranges>
#include <concepts>
#include <coroutine>
#include <queue>
#include <condition_variable>
#include <semaphore>
#include <stop_token>

// Project Types
#include "Types.h"

// Smart Pointers Aliases
namespace Microsoft::WRL {
    template<typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;
}

// Global Constants
constexpr int WINDOW_MIN_WIDTH = 800;
constexpr int WINDOW_MIN_HEIGHT = 600;
constexpr int WINDOW_DEFAULT_WIDTH = 1200;
constexpr int WINDOW_DEFAULT_HEIGHT = 800;

// Utility Macros
#define LOG_ERROR(msg) Logger::getInstance().log(LogLevel::Error, msg, std::source_location::current())
#define LOG_INFO(msg) Logger::getInstance().log(LogLevel::Info, msg, std::source_location::current())
#define LOG_DEBUG(msg) Logger::getInstance().log(LogLevel::Debug, msg, std::source_location::current())
#define LOG_WARN(msg) Logger::getInstance().log(LogLevel::Warning, msg, std::source_location::current())
