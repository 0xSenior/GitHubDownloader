#pragma once
#include "pch.h"
#include "download/DownloadTask.h"
#include "network/HttpClient.h"
#include "utils/ThreadPool.h"

class DownloadManager {
public:
    DownloadManager();
    ~DownloadManager();

    void startDownload(const std::wstring& owner, const std::wstring& name,
                       const std::wstring& branch, const fs::path& destDir,
                       std::shared_ptr<FileNode> fileTree);

    void pause();
    void resume();
    void cancel();

    DownloadState getState() const { return m_state; }
    const DownloadProgress& getProgress() const { return m_progress; }
    const std::vector<DownloadTask>& getTasks() const { return m_tasks; }
    const std::vector<LogEntry>& getLogs() const { return m_logs; }

    void setProgressCallback(ProgressCallback cb) { m_progressCb = std::move(cb); }
    void setLogCallback(LogCallback cb) { m_logCb = std::move(cb); }

    bool isRunning() const { return m_state == DownloadState::Downloading || m_state == DownloadState::FetchingTree; }

private:
    DownloadState m_state = DownloadState::Idle;
    DownloadProgress m_progress;
    std::vector<DownloadTask> m_tasks;
    std::vector<LogEntry> m_logs;
    fs::path m_destDir;
    std::wstring m_branch;
    std::wstring m_owner;
    std::wstring m_name;

    ProgressCallback m_progressCb;
    LogCallback m_logCb;

    std::unique_ptr<ThreadPool> m_threadPool;
    std::mutex m_mutex;
    std::atomic<bool> m_cancelled{ false };
    std::atomic<bool> m_paused{ false };

    void collectFiles(std::shared_ptr<FileNode> node, const std::wstring& prefix);
    void downloadFile(size_t taskIndex);
    void updateProgress();
    void addLog(LogLevel level, std::wstring msg);
};
