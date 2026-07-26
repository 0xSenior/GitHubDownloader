#pragma once
#include "pch.h"
#include "network/HttpClient.h"

enum class TaskState { Pending, Downloading, Completed, Failed, Cancelled };

struct DownloadTask {
    std::wstring url;
    fs::path destPath;
    std::wstring relativePath;
    TaskState state = TaskState::Pending;
    uint64_t fileSize = 0;
    uint64_t bytesDownloaded = 0;
    std::string error;
};
