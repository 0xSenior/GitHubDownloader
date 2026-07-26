#pragma once

enum class LogLevel { Debug, Info, Warning, Error };

enum class DownloadState { Idle, FetchingTree, Downloading, Paused, Completed, Failed, Cancelled };

struct RepoInfo {
    std::wstring owner;
    std::wstring name;
    std::wstring branch;
    std::wstring defaultBranch;
};

struct FileNode {
    std::wstring path;
    std::wstring name;
    bool isDir = false;
    uint64_t size = 0;
    std::vector<std::shared_ptr<FileNode>> children;
    bool selected = true;
    bool expanded = true;
};

struct DownloadProgress {
    std::wstring currentFile;
    int totalFiles = 0;
    int completedFiles = 0;
    int failedFiles = 0;
    double totalProgress = 0.0;
    uint64_t bytesDownloaded = 0;
    uint64_t totalBytes = 0;
    std::chrono::steady_clock::time_point startTime;
};

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::wstring message;
};

using ProgressCallback = std::function<void(const DownloadProgress&)>;
using LogCallback = std::function<void(const LogEntry&)>;

namespace Colors {
    inline D2D1_COLOR_F Background()        { return { 0.08f, 0.08f, 0.10f, 1.0f }; }
    inline D2D1_COLOR_F Surface()           { return { 0.13f, 0.13f, 0.15f, 1.0f }; }
    inline D2D1_COLOR_F SurfaceLight()      { return { 0.18f, 0.18f, 0.20f, 1.0f }; }
    inline D2D1_COLOR_F SurfaceHover()      { return { 0.22f, 0.22f, 0.25f, 1.0f }; }
    inline D2D1_COLOR_F Primary()           { return { 0.30f, 0.55f, 1.0f,  1.0f }; }
    inline D2D1_COLOR_F PrimaryHover()      { return { 0.40f, 0.65f, 1.0f,  1.0f }; }
    inline D2D1_COLOR_F PrimaryPressed()    { return { 0.25f, 0.45f, 0.90f, 1.0f }; }
    inline D2D1_COLOR_F Accent()            { return { 0.55f, 0.35f, 0.95f, 1.0f }; }
    inline D2D1_COLOR_F Success()           { return { 0.25f, 0.80f, 0.45f, 1.0f }; }
    inline D2D1_COLOR_F Error()             { return { 0.95f, 0.30f, 0.35f, 1.0f }; }
    inline D2D1_COLOR_F Warning()           { return { 1.00f, 0.75f, 0.20f, 1.0f }; }
    inline D2D1_COLOR_F TextPrimary()       { return { 0.93f, 0.93f, 0.95f, 1.0f }; }
    inline D2D1_COLOR_F TextSecondary()     { return { 0.58f, 0.58f, 0.63f, 1.0f }; }
    inline D2D1_COLOR_F TextMuted()         { return { 0.38f, 0.38f, 0.42f, 1.0f }; }
    inline D2D1_COLOR_F Border()            { return { 0.22f, 0.22f, 0.25f, 1.0f }; }
    inline D2D1_COLOR_F BorderFocus()       { return { 0.30f, 0.55f, 1.0f,  1.0f }; }
    inline D2D1_COLOR_F InputBg()           { return { 0.10f, 0.10f, 0.12f, 1.0f }; }
    inline D2D1_COLOR_F ProgressBar()       { return { 0.30f, 0.55f, 1.0f,  1.0f }; }
    inline D2D1_COLOR_F ProgressTrack()     { return { 0.18f, 0.18f, 0.20f, 1.0f }; }
    inline D2D1_COLOR_F TitleBar()          { return { 0.06f, 0.06f, 0.08f, 1.0f }; }
    inline D2D1_COLOR_F Scrollbar()         { return { 0.25f, 0.25f, 0.28f, 0.5f }; }
    inline D2D1_COLOR_F ScrollbarHover()    { return { 0.35f, 0.35f, 0.38f, 0.7f }; }
    inline D2D1_COLOR_F CheckBox()          { return { 0.30f, 0.55f, 1.0f,  1.0f }; }
    inline D2D1_COLOR_F Overlay()           { return { 0.0f,  0.0f,  0.0f,  0.5f }; }
    inline D2D1_COLOR_F FileIcon()          { return { 0.45f, 0.65f, 1.0f,  0.8f }; }
    inline D2D1_COLOR_F FolderIcon()        { return { 1.0f,  0.75f, 0.20f, 0.9f }; }
}

namespace Layout {
    constexpr float TitleBarHeight = 44.0f;
    constexpr float Padding = 16.0f;
    constexpr float SmallPadding = 8.0f;
    constexpr float TinyPadding = 4.0f;
    constexpr float CornerRadius = 8.0f;
    constexpr float SmallCornerRadius = 4.0f;
    constexpr float ButtonHeight = 36.0f;
    constexpr float InputHeight = 38.0f;
    constexpr float ProgressBarHeight = 5.0f;
    constexpr float TreeItemHeight = 28.0f;
    constexpr float LogItemHeight = 20.0f;
    constexpr float StatusbarHeight = 28.0f;
}
