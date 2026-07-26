#pragma once
#include "pch.h"
#include "ui/D2DRenderer.h"
#include "ui/UIComponents.h"
#include "ui/AnimManager.h"
#include "github/GitHubApi.h"
#include "github/RepoParser.h"
#include "download/DownloadManager.h"

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    bool create(HINSTANCE hInstance, int width, int height);
    HWND getHwnd() const { return m_hwnd; }

private:
    HWND m_hwnd = nullptr;
    D2DRenderer m_renderer;
    GitHubApi m_githubApi;
    DownloadManager m_downloader;

    // State
    DownloadState m_uiState = DownloadState::Idle;
    std::wstring m_statusText = L"Ready";
    std::wstring m_repoOwner, m_repoName, m_repoBranch;
    std::wstring m_defaultBranch;
    std::wstring m_downloadPath;
    std::shared_ptr<FileNode> m_fileTree;
    bool m_treeLoaded = false;
    DownloadProgress m_lastProgress;

    // UI Elements
    UIButton m_btnPaste{ 0, 0, 70, 0, L"Paste" };
    UIButton m_btnFetch{ 0, 0, 80, 0, L"Fetch" };
    UIButton m_btnBrowse{ 0, 0, 36, 0, L"..." };
    UIButton m_btnDownload{ 0, 0, 140, 0, L"Download" };
    UIButton m_btnCancel{ 0, 0, 90, 0, L"Cancel" };
    UIButton m_btnOpenFolder{ 0, 0, 110, 0, L"Open Folder" };
    UIButton m_btnSelectAll{ 0, 0, 90, 0, L"Select All" };
    UIButton m_btnDeselectAll{ 0, 0, 100, 0, L"Deselect" };
    UIInput m_inputUrl{ 0, 0, 0, 0, L"Enter GitHub repo URL (e.g. https://github.com/user/repo)" };
    UIInput m_inputBranch{ 0, 0, 0, 0, L"Branch (default)" };
    UIInput m_inputPath{ 0, 0, 0, 0, L"Download folder" };
    UICheckbox m_chkExtract{ 0, 0, 16, L"Extract ZIP", false };
    UITreeView m_treeView{ 0, 0, 0, 0 };
    UILogPanel m_logPanel{ 0, 0, 0, 0 };
    UIProgressBar m_progressBar{ 0, 0, 0, 0 };
    UITextLabel m_lblStatus{ 0, 0, 0, 0, L"Ready", 11.0f, Colors::TextMuted() };
    UITextLabel m_lblRepoInfo{ 0, 0, 0, 0, L"", 12.0f, Colors::TextSecondary() };
    UITextLabel m_lblProgress{ 0, 0, 0, 0, L"", 12.0f, Colors::TextPrimary() };
    UITextLabel m_lblSpeed{ 0, 0, 0, 0, L"", 11.0f, Colors::TextMuted() };
    UITextLabel m_lblFiles{ 0, 0, 0, 0, L"", 11.0f, Colors::TextMuted() };
    UITextLabel m_lblTitle{ 0, 0, 0, 0, L"GitHub Repository Downloader", 16.0f, Colors::TextPrimary() };
    UITextLabel m_lblInputSection{ 0, 0, 0, 0, L"Repository URL", 12.0f, Colors::TextMuted() };
    UITextLabel m_lblBranch{ 0, 0, 0, 0, L"Branch", 12.0f, Colors::TextMuted() };
    UITextLabel m_lblSaveTo{ 0, 0, 0, 0, L"Save to", 12.0f, Colors::TextMuted() };
    UITextLabel m_lblTree{ 0, 0, 0, 0, L"Files", 12.0f, Colors::TextMuted() };
    UITextLabel m_lblLog{ 0, 0, 0, 0, L"Log", 12.0f, Colors::TextMuted() };

    // Layout
    float m_dpiScale = 1.0f;
    void relayout();
    void startFetch();
    void startDownload();
    void fetchComplete(std::shared_ptr<FileNode> tree, std::wstring defaultBranch);

    // Window procedure
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onMouseMove(int x, int y);
    void onMouseDown(int x, int y);
    void onMouseUp(int x, int y);
    void onWheel(int x, int y, int delta);
    void onSize(int w, int h);
    void onKeyDown(WCHAR key, bool ctrl);
    void onPaint();
    void onTimer(UINT id);
};
