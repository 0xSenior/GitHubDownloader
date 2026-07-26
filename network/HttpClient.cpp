#include "pch.h"
#include "network\HttpClient.h"

#pragma comment(lib, "winhttp.lib")

HttpClient::HttpClient() {
    m_session = WinHttpOpen(L"GitHubDownloader/2.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
}

HttpClient::~HttpClient() {
    if (m_session) WinHttpCloseHandle(m_session);
}

bool HttpClient::parseUrl(const std::wstring& url, std::wstring& host, std::wstring& path, bool& useHttps, int& port) {
    useHttps = true;
    port = 443;
    std::wstring work = url;

    if (Str::startsWith(work, L"https://")) {
        work = work.substr(8);
    } else if (Str::startsWith(work, L"http://")) {
        work = work.substr(7);
        useHttps = false;
        port = 80;
    }

    size_t pathStart = work.find(L'/');
    if (pathStart != std::wstring::npos) {
        host = work.substr(0, pathStart);
        path = work.substr(pathStart);
    } else {
        host = work;
        path = L"/";
    }

    size_t colonPos = host.find(L':');
    if (colonPos != std::wstring::npos) {
        port = std::stoi(host.substr(colonPos + 1));
        host = host.substr(0, colonPos);
    }
    return !host.empty();
}

HINTERNET HttpClient::openUrl(const std::wstring& url, const std::wstring& headers) {
    std::wstring host, path;
    bool useHttps;
    int port;
    if (!parseUrl(url, host, path, useHttps, port)) return nullptr;

    HINTERNET connect = WinHttpConnect(m_session, host.c_str(), (INTERNET_PORT)port, 0);
    if (!connect) return nullptr;

    HINTERNET request = WinHttpOpenRequest(connect, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, useHttps ? WINHTTP_FLAG_SECURE : 0);
    if (!request) { WinHttpCloseHandle(connect); return nullptr; }

    DWORD securityFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
        SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
    WinHttpSetOption(request, WINHTTP_OPTION_SECURITY_FLAGS, &securityFlags, sizeof(securityFlags));

    BOOL sent = WinHttpSendRequest(request, headers.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headers.c_str(),
        (DWORD)headers.size(), WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

    if (!sent) { WinHttpCloseHandle(request); WinHttpCloseHandle(connect); return nullptr; }

    if (!WinHttpReceiveResponse(request, nullptr)) {
        WinHttpCloseHandle(request); WinHttpCloseHandle(connect); return nullptr;
    }

    WinHttpCloseHandle(connect);
    return request;
}

HttpResponse HttpClient::get(const std::wstring& url, const std::wstring& headers) {
    HttpResponse resp;
    m_cancelled = false;

    HINTERNET request = openUrl(url, headers);
    if (!request) { resp.error = "Failed to connect"; return resp; }

    DWORD statusCode = 0;
    DWORD size = sizeof(statusCode);
    WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &size, WINHTTP_NO_HEADER_INDEX);
    resp.statusCode = (int)statusCode;

    std::string body;
    DWORD bytesAvailable;
    while (WinHttpQueryDataAvailable(request, &bytesAvailable) && bytesAvailable > 0) {
        if (m_cancelled) { WinHttpCloseHandle(request); resp.error = "Cancelled"; return resp; }
        std::vector<char> buf(bytesAvailable);
        DWORD bytesRead;
        WinHttpReadData(request, buf.data(), bytesAvailable, &bytesRead);
        body.append(buf.data(), bytesRead);
    }

    WinHttpCloseHandle(request);
    resp.body = std::move(body);
    resp.success = (statusCode >= 200 && statusCode < 300);
    return resp;
}

bool HttpClient::downloadToFile(const std::wstring& url, const fs::path& dest,
    std::function<void(uint64_t, uint64_t)> progress) {
    m_cancelled = false;

    HINTERNET request = openUrl(url);
    if (!request) return false;

    DWORD totalSize = 0;
    DWORD sizeLen = sizeof(totalSize);
    WinHttpQueryHeaders(request, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &totalSize, &sizeLen, WINHTTP_NO_HEADER_INDEX);

    fs::create_directories(dest.parent_path());
    std::ofstream file(dest, std::ios::binary);
    if (!file.is_open()) { WinHttpCloseHandle(request); return false; }

    uint64_t downloaded = 0;
    DWORD bytesAvailable;
    while (WinHttpQueryDataAvailable(request, &bytesAvailable) && bytesAvailable > 0) {
        if (m_cancelled) { WinHttpCloseHandle(request); file.close(); fs::remove(dest); return false; }
        std::vector<char> buf(bytesAvailable);
        DWORD bytesRead;
        WinHttpReadData(request, buf.data(), bytesAvailable, &bytesRead);
        file.write(buf.data(), bytesRead);
        downloaded += bytesRead;
        if (progress) progress(downloaded, totalSize);
    }

    file.close();
    WinHttpCloseHandle(request);
    return downloaded > 0;
}
