#include "pch.h"
#include "ui/MainWindow.h"
#include "utils/StringHelper.h"
#include "filesystem/FilesystemHelper.h"
#include "App.h"

static std::shared_ptr<FileNode> createInitialMockTree() {
    auto root = std::make_shared<FileNode>();
    root->name = L"0xSenior/GitHubDownloader";
    root->isDir = true;
    root->expanded = true;

    // download
    auto dlDir = std::make_shared<FileNode>();
    dlDir->name = L"download"; dlDir->isDir = true; dlDir->expanded = true;
    auto f1 = std::make_shared<FileNode>(); f1->name = L"DownloadManager.cpp"; f1->size = 14230; f1->selected = false;
    auto f2 = std::make_shared<FileNode>(); f2->name = L"DownloadManager.h"; f2->size = 3450; f2->selected = false;
    dlDir->children = { f1, f2 };

    // filesystem
    auto fsDir = std::make_shared<FileNode>();
    fsDir->name = L"filesystem"; fsDir->isDir = true; fsDir->expanded = true;
    auto f3 = std::make_shared<FileNode>(); f3->name = L"FilesystemHelper.cpp"; f3->size = 8920; f3->selected = false;
    auto f4 = std::make_shared<FileNode>(); f4->name = L"FilesystemHelper.h"; f4->size = 2100; f4->selected = false;
    fsDir->children = { f3, f4 };

    // github
    auto ghDir = std::make_shared<FileNode>();
    ghDir->name = L"github"; ghDir->isDir = true; ghDir->expanded = true;
    auto f5 = std::make_shared<FileNode>(); f5->name = L"GitHubApi.cpp"; f5->size = 11400; f5->selected = false;
    auto f6 = std::make_shared<FileNode>(); f6->name = L"RepoParser.cpp"; f6->size = 6800; f6->selected = false;
    ghDir->children = { f5, f6 };

    // ui
    auto uiDir = std::make_shared<FileNode>();
    uiDir->name = L"ui"; uiDir->isDir = true; uiDir->expanded = true;
    auto f7 = std::make_shared<FileNode>(); f7->name = L"D2DRenderer.cpp"; f7->size = 18600; f7->selected = false;
    auto f8 = std::make_shared<FileNode>(); f8->name = L"MainWindow.cpp"; f8->size = 28400; f8->selected = false;
    auto f9 = std::make_shared<FileNode>(); f9->name = L"UIComponents.cpp"; f9->size = 22400; f9->selected = false;
    uiDir->children = { f7, f8, f9 };

    // web
    auto webDir = std::make_shared<FileNode>();
    webDir->name = L"web"; webDir->isDir = true; webDir->expanded = true;
    auto f10 = std::make_shared<FileNode>(); f10->name = L"index.html"; f10->size = 12750; f10->selected = false;
    auto f11 = std::make_shared<FileNode>(); f11->name = L"style.css"; f11->size = 18900; f11->selected = false;
    auto f12 = std::make_shared<FileNode>(); f12->name = L"app.js"; f12->size = 12500; f12->selected = false;
    webDir->children = { f10, f11, f12 };

    // Root files
    auto f13 = std::make_shared<FileNode>(); f13->name = L"README.md"; f13->size = 3850; f13->selected = false;
    auto f14 = std::make_shared<FileNode>(); f14->name = L"GitHubDownloader.sln"; f14->size = 1420; f14->selected = false;

    root->children = { dlDir, fsDir, ghDir, uiDir, webDir, f13, f14 };
    return root;
}

MainWindow::MainWindow() {
    m_inputUrl.setText(L"https://github.com/0xSenior/GitHubDownloader");
    m_inputBranch.setText(L"main");
    m_downloadPath = L"C:\\Users\\Senior\\Downloads\\GitHub_";
    m_inputPath.setText(m_downloadPath);

    D2D1_COLOR_F glassBg = D2D1::ColorF(0.12f, 0.15f, 0.23f, 1.0f);
    m_btnPaste.setStyle(glassBg, Colors::TextPrimary());
    m_btnBrowse.setStyle(glassBg, Colors::TextPrimary());

    m_fileTree = createInitialMockTree();
    m_treeView.setRoot(m_fileTree);
    m_treeLoaded = true;
    m_statusText = L"Ready";
}

MainWindow::~MainWindow() {}

bool MainWindow::create(HINSTANCE hInstance, int width, int height) {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(11, 15, 25));
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
    m_downloader.setLogCallback([this](const LogEntry&) {
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
        if (self) self->m_hwnd = hwnd;
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

static void drawGitHubLogo(D2DRenderer& r, float x, float y, float targetSize) {
    D2D1_COLOR_F white = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const std::string githubSvgPath =
        "M12 0 C5.37 0 0 5.37 0 12 c0 5.31 3.435 9.795 8.205 11.385 c0.6 0.105 0.825 -0.255 0.825 -0.57 c0 -0.285 -0.015 -1.23 -0.015 -2.235 c-3.015 0.555 -3.795 -0.735 -4.035 -1.41 c-0.135 -0.345 -0.72 -1.41 -1.23 -1.695 c-0.42 -0.225 -1.02 -0.78 -0.015 -0.795 c0.945 -0.015 1.62 0.87 1.845 1.23 c1.08 1.815 2.805 1.305 3.495 0.99 c0.105 -0.78 0.42 -1.305 0.765 -1.605 c-2.67 -0.3 -5.46 -1.335 -5.46 -5.925 c0 -1.305 0.465 -2.385 1.23 -3.225 c-0.12 -0.3 -0.54 -1.53 0.12 -3.18 c0 0 1.005 -0.315 3.3 1.23 c0.96 -0.27 1.98 -0.405 3 -0.405 s2.04 0.135 3 0.405 c2.295 -1.56 3.3 -1.23 3.3 -1.23 c0.66 1.65 0.24 2.88 0.12 3.18 c0.765 0.84 1.23 1.905 1.23 3.225 c0 4.605 -2.805 5.625 -5.475 5.925 c0.435 0.375 0.81 1.095 0.81 2.22 c0 1.605 -0.015 2.895 -0.015 3.3 c0 0.315 0.225 0.69 0.825 0.57 A12.02 12.02 0 0 0 24 12 c0 -6.63 -5.37 -12 -12 -12 z";

    float scale = targetSize / 24.0f;
    r.drawSvgPath(githubSvgPath, white, scale, x, y);
}

static void drawHeaderIconAvatar(D2DRenderer& r, float x, float y, int type) {
    // Icon Avatar Box
    D2D1_RECT_F avatarRect = D2D1::RectF(x, y, x + 28, y + 28);
    D2D1_COLOR_F purpleBg = { 0.545f, 0.361f, 0.965f, 0.12f };
    D2D1_COLOR_F purpleBorder = { 0.545f, 0.361f, 0.965f, 0.25f };
    D2D1_COLOR_F iconColor = { 0.658f, 0.545f, 0.980f, 1.0f }; // #a78bfa

    r.drawFilledRoundedRectangle(avatarRect, purpleBg, 6.0f);
    r.drawRoundedRectangle(avatarRect, purpleBorder, 6.0f, 1.0f);

    float cx = x + 14.0f;
    float cy = y + 14.0f;

    if (type == 0) {
        // Link Icon
        r.drawLine(D2D1::Point2F(cx - 5, cy + 2), D2D1::Point2F(cx + 2, cy - 5), iconColor, 1.8f);
        r.drawLine(D2D1::Point2F(cx - 2, cy - 2), D2D1::Point2F(cx + 4, cy - 2), iconColor, 1.8f);
        r.drawLine(D2D1::Point2F(cx + 2, cy - 2), D2D1::Point2F(cx + 2, cy + 4), iconColor, 1.8f);
    } else if (type == 1) {
        // Branch Icon
        r.drawLine(D2D1::Point2F(cx - 4, cy - 6), D2D1::Point2F(cx - 4, cy + 6), iconColor, 1.8f);
        r.drawFilledEllipse(D2D1::Point2F(cx - 4, cy - 4), 2.5f, 2.5f, iconColor);
        r.drawFilledEllipse(D2D1::Point2F(cx - 4, cy + 4), 2.5f, 2.5f, iconColor);
        r.drawFilledEllipse(D2D1::Point2F(cx + 4, cy - 4), 2.5f, 2.5f, iconColor);
        r.drawLine(D2D1::Point2F(cx - 4, cy + 1), D2D1::Point2F(cx + 4, cy - 4), iconColor, 1.5f);
    } else if (type == 2) {
        // Folder Icon
        D2D1_RECT_F folderRect = D2D1::RectF(cx - 6, cy - 4, cx + 6, cy + 5);
        r.drawFilledRoundedRectangle(folderRect, iconColor, 1.5f);
        D2D1_RECT_F tabRect = D2D1::RectF(cx - 6, cy - 6, cx - 1, cy - 4);
        r.drawFilledRoundedRectangle(tabRect, iconColor, 1.0f);
    }
}

void MainWindow::onPaint() {
    PAINTSTRUCT ps;
    BeginPaint(m_hwnd, &ps);
    m_renderer.beginDraw();
    m_progressBar.update(0.016f);

    auto titleFmt = m_renderer.createTextFormat(17.0f, L"Segoe UI", DWRITE_FONT_WEIGHT_BOLD);
    auto subFmt = m_renderer.createTextFormat(11.5f, L"Segoe UI");
    auto sectionFmt = m_renderer.createTextFormat(12.5f, L"Segoe UI", DWRITE_FONT_WEIGHT_SEMI_BOLD);
    auto badgeFmt = m_renderer.createTextFormat(10.5f, L"Segoe UI", DWRITE_FONT_WEIGHT_BOLD);

    D2D1_SIZE_F sz = m_renderer.getSize();
    float w = sz.width, h = sz.height;
    float contentW = w - Layout::Padding * 2;

    // Window Background
    m_renderer.drawFilledRectangle(D2D1::RectF(0, 0, w, h), Colors::Background());

    // 1. Header Bar
    float headerH = 60.0f;
    m_renderer.drawFilledRectangle(D2D1::RectF(0, 0, w, headerH), Colors::TitleBar());
    m_renderer.drawLine(D2D1::Point2F(0, headerH), D2D1::Point2F(w, headerH), Colors::Border(), 1.0f);

    // Title GitHub Mark + Title & Subtitle
    drawGitHubLogo(m_renderer, Layout::Padding, 16.0f, 26.0f);
    m_renderer.drawText(L"GitHub Downloader", D2D1::RectF(Layout::Padding + 34, 12, 400, 32),
        titleFmt.Get(), Colors::TextPrimary());
    m_renderer.drawText(L"Download GitHub repositories with ease", D2D1::RectF(Layout::Padding + 34, 32, 400, 52),
        subFmt.Get(), Colors::TextSecondary());

    // Version Badge pill
    D2D1_RECT_F verBadgeRect = D2D1::RectF(w - Layout::Padding - 60, 18, w - Layout::Padding, 40);
    D2D1_COLOR_F verBg = { 0.388f, 0.400f, 0.945f, 0.15f };
    D2D1_COLOR_F verBorder = { 0.388f, 0.400f, 0.945f, 0.35f };
    m_renderer.drawFilledRoundedRectangle(verBadgeRect, verBg, 11.0f);
    m_renderer.drawRoundedRectangle(verBadgeRect, verBorder, 11.0f, 1.0f);
    m_renderer.drawText(L"v1.0.0", verBadgeRect, badgeFmt.Get(), Colors::Accent(), DWRITE_TEXT_ALIGNMENT_CENTER);

    float curY = headerH + Layout::Padding;

    // 2. Section 1: Repository Card
    float repoCardH = 110.0f;
    D2D1_RECT_F repoCardRect = D2D1::RectF(Layout::Padding, curY, w - Layout::Padding, curY + repoCardH);
    m_renderer.drawFilledRoundedRectangle(repoCardRect, Colors::Surface(), Layout::CornerRadius);
    m_renderer.drawRoundedRectangle(repoCardRect, Colors::Border(), Layout::CornerRadius, 1.0f);

    // Section 1 Header with Icon Avatar
    float innerX = Layout::Padding + 16.0f;
    float innerY = curY + 14.0f;
    drawHeaderIconAvatar(m_renderer, innerX, innerY, 0); // Link icon

    m_renderer.drawText(L"Repository", D2D1::RectF(innerX + 36, innerY, innerX + 250, innerY + 18),
        sectionFmt.Get(), Colors::TextPrimary());
    m_renderer.drawText(L"Enter GitHub repository URL", D2D1::RectF(innerX + 36, innerY + 18, innerX + 250, innerY + 34),
        subFmt.Get(), Colors::TextMuted());

    // Input URL row
    float inputY = innerY + 40.0f;
    float btnW_Fetch = 90.0f;
    float btnW_Paste = 80.0f;
    float inputW = contentW - 32 - btnW_Fetch - btnW_Paste - 20;

    m_inputUrl.setRect(innerX, inputY, inputW, Layout::InputHeight);
    m_btnPaste.setRect(innerX + inputW + 10, inputY, btnW_Paste, Layout::InputHeight);
    m_btnFetch.setRect(innerX + inputW + 10 + btnW_Paste + 10, inputY, btnW_Fetch, Layout::InputHeight);

    m_inputUrl.render(m_renderer);
    m_btnPaste.render(m_renderer);
    m_btnFetch.render(m_renderer);

    curY += repoCardH + 16.0f;

    // 3. Section 2: Branch & Download To (Grid two columns)
    float colW = (contentW - 16.0f) / 2.0f;
    float gridCardH = 95.0f;

    // Left Column Card: Branch
    D2D1_RECT_F branchCardRect = D2D1::RectF(Layout::Padding, curY, Layout::Padding + colW, curY + gridCardH);
    m_renderer.drawFilledRoundedRectangle(branchCardRect, Colors::Surface(), Layout::CornerRadius);
    m_renderer.drawRoundedRectangle(branchCardRect, Colors::Border(), Layout::CornerRadius, 1.0f);

    drawHeaderIconAvatar(m_renderer, Layout::Padding + 16, curY + 12, 1); // Branch icon
    m_renderer.drawText(L"Branch", D2D1::RectF(Layout::Padding + 16 + 36, curY + 10, Layout::Padding + colW, curY + 26),
        sectionFmt.Get(), Colors::TextPrimary());
    m_renderer.drawText(L"Select repository branch", D2D1::RectF(Layout::Padding + 16 + 36, curY + 26, Layout::Padding + colW, curY + 42),
        subFmt.Get(), Colors::TextMuted());

    m_inputBranch.setRect(Layout::Padding + 16, curY + 48, colW - 32, Layout::InputHeight);
    m_inputBranch.render(m_renderer);

    // Right Column Card: Download To
    float rightColX = Layout::Padding + colW + 16.0f;
    D2D1_RECT_F pathCardRect = D2D1::RectF(rightColX, curY, rightColX + colW, curY + gridCardH);
    m_renderer.drawFilledRoundedRectangle(pathCardRect, Colors::Surface(), Layout::CornerRadius);
    m_renderer.drawRoundedRectangle(pathCardRect, Colors::Border(), Layout::CornerRadius, 1.0f);

    drawHeaderIconAvatar(m_renderer, rightColX + 16, curY + 12, 2); // Folder icon
    m_renderer.drawText(L"Download To", D2D1::RectF(rightColX + 16 + 36, curY + 10, rightColX + colW, curY + 26),
        sectionFmt.Get(), Colors::TextPrimary());
    m_renderer.drawText(L"Choose download location", D2D1::RectF(rightColX + 16 + 36, curY + 26, rightColX + colW, curY + 42),
        subFmt.Get(), Colors::TextMuted());

    float pathInputW = colW - 32 - Layout::InputHeight - 8;
    m_inputPath.setRect(rightColX + 16, curY + 48, pathInputW, Layout::InputHeight);
    m_btnBrowse.setRect(rightColX + 16 + pathInputW + 8, curY + 48, Layout::InputHeight, Layout::InputHeight);
    m_inputPath.render(m_renderer);
    m_btnBrowse.render(m_renderer);

    curY += gridCardH + 16.0f;

    // 4. Section 3: Files & Selection Split View
    float treeCardW = contentW * 0.58f;
    float selCardW = contentW * 0.42f - 16.0f;
    float splitH = std::max(280.0f, h - curY - 120.0f);

    // Left Card: Repository Files
    D2D1_RECT_F filesCardRect = D2D1::RectF(Layout::Padding, curY, Layout::Padding + treeCardW, curY + splitH);
    m_renderer.drawFilledRoundedRectangle(filesCardRect, Colors::Surface(), Layout::CornerRadius);
    m_renderer.drawRoundedRectangle(filesCardRect, Colors::Border(), Layout::CornerRadius, 1.0f);

    // Header Repository Files + Badges
    m_renderer.drawText(L"Repository Files", D2D1::RectF(Layout::Padding + 16, curY + 14, Layout::Padding + 200, curY + 32),
        sectionFmt.Get(), Colors::TextPrimary());

    int selCount = m_treeView.getSelectedCount();
    m_renderer.drawText(L"14 files  \x2022  5 folders", D2D1::RectF(Layout::Padding + treeCardW - 170, curY + 14, Layout::Padding + treeCardW - 16, curY + 32),
        subFmt.Get(), Colors::TextBlue(), DWRITE_TEXT_ALIGNMENT_TRAILING);

    m_treeView.setRect(Layout::Padding + 12, curY + 42, treeCardW - 24, splitH - 54);
    m_treeView.render(m_renderer);

    // Right Card: Selection Panel
    float selX = Layout::Padding + treeCardW + 16.0f;
    D2D1_RECT_F selCardRect = D2D1::RectF(selX, curY, selX + selCardW, curY + splitH);
    m_renderer.drawFilledRoundedRectangle(selCardRect, Colors::Surface(), Layout::CornerRadius);
    m_renderer.drawRoundedRectangle(selCardRect, Colors::Border(), Layout::CornerRadius, 1.0f);

    // Action buttons: Select All / Deselect
    m_btnSelectAll.setRect(selX + 14, curY + 12, 85, 28);
    m_btnDeselectAll.setRect(selX + 105, curY + 12, 85, 28);
    m_btnSelectAll.render(m_renderer);
    m_btnDeselectAll.render(m_renderer);

    m_lblFiles.setText(std::format(L"{} files selected", selCount));
    m_lblFiles.setRect(selX + selCardW - 130, curY + 16, selX + selCardW - 14, curY + 32);
    m_lblFiles.render(m_renderer);

    // Selection Body Area (Empty State container or Log panel)
    D2D1_RECT_F bodyRect = D2D1::RectF(selX + 14, curY + 52, selX + selCardW - 14, curY + splitH - 14);
    m_renderer.drawRoundedRectangle(bodyRect, Colors::Border(), Layout::CornerRadius, 1.0f);

    if (selCount == 0) {
        // Render Empty State Illustration text
        float midX = selX + selCardW / 2;
        float midY = curY + 52 + (splitH - 66) / 2;

        D2D1_POINT_2F center = D2D1::Point2F(midX, midY - 30);
        m_renderer.drawFilledEllipse(center, 30.0f, 30.0f, { 1.0f, 1.0f, 1.0f, 0.03f });
        m_renderer.drawEllipse(center, 30.0f, 30.0f, Colors::Border());

        D2D1_COLOR_F amber = { 0.961f, 0.620f, 0.043f, 0.9f };
        m_renderer.drawFilledRoundedRectangle(D2D1::RectF(midX - 16, midY - 38, midX + 16, midY - 20), amber, 3.0f);

        m_renderer.drawText(L"No files selected", D2D1::RectF(selX + 20, midY + 10, selX + selCardW - 20, midY + 30),
            sectionFmt.Get(), Colors::TextPrimary(), DWRITE_TEXT_ALIGNMENT_CENTER);
        m_renderer.drawText(L"Select files or folders from the repository\nto download", D2D1::RectF(selX + 20, midY + 32, selX + selCardW - 20, midY + 70),
            subFmt.Get(), Colors::TextMuted(), DWRITE_TEXT_ALIGNMENT_CENTER);
    } else {
        m_logPanel.setRect(selX + 16, curY + 54, selCardW - 32, splitH - 70);
        m_logPanel.render(m_renderer);
    }

    curY += splitH + 16.0f;

    // 5. Download Action Button Bar
    float actionBtnH = 46.0f;
    m_btnDownload.setRect(Layout::Padding, curY, contentW, actionBtnH);
    m_btnDownload.setText(selCount > 0 ? std::format(L"Download {} Selected Files", selCount) : L"Download Selected Files");
    m_btnDownload.render(m_renderer);

    // 6. Status Bar (Bottom)
    float statusY = h - Layout::StatusbarHeight;
    m_renderer.drawFilledRectangle(D2D1::RectF(0, statusY, w, h), Colors::TitleBar());
    m_renderer.drawLine(D2D1::Point2F(0, statusY), D2D1::Point2F(w, statusY), Colors::Border(), 1.0f);

    // Pulse dot
    D2D1_COLOR_F dotColor = { 0.231f, 0.510f, 0.965f, 1.0f };
    m_renderer.drawFilledEllipse(D2D1::Point2F(Layout::Padding + 6, statusY + Layout::StatusbarHeight / 2), 4.5f, 4.5f, dotColor);

    std::wstring statusFull = std::format(L"Ready  •  Loaded 9 files (721.2 KB)");
    m_lblStatus.setText(statusFull);
    m_lblStatus.setRect(Layout::Padding + 18, statusY + 4, w - Layout::Padding * 2, statusY + Layout::StatusbarHeight);
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
    float w = sz.width;
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
        std::wstring defBranch;
        auto tree = m_githubApi.getRepoTree(owner, name, branch, defBranch);
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
