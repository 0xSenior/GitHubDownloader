#pragma once
#include "pch.h"

enum class FilesystemResult { Success, AlreadyExists, Error };

class FilesystemHelper {
public:
    static bool ensureDirectory(const fs::path& path);
    static bool removeDirectory(const fs::path& path);
    static bool fileExists(const fs::path& path);
    static uint64_t getFileSize(const fs::path& path);
    static fs::path getDownloadsFolder();
    static fs::path getDefaultDownloadPath(const std::wstring& repoOwner, const std::wstring& repoName);
    static bool openFolderDialog(HWND hwnd, fs::path& outPath);
    static std::wstring sanitizeFilename(const std::wstring& name);
};
