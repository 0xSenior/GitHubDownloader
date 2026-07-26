#pragma once
#include "pch.h"

namespace Str {
    inline std::wstring toWide(const std::string& s) {
        if (s.empty()) return {};
        int len = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
        std::wstring result(len, 0);
        MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), result.data(), len);
        return result;
    }

    inline std::string toUtf8(const std::wstring& ws) {
        if (ws.empty()) return {};
        int len = WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
        std::string result(len, 0);
        WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), result.data(), len, nullptr, nullptr);
        return result;
    }

    inline std::wstring trim(const std::wstring& s) {
        auto start = s.find_first_not_of(L" \t\r\n");
        auto end = s.find_last_not_of(L" \t\r\n");
        return start == std::wstring::npos ? L"" : s.substr(start, end - start + 1);
    }

    inline bool startsWith(const std::wstring& s, const std::wstring& prefix) {
        return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
    }

    inline std::wstring toLower(std::wstring s) {
        std::transform(s.begin(), s.end(), s.begin(), ::towlower);
        return s;
    }

    inline std::wstring formatBytes(uint64_t bytes) {
        if (bytes < 1024) return std::to_wstring(bytes) + L" B";
        if (bytes < 1024 * 1024) return std::format(L"{:.1f} KB", bytes / 1024.0);
        if (bytes < 1024ULL * 1024 * 1024) return std::format(L"{:.1f} MB", bytes / (1024.0 * 1024));
        return std::format(L"{:.2f} GB", bytes / (1024.0 * 1024 * 1024));
    }

    inline std::wstring formatDuration(std::chrono::duration<double> dur) {
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur).count();
        if (secs < 60) return std::format(L"{}s", secs);
        if (secs < 3600) return std::format(L"{}m {}s", secs / 60, secs % 60);
        return std::format(L"{}h {}m", secs / 3600, (secs % 3600) / 60);
    }

    inline std::wstring formatSpeed(double bytesPerSec) {
        return formatBytes((uint64_t)bytesPerSec) + L"/s";
    }
}
