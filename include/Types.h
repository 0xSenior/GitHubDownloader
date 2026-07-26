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
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
    LogLevel level = LogLevel::Info;
    std::wstring message;
};

using ProgressCallback = std::function<void(const DownloadProgress&)>;
using LogCallback = std::function<void(const LogEntry&)>;

namespace Colors {
    inline D2D1_COLOR_F Background()        { return { 0.043f, 0.059f, 0.098f, 1.0f }; } // #0b0f19
    inline D2D1_COLOR_F Surface()           { return { 0.075f, 0.098f, 0.149f, 1.0f }; } // #131926
    inline D2D1_COLOR_F SurfaceLight()      { return { 0.098f, 0.129f, 0.196f, 1.0f }; } // #192132
    inline D2D1_COLOR_F SurfaceHover()      { return { 0.133f, 0.173f, 0.259f, 1.0f }; } // #222c42
    inline D2D1_COLOR_F Primary()           { return { 0.388f, 0.400f, 0.945f, 1.0f }; } // #6366f1 (Indigo/Purple)
    inline D2D1_COLOR_F PrimaryHover()      { return { 0.310f, 0.275f, 0.898f, 1.0f }; } // #4f46e5
    inline D2D1_COLOR_F PrimaryPressed()    { return { 0.263f, 0.224f, 0.792f, 1.0f }; } // #4338ca
    inline D2D1_COLOR_F Accent()            { return { 0.545f, 0.361f, 0.965f, 1.0f }; } // #8b5cf6
    inline D2D1_COLOR_F Success()           { return { 0.133f, 0.773f, 0.369f, 1.0f }; } // #22c55e
    inline D2D1_COLOR_F Error()             { return { 0.937f, 0.267f, 0.267f, 1.0f }; } // #ef4444
    inline D2D1_COLOR_F Warning()           { return { 0.961f, 0.620f, 0.043f, 1.0f }; } // #f59e0b
    inline D2D1_COLOR_F TextPrimary()       { return { 0.973f, 0.980f, 0.988f, 1.0f }; } // #f8fafc
    inline D2D1_COLOR_F TextSecondary()     { return { 0.580f, 0.639f, 0.722f, 1.0f }; } // #94a3b8
    inline D2D1_COLOR_F TextMuted()         { return { 0.392f, 0.455f, 0.545f, 1.0f }; } // #64748b
    inline D2D1_COLOR_F TextBlue()          { return { 0.220f, 0.741f, 0.973f, 1.0f }; } // #38bdf8
    inline D2D1_COLOR_F TextCodeIcon()      { return { 0.976f, 0.451f, 0.086f, 1.0f }; } // #f97316
    inline D2D1_COLOR_F Border()            { return { 0.118f, 0.157f, 0.239f, 1.0f }; } // #1e283d
    inline D2D1_COLOR_F BorderFocus()       { return { 0.388f, 0.400f, 0.945f, 1.0f }; }
    inline D2D1_COLOR_F InputBg()           { return { 0.047f, 0.063f, 0.102f, 1.0f }; } // #0c101a
    inline D2D1_COLOR_F ProgressBar()       { return { 0.545f, 0.361f, 0.965f, 1.0f }; }
    inline D2D1_COLOR_F ProgressTrack()     { return { 0.098f, 0.129f, 0.196f, 1.0f }; }
    inline D2D1_COLOR_F TitleBar()          { return { 0.035f, 0.047f, 0.078f, 1.0f }; }
    inline D2D1_COLOR_F Scrollbar()         { return { 0.118f, 0.157f, 0.239f, 0.6f }; }
    inline D2D1_COLOR_F ScrollbarHover()    { return { 0.250f, 0.300f, 0.400f, 0.8f }; }
    inline D2D1_COLOR_F CheckBox()          { return { 0.388f, 0.400f, 0.945f, 1.0f }; }
    inline D2D1_COLOR_F Overlay()           { return { 0.0f,  0.0f,  0.0f,  0.5f }; }
    inline D2D1_COLOR_F FileIcon()          { return { 0.580f, 0.639f, 0.722f, 1.0f }; }
    inline D2D1_COLOR_F FolderIcon()        { return { 0.658f, 0.333f, 0.968f, 1.0f }; } // #a855f7
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
