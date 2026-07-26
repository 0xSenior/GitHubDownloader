#pragma once

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
#include <commdlg.h>

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
#include <filesystem>
#include <fstream>
#include <sstream>
#include <optional>
#include <queue>
#include <condition_variable>
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <source_location>

#include "Types.h"

namespace fs = std::filesystem;

constexpr int WINDOW_MIN_WIDTH = 860;
constexpr int WINDOW_MIN_HEIGHT = 640;
constexpr int WINDOW_DEFAULT_WIDTH = 1280;
constexpr int WINDOW_DEFAULT_HEIGHT = 860;
constexpr UINT_PTR TIMER_ID_ANIM = 1;
constexpr UINT ANIM_TIMER_MS = 16;
