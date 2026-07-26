#pragma once
#include "pch.h"

class Logger {
public:
    Logger() = default;

    void log(LogLevel level, std::wstring msg, const std::source_location& loc = std::source_location::current()) {
        LogEntry entry;
        entry.timestamp = std::chrono::system_clock::now();
        entry.level = level;
        entry.message = std::move(msg);

        {
            std::lock_guard lock(m_mutex);
            m_entries.push_back(entry);
            if (m_entries.size() > 500) m_entries.erase(m_entries.begin());
        }

        if (m_callback) m_callback(entry);
    }

    void setCallback(LogCallback cb) { m_callback = std::move(cb); }
    const std::vector<LogEntry>& getEntries() const { return m_entries; }
    void clear() { std::lock_guard lock(m_mutex); m_entries.clear(); }

private:
    std::vector<LogEntry> m_entries;
    std::mutex m_mutex;
    LogCallback m_callback;
};

#define LOG_INFO(msg)    App::getInstance().getLogger().log(LogLevel::Info, msg)
#define LOG_WARN(msg)    App::getInstance().getLogger().log(LogLevel::Warning, msg)
#define LOG_ERROR(msg)   App::getInstance().getLogger().log(LogLevel::Error, msg)
#define LOG_DEBUG(msg)   App::getInstance().getLogger().log(LogLevel::Debug, msg)
