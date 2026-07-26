#include "pch.h"
#include "download/DownloadManager.h"

DownloadManager::DownloadManager() {
    m_threadPool = std::make_unique<ThreadPool>(4);
}

DownloadManager::~DownloadManager() { cancel(); }

void DownloadManager::startDownload(const std::wstring& owner, const std::wstring& name,
    const std::wstring& branch, const fs::path& destDir, std::shared_ptr<FileNode> fileTree) {
    
    cancel();
    m_owner = owner;
    m_name = name;
    m_branch = branch;
    m_destDir = destDir;
    m_cancelled = false;
    m_paused = false;
    m_tasks.clear();
    m_logs.clear();

    FilesystemHelper::ensureDirectory(destDir);
    collectFiles(fileTree, L"");

    m_progress = {};
    m_progress.totalFiles = (int)m_tasks.size();
    m_state = DownloadState::Downloading;
    addLog(LogLevel::Info, std::format(L"Starting download of {} files to {}", m_tasks.size(), destDir.wstring()));

    for (size_t i = 0; i < m_tasks.size(); ++i) {
        m_threadPool->enqueue([this, i] { downloadFile(i); });
    }
}

void DownloadManager::collectFiles(std::shared_ptr<FileNode> node, const std::wstring& prefix) {
    if (!node) return;
    if (!node->isDir) {
        DownloadTask task;
        task.relativePath = prefix.empty() ? node->name : prefix + L"/" + node->name;
        task.destPath = m_destDir / fs::path(task.relativePath);
        task.url = L"https://raw.githubusercontent.com/" + m_owner + L"/" + m_name +
                   L"/" + m_branch + L"/" + task.relativePath;
        task.fileSize = node->size;
        m_tasks.push_back(std::move(task));
    }
    for (auto& child : node->children) {
        std::wstring childPrefix = prefix.empty() ? node->name : prefix + L"/" + node->name;
        if (node == child) continue;
        collectFiles(child, childPrefix);
    }
}

void DownloadManager::downloadFile(size_t taskIndex) {
    if (m_cancelled || taskIndex >= m_tasks.size()) return;

    auto& task = m_tasks[taskIndex];
    task.state = TaskState::Downloading;
    updateProgress();

    HttpClient http;
    bool ok = http.downloadToFile(task.url, task.destPath,
        [this, &task](uint64_t downloaded, uint64_t total) {
            task.bytesDownloaded = downloaded;
            task.fileSize = total;
            updateProgress();
        });

    {
        std::lock_guard lock(m_mutex);
        if (m_cancelled) {
            task.state = TaskState::Cancelled;
        } else if (ok) {
            task.state = TaskState::Completed;
            m_progress.completedFiles++;
            addLog(LogLevel::Info, L"Downloaded: " + task.relativePath);
        } else {
            task.state = TaskState::Failed;
            m_progress.failedFiles++;
            addLog(LogLevel::Error, L"Failed: " + task.relativePath);
        }
        m_progress.bytesDownloaded += task.bytesDownloaded;
    }

    bool allDone = true;
    for (auto& t : m_tasks) {
        if (t.state == TaskState::Pending || t.state == TaskState::Downloading) {
            allDone = false;
            break;
        }
    }

    if (allDone && !m_cancelled) {
        m_state = m_progress.failedFiles > 0 ? DownloadState::Failed : DownloadState::Completed;
        addLog(LogLevel::Info, std::format(L"Download complete. {} succeeded, {} failed.",
            m_progress.completedFiles, m_progress.failedFiles));
    }
    updateProgress();
}

void DownloadManager::updateProgress() {
    int total = (int)m_tasks.size();
    int done = 0;
    uint64_t bytes = 0;
    uint64_t totalBytes = 0;
    std::wstring current;

    for (auto& t : m_tasks) {
        if (t.state == TaskState::Completed) done++;
        bytes += t.bytesDownloaded;
        totalBytes += t.fileSize > 0 ? t.fileSize : t.bytesDownloaded;
        if (t.state == TaskState::Downloading) current = t.relativePath;
    }

    m_progress.totalFiles = total;
    m_progress.completedFiles = done;
    m_progress.bytesDownloaded = bytes;
    m_progress.totalBytes = totalBytes;
    m_progress.totalProgress = total > 0 ? (double)done / total : 0.0;
    m_progress.currentFile = current;

    if (m_progressCb) m_progressCb(m_progress);
}

void DownloadManager::addLog(LogLevel level, std::wstring msg) {
    LogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.level = level;
    entry.message = std::move(msg);
    {
        std::lock_guard lock(m_mutex);
        m_logs.push_back(entry);
        if (m_logs.size() > 200) m_logs.erase(m_logs.begin());
    }
    if (m_logCb) m_logCb(entry);
}

void DownloadManager::pause() {
    m_paused = true;
    m_state = DownloadState::Paused;
    addLog(LogLevel::Info, L"Download paused");
}

void DownloadManager::resume() {
    m_paused = false;
    m_state = DownloadState::Downloading;
    addLog(LogLevel::Info, L"Download resumed");
}

void DownloadManager::cancel() {
    m_cancelled = true;
    m_state = DownloadState::Cancelled;
    if (m_threadPool) m_threadPool = std::make_unique<ThreadPool>(4);
    addLog(LogLevel::Info, L"Download cancelled");
}
