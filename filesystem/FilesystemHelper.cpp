#include "pch.h"
#include "FilesystemHelper.h"

bool FilesystemHelper::ensureDirectory(const fs::path& path) {
    std::error_code ec;
    fs::create_directories(path, ec);
    return !ec;
}

bool FilesystemHelper::removeDirectory(const fs::path& path) {
    std::error_code ec;
    fs::remove_all(path, ec);
    return !ec;
}

bool FilesystemHelper::fileExists(const fs::path& path) {
    std::error_code ec;
    return fs::exists(path, ec);
}

uint64_t FilesystemHelper::getFileSize(const fs::path& path) {
    std::error_code ec;
    return fs::file_size(path, ec);
}

fs::path FilesystemHelper::getDownloadsFolder() {
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Downloads, 0, nullptr, &path))) {
        fs::path result(path);
        CoTaskMemFree(path);
        return result;
    }
    return fs::temp_directory_path();
}

fs::path FilesystemHelper::getDefaultDownloadPath(const std::wstring& owner, const std::wstring& name) {
    return getDownloadsFolder() / (L"GitHub_" + owner + L"_" + name);
}

bool FilesystemHelper::openFolderDialog(HWND hwnd, fs::path& outPath) {
    wchar_t selected[MAX_PATH] = { 0 };
    BROWSEINFOW bi = {};
    bi.hwndOwner = hwnd;
    bi.lpszTitle = L"Select Download Folder";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn = [](HWND, UINT, LPARAM, LPARAM) -> int { return 0; };

    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    if (pidl) {
        SHGetPathFromIDListW(pidl, selected);
        CoTaskMemFree(pidl);
        outPath = selected;
        return true;
    }
    return false;
}

std::wstring FilesystemHelper::sanitizeFilename(const std::wstring& name) {
    std::wstring result = name;
    for (wchar_t& c : result) {
        if (c == L'<' || c == L'>' || c == L':' || c == L'"' || c == L'/' || c == L'\\' ||
            c == L'|' || c == L'?' || c == L'*') {
            c = L'_';
        }
    }
    return result;
}
