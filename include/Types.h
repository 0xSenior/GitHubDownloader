#pragma once

// Enums
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

enum class DownloadState {
    Pending,
    Connecting,
    Downloading,
    Paused,
    Completed,
    Failed,
    Cancelled
};

enum class ThemeType {
    Dark,
    Light
};

// Structs
struct RepoInfo {
    std::wstring owner;
    std::wstring name;
    std::wstring branch;
    std::wstring defaultBranch;
};

struct FileNode {
    std::wstring path;
    std::wstring name;
    std::wstring type;       // "file" or "dir"
    uint64_t size = 0;
    std::vector<std::shared_ptr<FileNode>> children;
};

struct DownloadProgress {
    std::wstring currentFile;
    int totalFiles = 0;
    int completedFiles = 0;
    int failedFiles = 0;
    double currentProgress = 0.0;
    double totalProgress = 0.0;
    uint64_t bytesDownloaded = 0;
    uint64_t totalBytes = 0;
    double speed = 0.0;  // bytes per second
    std::chrono::steady_clock::time_point startTime;
};

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::wstring message;
};

// Callbacks
using ProgressCallback = std::function<void(const DownloadProgress&)>;
using LogCallback = std::function<void(const LogEntry&)>;
using ErrorCallback = std::function<void(const std::wstring&)>;

// Constants
namespace Colors {
    constexpr D2D1_COLOR_F Background = { 0.10f, 0.10f, 0.12f, 1.0f };
    constexpr D2D1_COLOR_F Surface = { 0.15f, 0.15f, 0.17f, 1.0f };
    constexpr D2D1_COLOR_F SurfaceLight = { 0.20f, 0.20f, 0.22f, 1.0f };
    constexpr D2D1_COLOR_F Primary = { 0.35f, 0.60f, 1.0f, 1.0f };
    constexpr D2D1_COLOR_F PrimaryHover = { 0.45f, 0.70f, 1.0f, 1.0f };
    constexpr D2D1_COLOR_F Secondary = { 0.55f, 0.35f, 0.90f, 1.0f };
    constexpr D2D1_COLOR_F Success = { 0.30f, 0.85f, 0.45f, 1.0f };
    constexpr D2D1_COLOR_F Error = { 0.95f, 0.30f, 0.35f, 1.0f };
    constexpr D2D1_COLOR_F Warning = { 1.0f, 0.75f, 0.20f, 1.0f };
    constexpr D2D1_COLOR_F TextPrimary = { 0.95f, 0.95f, 0.97f, 1.0f };
    constexpr D2D1_COLOR_F TextSecondary = { 0.60f, 0.60f, 0.65f, 1.0f };
    constexpr D2D1_COLOR_F TextMuted = { 0.40f, 0.40f, 0.45f, 1.0f };
    constexpr D2D1_COLOR_F Border = { 0.25f, 0.25f, 0.28f, 1.0f };
    constexpr D2D1_COLOR_F InputBackground = { 0.12f, 0.12f, 0.14f, 1.0f };
    constexpr D2D1_COLOR_F ProgressBar = { 0.35f, 0.60f, 1.0f, 1.0f };
    constexpr D2D1_COLOR_F ProgressBarTrack = { 0.20f, 0.20f, 0.22f, 1.0f };
}

namespace Layout {
    constexpr float TitleBarHeight = 40.0f;
    constexpr float Padding = 16.0f;
    constexpr float SmallPadding = 8.0f;
    constexpr float CornerRadius = 8.0f;
    constexpr float ButtonHeight = 36.0f;
    constexpr float InputHeight = 32.0f;
    constexpr float ProgressBarHeight = 6.0f;
}
