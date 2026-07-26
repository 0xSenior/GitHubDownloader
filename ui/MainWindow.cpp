#include "pch.h"
#include "ui/MainWindow.h"
#include "utils/StringHelper.h"
#include "filesystem/FilesystemHelper.h"
#include "App.h"

MainWindow::MainWindow() {
    m_downloadPath = FilesystemHelper::getDefaultDownloadPath(L"", L"").wstring();
    m_inputPath.setText(m_downloadPath);
}

MainWindow::~MainWindow() {}

bool MainWindow::create(HINSTANCE hInstance, int width, int height) {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(20, 20, 25));
    wc.lpszClassName = L"GitHubDownloaderWnd";
    RegisterClassExW(&wc);

    RECT rc = { 0, 0, width, height };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    m_hwnd = CreateWindowExW(0, wc.lpszClassName, L"GitHub Downloader",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInstance, this);

    if (!m_hwnd) return false;

    if (!m_renderer.initialize(m_hwnd)) return false;

    D2D1_SIZE_F sz = m_renderer.getSize();
    relayout();

    m_downloader.setProgressCallback([this](const DownloadProgress& p) {
        m_lastProgress = p;
    });
    m_downloader.setLogCallback([this](const LogEntry& e) {
        PostMessage(m_hwnd, WM_USER + 100, 0, 0);
    });

    App::getInstance().getLogger().setCallback([this](const LogEntry& e) {
        m_logPanel.addEntry(e);
    });

    SetTimer(m_hwnd, TIMER_ID_ANIM, ANIM_TIMER_MS, nullptr);
    return true;
}

LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    MainWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto cs = reinterpret_cast<CREATESTRUCT*>(lp);
        self = static_cast<MainWindow*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->m_hwnd = hwnd;
    } else {
        self = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    if (self) return self->handleMessage(msg, wp, lp);
    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT MainWindow::handleMessage(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_PAINT: onPaint(); return 0;
    case WM_SIZE: onSize(LOWORD(lp), HIWORD(lp)); return 0;
    case WM_MOUSEMOVE: onMouseMove(GET_X_LPARAM(lp), GET_Y_LPARAM(lp)); return 0;
    case WM_LBUTTONDOWN: onMouseDown(GET_X_LPARAM(lp), GET_Y_LPARAM(lp)); return 0;
    case WM_LBUTTONUP: onMouseUp(GET_X_LPARAM(lp), GET_Y_LPARAM(lp)); return 0;
    case WM_MOUSEWHEEL: {
        POINT pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
        ScreenToClient(m_hwnd, &pt);
        onWheel(pt.x, pt.y, GET_WHEEL_DELTA_WPARAM(wp) / WHEEL_DELTA);
        return 0;
    }
    case WM_KEYDOWN: onKeyDown((WCHAR)wp, (GetKeyState(VK_CONTROL) & 0x8000) != 0); return 0;
    case WM_TIMER: onTimer((UINT)wp); return 0;
    case WM_USER + 100: {
        auto& entries = m_downloader.getLogs();
        if (!entries.empty()) m_logPanel.addEntry(entries.back());
        return 0;
    }
    case WM_USER + 200: {
        auto* data = reinterpret_cast<std::pair<std::shared_ptr<FileNode>, std::wstring>*>(lp);
        fetchComplete(data->first, data->second);
        delete data;
        return 0;
    }
    case WM_USER + 300: {
        auto& p = m_downloader.getProgress();
        m_lastProgress = p;
        m_progressBar.setProgress(p.totalProgress);
        InvalidateRect(m_hwnd, nullptr, FALSE);
        return 0;
    }
    case WM_USER + 301: {
        if (m_downloader.getState() == DownloadState::Completed) {
            m_uiState = DownloadState::Completed;
            m_statusText = std::format(L"Download complete! {} files downloaded.",
                m_lastProgress.completedFiles);
        } else {
            m_uiState = DownloadState::Failed;
            m_statusText = std::format(L"Download finished with {} failures.", m_lastProgress.failedFiles);
        }
        m_lblStatus.setText(m_statusText);
        relayout();
        InvalidateRect(m_hwnd, nullptr, FALSE);
        return 0;
    }
    case WM_DESTROY:
        KillTimer(m_hwnd, TIMER_ID_ANIM);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(m_hwnd, msg, wp, lp);
}

void MainWindow::onPaint() {
    PAINTSTRUCT ps;
    BeginPaint(m_hwnd, &ps);
    m_renderer.beginDraw();
    m_progressBar.update(0.016f);

    auto titleFmt = m_renderer.createTextFormat(16.0f, L"Segoe UI Variable", DWRITE_FONT_WEIGHT_SEMI_BOLD);
    auto sectionFmt = m_renderer.createTextFormat(11.0f, L"Segoe UI Variable", DWRITE_FONT_WEIGHT_SEMI_BOLD);
    auto smallFmt = m_renderer.createTextFormat(11.0f, L"Cascadia Code");

    D2D1_SIZE_F sz = m_renderer.getSize();
    float w = sz.width, h = sz.height;

    m_renderer.drawFilledRectangle(D2D1::RectF(0, 0, w, h), Colors::Background());
    m_renderer.drawFilledRectangle(D2D1::RectF(0, 0, w, Layout::TitleBarHeight), Colors::TitleBar());
    m_renderer.drawLine(D2D1::Point2F(0, Layout::TitleBarHeight), D2D1::Point2F(w, Layout::TitleBarHeight),
        Colors::Border(), 1.0f);

    m_renderer.drawText(L"GitHub Downloader", D2D1::RectF(Layout::Padding, 0, 400, Layout::TitleBarHeight),
        titleFmt.Get(), Colors::TextPrimary());

    m_renderer.drawText(L"v2.0", D2D1::RectF(w - 60, 0, w - Layout::Padding, Layout::TitleBarHeight),
        smallFmt.Get(), Colors::TextMuted(), D2D1_TEXT_ALIGNMENT_TRAILING);

    float y = Layout::TitleBarHeight + Layout::Padding;
    float contentW = w - Layout::Padding * 2;

    m_renderer.drawText(L"REPOSITORY URL", D2D1::RectF(Layout::Padding, y, 300, y + 16),
        sectionFmt.Get(), Colors::TextMuted());
    y += 20;

    m_inputUrl.render(m_renderer);
    y += Layout::InputHeight + Layout::SmallPadding;

    m_btnPaste.render(m_renderer);
    m_btnFetch.render(m_renderer);
    y += Layout::ButtonHeight + Layout::Padding;

    m_renderer.drawLine(D2D1::Point2F(Layout::Padding, y), D2D1::Point2F(w - Layout::Padding, y),
        Colors::Border(), 0.5f);
    y += Layout::Padding;

    if (m_treeLoaded || m_fileTree) {
        float leftW = contentW * 0.65f - Layout::SmallPadding / 2;
        float rightW = contentW * 0.35f - Layout::SmallPadding / 2;
        float leftX = Layout::Padding;
        float rightX = Layout::Padding + leftW + Layout::SmallPadding;

        float branchY = y;
        m_renderer.drawText(L"BRANCH", D2D1::RectF(leftX, branchY, leftX + 100, branchY + 16),
            sectionFmt.Get(), Colors::TextMuted());
        m_renderer.drawText(L"DOWNLOAD TO", D2D1::RectF(rightX, branchY, rightX + 200, branchY + 16),
            sectionFmt.Get(), Colors::TextMuted());
        y += 20;

        m_inputBranch.setRect(leftX, y, leftW, Layout::InputHeight);
        m_inputPath.setRect(rightX, y, rightW - Layout::ButtonHeight - Layout::SmallPadding, Layout::InputHeight);
        m_btnBrowse.setRect(rightX + rightW - Layout::ButtonHeight, y, Layout::ButtonHeight, Layout::InputHeight);
        m_inputBranch.render(m_renderer);
        m_inputPath.render(m_renderer);
        m_btnBrowse.render(m_renderer);
        y += Layout::InputHeight + Layout::Padding;

        float treeH = std::max(150.0f, h * 0.38f);
        m_treeView.setRect(leftX, y, leftW, treeH);
        m_treeView.render(m_renderer);

        float rightY = y;
        float btnRow = rightY;
        m_btnSelectAll.setRect(rightX, btnRow, 85, 26);
        m_btnDeselectAll.setRect(rightX + 90, btnRow, 90, 26);
        m_btnSelectAll.render(m_renderer);
        m_btnDeselectAll.render(m_renderer);
        btnRow += 30;

        int selCount = m_treeView.getSelectedCount();
        int totalCount = 0;
        if (m_fileTree) {
            std::function<void(std::shared_ptr<FileNode>)> countAll = [&](std::shared_ptr<FileNode> n) {
                if (!n->isDir) totalCount++;
                for (auto& c : n->children) countAll(c);
            };
            countAll(m_fileTree);
        }
        m_lblFiles.setText(std::format(L"{} of {} files selected", selCount, totalCount));
        m_lblFiles.setRect(rightX, btnRow, rightW, 18);
        m_lblFiles.render(m_renderer);
        btnRow += 24;

        m_logPanel.setRect(rightX, btnRow, rightW, treeH - 56);
        m_logPanel.render(m_renderer);
        y += treeH + Layout::Padding;

        if (m_uiState == DownloadState::Downloading) {
            m_progressBar.setRect(Layout::Padding, y, contentW - 130, Layout::ProgressBarHeight);
            m_progressBar.render(m_renderer);

            float pct = (float)(m_lastProgress.totalProgress * 100.0);
            m_lblProgress.setText(std::format(L"{:.1f}%", pct));
            m_lblProgress.setRect(contentW - 120, y - 8, contentW - 50, y + 20);
            m_lblProgress.render(m_renderer);

            m_lblSpeed.setText(Str::formatSpeed(
                m_lastProgress.totalBytes > 0 ?
                (double)m_lastProgress.bytesDownloaded /
                std::max(1.0, std::chrono::duration<double>(
                    std::chrono::steady_clock::now() - m_lastProgress.startTime).count()) : 0));
            m_lblSpeed.setRect(Layout::Padding, y + 12, 200, y + 28);
            m_lblSpeed.render(m_renderer);

            std::wstring progressText = std::format(L"{}  |  {}/{} files  |  {}/{}",
                m_lastProgress.currentFile.empty() ? L"" : m_lastProgress.currentFile,
                m_lastProgress.completedFiles, m_lastProgress.totalFiles,
                Str::formatBytes(m_lastProgress.bytesDownloaded),
                Str::formatBytes(m_lastProgress.totalBytes));
            m_lblStatus.setText(progressText);
            m_lblStatus.setRect(Layout::Padding, y + 28, contentW - 130, y + 44);
            m_lblStatus.render(m_renderer);

            m_btnCancel.setRect(contentW - 110, y - 8, 110, 34);
            m_btnCancel.render(m_renderer);
        } else {
            m_btnDownload.setRect(Layout::Padding, y, contentW - Layout::Padding, Layout::ButtonHeight + 4);
            m_btnDownload.setText(selCount > 0 ? std::format(L"Download {} Files", selCount) : L"Download");
            m_btnDownload.render(m_renderer);

            if (m_uiState == DownloadState::Completed) {
                m_btnOpenFolder.setRect(Layout::Padding, y + Layout::ButtonHeight + 12, 130, 30);
                m_btnOpenFolder.render(m_renderer);
            }
        }
    }

    float statusY = h - Layout::StatusbarHeight;
    m_renderer.drawLine(D2D1::Point2F(0, statusY), D2D1::Point2F(w, statusY), Colors::Border(), 0.5f);
    m_renderer.drawFilledRectangle(D2D1::RectF(0, statusY, w, h), Colors::TitleBar());
    m_lblStatus.setRect(Layout::Padding, statusY, w - Layout::Padding * 2, Layout::StatusbarHeight);
    m_lblStatus.render(m_renderer);

    m_renderer.endDraw();
    ValidateRect(m_hwnd, nullptr);
}

void MainWindow::onSize(int w, int h) {
    if (w < 10 || h < 10) return;
    m_renderer.resize(w, h);
    relayout();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void MainWindow::relayout() {
    D2D1_SIZE_F sz = m_renderer.getSize();
    float w = sz.width, h = sz.height;
    float contentW = w - Layout::Padding * 2;
    float y = Layout::TitleBarHeight + Layout::Padding + 20;

    m_inputUrl.setRect(Layout::Padding, y, contentW - 160, Layout::InputHeight);
    m_btnPaste.setRect(Layout::Padding + contentW - 155, y, 70, Layout::InputHeight);
    m_btnFetch.setRect(Layout::Padding + contentW - 80, y, 80, Layout::InputHeight);
}

void MainWindow::onMouseMove(int x, int y) {
    m_btnPaste.handleMouseMove(x, y);
    m_btnFetch.handleMouseMove(x, y);
    m_btnBrowse.handleMouseMove(x, y);
    m_btnDownload.handleMouseMove(x, y);
    m_btnCancel.handleMouseMove(x, y);
    m_btnOpenFolder.handleMouseMove(x, y);
    m_btnSelectAll.handleMouseMove(x, y);
    m_btnDeselectAll.handleMouseMove(x, y);
    m_treeView.handleMouseMove(x, y);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void MainWindow::onMouseDown(int x, int y) {
    if (m_inputUrl.handleMouseDown(x, y)) {
        m_inputBranch.setFocused(false);
        m_inputPath.setFocused(false);
    } else if (m_inputBranch.handleMouseDown(x, y)) {
        m_inputUrl.setFocused(false);
        m_inputPath.setFocused(false);
    } else if (m_inputPath.handleMouseDown(x, y)) {
        m_inputUrl.setFocused(false);
        m_inputBranch.setFocused(false);
    } else {
        m_inputUrl.setFocused(false);
        m_inputBranch.setFocused(false);
        m_inputPath.setFocused(false);
    }

    m_treeView.handleMouseDown(x, y);

    if (m_btnPaste.handleMouseDown(x, y)) {
        if (IsClipboardFormatAvailable(CF_UNICODETEXT) && OpenClipboard(nullptr)) {
            HANDLE h = GetClipboardData(CF_UNICODETEXT);
            if (h) {
                wchar_t* text = (wchar_t*)GlobalLock(h);
                if (text) {
                    m_inputUrl.setText(text);
                    GlobalUnlock(h);
                }
            }
            CloseClipboard();
        }
    }

    if (m_btnFetch.handleMouseDown(x, y)) startFetch();
    if (m_btnBrowse.handleMouseDown(x, y)) {
        fs::path p;
        if (FilesystemHelper::openFolderDialog(m_hwnd, p)) {
            m_downloadPath = p.wstring();
            m_inputPath.setText(m_downloadPath);
        }
    }
    if (m_btnDownload.handleMouseDown(x, y)) startDownload();
    if (m_btnCancel.handleMouseDown(x, y)) {
        m_downloader.cancel();
        m_uiState = DownloadState::Idle;
        m_statusText = L"Cancelled";
        m_lblStatus.setText(m_statusText);
    }
    if (m_btnOpenFolder.handleMouseDown(x, y)) {
        ShellExecuteW(nullptr, L"open", m_downloadPath.c_str(), nullptr, nullptr, SW_SHOW);
    }
    if (m_btnSelectAll.handleMouseDown(x, y) && m_fileTree) {
        std::function<void(std::shared_ptr<FileNode>)> setAll = [&](std::shared_ptr<FileNode> n) {
            n->selected = true;
            for (auto& c : n->children) setAll(c);
        };
        setAll(m_fileTree);
    }
    if (m_btnDeselectAll.handleMouseDown(x, y) && m_fileTree) {
        std::function<void(std::shared_ptr<FileNode>)> clearAll = [&](std::shared_ptr<FileNode> n) {
            n->selected = false;
            for (auto& c : n->children) clearAll(c);
        };
        clearAll(m_fileTree);
    }

    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void MainWindow::onMouseUp(int x, int y) {
    m_btnPaste.handleMouseUp(x, y);
    m_btnFetch.handleMouseUp(x, y);
    m_btnBrowse.handleMouseUp(x, y);
    m_btnDownload.handleMouseUp(x, y);
    m_btnCancel.handleMouseUp(x, y);
    m_btnOpenFolder.handleMouseUp(x, y);
    m_btnSelectAll.handleMouseUp(x, y);
    m_btnDeselectAll.handleMouseUp(x, y);
}

void MainWindow::onWheel(int x, int y, int delta) {
    if (m_treeView.hitTest(x, y)) m_treeView.handleWheel(delta);
    else if (m_logPanel.hitTest(x, y)) m_logPanel.handleWheel(delta);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void MainWindow::onKeyDown(WCHAR key, bool ctrl) {
    if (m_inputUrl.isFocused()) {
        m_inputUrl.handleKeyDown(key, ctrl);
        if (key == 0x0D) startFetch();
    } else if (m_inputBranch.isFocused()) {
        m_inputBranch.handleKeyDown(key, ctrl);
        if (key == 0x0D) startFetch();
    } else if (m_inputPath.isFocused()) {
        m_inputPath.handleKeyDown(key, ctrl);
    }
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void MainWindow::onTimer(UINT id) {
    if (id == TIMER_ID_ANIM) {
        if (m_downloader.isRunning()) {
            auto& p = m_downloader.getProgress();
            m_lastProgress = p;
            m_progressBar.setProgress(p.totalProgress);
        }
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void MainWindow::startFetch() {
    std::wstring url = Str::trim(m_inputUrl.getText());
    if (url.empty()) {
        m_statusText = L"Please enter a GitHub repository URL";
        m_lblStatus.setText(m_statusText);
        return;
    }

    auto parsed = RepoParser::parse(url);
    if (!parsed.valid) {
        m_statusText = L"Error: " + parsed.error;
        m_lblStatus.setText(m_statusText);
        return;
    }

    m_repoOwner = parsed.owner;
    m_repoName = parsed.name;
    m_repoBranch = parsed.branch;
    m_uiState = DownloadState::FetchingTree;
    m_statusText = L"Fetching repository tree...";
    m_lblStatus.setText(m_statusText);
    m_treeLoaded = false;
    m_fileTree = nullptr;
    InvalidateRect(m_hwnd, nullptr, FALSE);

    std::wstring owner = m_repoOwner;
    std::wstring name = m_repoName;
    std::wstring branch = m_repoBranch;

    std::thread([this, owner, name, branch]() {
        auto tree = m_githubApi.getRepoTree(owner, name, branch, m_defaultBranch);
        std::wstring defBranch = m_defaultBranch;
        PostMessage(m_hwnd, WM_USER + 200, 0, (LPARAM)new std::pair<std::shared_ptr<FileNode>, std::wstring>(tree, defBranch));
    }).detach();
}

void MainWindow::fetchComplete(std::shared_ptr<FileNode> tree, std::wstring defaultBranch) {
    if (!tree || tree->children.empty()) {
        m_statusText = L"Error: Could not fetch repository tree. Check the URL and try again.";
        m_uiState = DownloadState::Failed;
    } else {
        m_fileTree = tree;
        m_defaultBranch = defaultBranch;
        m_treeView.setRoot(tree);
        m_treeLoaded = true;
        m_repoBranch = m_defaultBranch;
        m_inputBranch.setText(m_repoBranch);

        int fileCount = 0;
        uint64_t totalSize = 0;
        std::function<void(std::shared_ptr<FileNode>)> countFiles = [&](std::shared_ptr<FileNode> n) {
            if (!n->isDir) { fileCount++; totalSize += n->size; }
            for (auto& c : n->children) countFiles(c);
        };
        countFiles(tree);

        m_statusText = std::format(L"Loaded {} files ({})", fileCount, Str::formatBytes(totalSize));
        m_uiState = DownloadState::Idle;
    }
    m_lblStatus.setText(m_statusText);
    relayout();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void MainWindow::startDownload() {
    if (!m_fileTree) return;

    std::wstring path = Str::trim(m_inputPath.getText());
    if (path.empty()) path = FilesystemHelper::getDefaultDownloadPath(m_repoOwner, m_repoName).wstring();
    m_downloadPath = path;

    fs::path destDir(m_downloadPath);
    m_uiState = DownloadState::Downloading;
    m_lastProgress = {};
    m_lastProgress.startTime = std::chrono::steady_clock::now();
    m_progressBar.setProgress(0);

    m_downloader.startDownload(m_repoOwner, m_repoName, m_repoBranch, destDir, m_fileTree);
    m_statusText = L"Downloading...";
    m_lblStatus.setText(m_statusText);
    InvalidateRect(m_hwnd, nullptr, FALSE);

    std::thread([this]() {
        while (m_downloader.isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            PostMessage(m_hwnd, WM_USER + 300, 0, 0);
        }
        PostMessage(m_hwnd, WM_USER + 301, 0, 0);
    }).detach();
}
