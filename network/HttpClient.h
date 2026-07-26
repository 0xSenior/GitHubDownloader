#pragma once
#include "pch.h"

struct HttpResponse {
    bool success = false;
    int statusCode = 0;
    std::string body;
    std::string error;
};

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    HttpResponse get(const std::wstring& url, const std::wstring& headers = L"");
    bool downloadToFile(const std::wstring& url, const fs::path& dest, std::function<void(uint64_t, uint64_t)> progress = nullptr);

    void cancel() { m_cancelled = true; }
    void reset() { m_cancelled = false; }
    bool isCancelled() const { return m_cancelled; }

private:
    HINTERNET m_session = nullptr;
    std::atomic<bool> m_cancelled{ false };

    HINTERNET openUrl(const std::wstring& url, const std::wstring& headers = L"");
    bool parseUrl(const std::wstring& url, std::wstring& host, std::wstring& path, bool& useHttps, int& port);
};
