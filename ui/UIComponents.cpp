#include "pch.h"
#include "ui/UIComponents.h"
#include "utils/StringHelper.h"
#include "filesystem/FilesystemHelper.h"

// ===== UIButton =====

UIButton::UIButton(float x, float y, float w, float h, std::wstring text)
    : m_x(x), m_y(y), m_w(w), m_h(h), m_text(std::move(text)),
      m_bgColor(AnimatedColor(Colors::Primary())) {}

void UIButton::render(D2DRenderer& r) {
    m_bgColor.update(0.016f);
    D2D1_COLOR_F bg = m_bgColor.getColor();
    if (!m_enabled) bg.a = 0.4f;
    else if (m_pressed) bg = Colors::PrimaryPressed();
    else if (m_hovered) bg = Colors::PrimaryHover();

    auto rect = getRect();
    r.drawFilledRoundedRectangle(rect, bg, Layout::CornerRadius);

    if (!m_text.empty()) {
        auto fmt = r.createTextFormat(12.5f);
        r.drawText(m_text, rect, fmt.Get(), Colors::TextPrimary(), DWRITE_TEXT_ALIGNMENT_CENTER);
    }
}

bool UIButton::handleMouseDown(int x, int y) {
    if (hitTest(x, y) && m_enabled) { m_pressed = true; return true; }
    return false;
}

bool UIButton::handleMouseUp(int, int) {
    if (m_pressed) { m_pressed = false; return true; }
    return false;
}

void UIButton::handleMouseMove(int x, int y) { m_hovered = hitTest(x, y); }

bool UIButton::hitTest(int x, int y) const {
    return x >= m_x && x <= m_x + m_w && y >= m_y && y <= m_y + m_h;
}

bool UIButton::wasClicked(int, int) {
    if (m_clicked) { m_clicked = false; return true; }
    return false;
}

D2D1_RECT_F UIButton::getRect() const { return D2D1::RectF(m_x, m_y, m_x + m_w, m_y + m_h); }

// ===== UIInput =====

UIInput::UIInput(float x, float y, float w, float h, std::wstring placeholder)
    : m_x(x), m_y(y), m_w(w), m_h(h), m_placeholder(std::move(placeholder)) {}

void UIInput::render(D2DRenderer& r) {
    m_focusAnim.update(0.016f);
    float focusVal = m_focusAnim.getValue();

    auto rect = getRect();

    // Background
    D2D1_COLOR_F bg = Colors::InputBg();
    if (m_hovered && !m_focused) {
        bg = D2D1::ColorF(0.09f, 0.12f, 0.19f, 1.0f);
    }
    r.drawFilledRoundedRectangle(rect, bg, Layout::CornerRadius);

    // HTML-style Focus Glow Ring (Outer halo)
    if (focusVal > 0.01f) {
        D2D1_COLOR_F glowColor = Colors::Primary();
        glowColor.a = 0.25f * focusVal;
        D2D1_RECT_F glowRect = D2D1::RectF(m_x - 3.0f * focusVal, m_y - 3.0f * focusVal,
                                           m_x + m_w + 3.0f * focusVal, m_y + m_h + 3.0f * focusVal);
        r.drawRoundedRectangle(glowRect, glowColor, Layout::CornerRadius + 3.0f, 2.0f);
    }

    // Animated Border
    D2D1_COLOR_F border = Colors::Border();
    if (focusVal > 0.01f) {
        border.r = border.r + (Colors::BorderFocus().r - border.r) * focusVal;
        border.g = border.g + (Colors::BorderFocus().g - border.g) * focusVal;
        border.b = border.b + (Colors::BorderFocus().b - border.b) * focusVal;
        border.a = border.a + (Colors::BorderFocus().a - border.a) * focusVal;
    } else if (m_hovered) {
        border = D2D1::ColorF(0.55f, 0.36f, 0.96f, 0.45f);
    }
    r.drawRoundedRectangle(rect, border, Layout::CornerRadius, focusVal > 0.01f ? 1.5f : 1.0f);

    float textPad = 12.0f;
    D2D1_RECT_F textRect = D2D1::RectF(m_x + textPad, m_y, m_x + m_w - textPad, m_y + m_h);
    auto fmt = r.createTextFormat(13.0f, L"Segoe UI");

    if (m_text.empty() && !m_focused) {
        r.drawText(m_placeholder, textRect, fmt.Get(), Colors::TextMuted());
    } else {
        r.drawTextClipped(m_text, textRect, fmt.Get(), Colors::TextPrimary());
    }

    // Caret Cursor (Breathing smooth fade)
    if (m_focused && fmt) {
        std::wstring beforeCursor = m_text.substr(0, m_cursor);
        float cursorOffset = 0.0f;
        if (!beforeCursor.empty() && r.getDWrite()) {
            Microsoft::WRL::ComPtr<IDWriteTextLayout> layout;
            r.getDWrite()->CreateTextLayout(beforeCursor.c_str(), (UINT32)beforeCursor.size(),
                fmt.Get(), 10000.0f, m_h, layout.GetAddressOf());
            if (layout) {
                DWRITE_TEXT_METRICS metrics;
                layout->GetMetrics(&metrics);
                cursorOffset = metrics.width;
            }
        }

        float cursorX = m_x + textPad + cursorOffset;

        auto now = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        float alpha = 0.5f + 0.5f * std::sin(ms * 0.007f);
        D2D1_COLOR_F cursorColor = Colors::Accent();
        cursorColor.a = alpha;

        if (cursorX <= m_x + m_w - textPad) {
            r.drawLine(D2D1::Point2F(cursorX, m_y + 8), D2D1::Point2F(cursorX, m_y + m_h - 8),
                cursorColor, 1.8f);
        }
    }
}

bool UIInput::handleMouseDown(int x, int y) {
    if (hitTest(x, y)) {
        if (!m_focused) {
            m_focused = true;
            m_focusAnim.animateTo(1.0f, 0.2f);
        }
        return true;
    }
    if (m_focused) {
        m_focused = false;
        m_focusAnim.animateTo(0.0f, 0.2f);
    }
    return false;
}

void UIInput::handleKeyDown(WCHAR key, bool ctrl) {
    if (key == VK_LEFT) { if (m_cursor > 0) m_cursor--; }
    else if (key == VK_RIGHT) { if (m_cursor < (int)m_text.size()) m_cursor++; }
    else if (key == VK_HOME) { m_cursor = 0; }
    else if (key == VK_END) { m_cursor = (int)m_text.size(); }
    else if (key == VK_BACK) { if (m_cursor > 0) { m_text.erase(--m_cursor, 1); } }
    else if (key == VK_DELETE) { if (m_cursor < (int)m_text.size()) m_text.erase(m_cursor, 1); }
    else if (key == 0x0D) { /* Enter handled externally */ }
    else if (ctrl && key == 'A') { m_cursor = (int)m_text.size(); }
    else if (ctrl && key == 'V') {
        if (IsClipboardFormatAvailable(CF_UNICODETEXT) && OpenClipboard(nullptr)) {
            HANDLE h = GetClipboardData(CF_UNICODETEXT);
            if (h) {
                wchar_t* text = (wchar_t*)GlobalLock(h);
                if (text) {
                    m_text.insert(m_cursor, text);
                    m_cursor += (int)wcslen(text);
                    GlobalUnlock(h);
                }
            }
            CloseClipboard();
        }
    }
    else if (ctrl && key == 'C') {
        if (!m_text.empty() && OpenClipboard(nullptr)) {
            EmptyClipboard();
            size_t sz = (m_text.size() + 1) * sizeof(wchar_t);
            HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, sz);
            memcpy(GlobalLock(h), m_text.c_str(), sz);
            GlobalUnlock(h);
            SetClipboardData(CF_UNICODETEXT, h);
            CloseClipboard();
        }
    }
    else if (key == VK_ESCAPE) { m_focused = false; }
    else if (key >= 32 && key < 127) { m_text.insert(m_cursor, 1, (wchar_t)key); m_cursor++; }
}

bool UIInput::hitTest(int x, int y) const {
    return x >= m_x && x <= m_x + m_w && y >= m_y && y <= m_y + m_h;
}

D2D1_RECT_F UIInput::getRect() const { return D2D1::RectF(m_x, m_y, m_x + m_w, m_y + m_h); }

// ===== UICheckbox =====

UICheckbox::UICheckbox(float x, float y, float size, std::wstring label, bool checked)
    : m_x(x), m_y(y), m_size(size), m_label(std::move(label)), m_checked(checked),
      m_checkAnim(AnimatedFloat(checked ? 1.0f : 0.0f)) {}

void UICheckbox::render(D2DRenderer& r) {
    m_checkAnim.update(0.016f);
    float anim = m_checkAnim.getValue();
    D2D1_RECT_F boxRect = D2D1::RectF(m_x, m_y, m_x + m_size, m_y + m_size);

    D2D1_COLOR_F bgColor = m_checked ? Colors::CheckBox() : Colors::InputBg();
    D2D1_COLOR_F borderColor = m_hovered ? Colors::BorderFocus() : Colors::Border();
    r.drawFilledRoundedRectangle(boxRect, bgColor, 3.0f);
    r.drawRoundedRectangle(boxRect, borderColor, 3.0f, 1.0f);

    if (anim > 0.1f) {
        float checkSize = m_size * 0.4f;
        D2D1_COLOR_F white = Colors::TextPrimary();
        white.a = anim;
        float cx = m_x + m_size / 2;
        float cy = m_y + m_size / 2;
        float s = checkSize * anim;
        r.drawLine(D2D1::Point2F(cx - s * 0.6f, cy), D2D1::Point2F(cx - s * 0.1f, cy + s * 0.5f), white, 2.0f);
        r.drawLine(D2D1::Point2F(cx - s * 0.1f, cy + s * 0.5f), D2D1::Point2F(cx + s * 0.6f, cy - s * 0.5f), white, 2.0f);
    }

    if (!m_label.empty()) {
        auto fmt = r.createTextFormat(12.0f);
        D2D1_RECT_F labelRect = D2D1::RectF(m_x + m_size + 8, m_y, m_x + m_size + 300, m_y + m_size);
        r.drawText(m_label, labelRect, fmt.Get(), Colors::TextSecondary());
    }
}

bool UICheckbox::handleMouseDown(int x, int y) {
    if (hitTest(x, y)) {
        m_checked = !m_checked;
        m_checkAnim.animateTo(m_checked ? 1.0f : 0.0f, 0.2f);
        m_toggled = true;
        return true;
    }
    return false;
}

bool UICheckbox::hitTest(int x, int y) const {
    return x >= m_x && x <= m_x + m_size + 8 + 200 && y >= m_y && y <= m_y + m_size;
}


D2D1_RECT_F UICheckbox::getRect() const { return D2D1::RectF(m_x, m_y, m_x + m_size, m_y + m_size); }

// ===== UIProgressBar =====

UIProgressBar::UIProgressBar(float x, float y, float w, float h)
    : m_x(x), m_y(y), m_w(w), m_h(h) {}

void UIProgressBar::render(D2DRenderer& r) {
    m_animProgress.update(0.016f);
    D2D1_RECT_F trackRect = D2D1::RectF(m_x, m_y, m_x + m_w, m_y + m_h);
    r.drawFilledRoundedRectangle(trackRect, Colors::ProgressTrack(), m_h / 2);

    float progress = m_animProgress.getValue();
    if (progress > 0.001f) {
        float pw = std::max(m_h, m_w * progress);
        D2D1_RECT_F fillRect = D2D1::RectF(m_x, m_y, m_x + pw, m_y + m_h);
        r.drawFilledRoundedRectangle(fillRect, Colors::ProgressBar(), m_h / 2);
    }
}

// ===== UITextLabel =====

UITextLabel::UITextLabel(float x, float y, float w, float h, std::wstring text, float fontSize, D2D1_COLOR_F color)
    : m_x(x), m_y(y), m_w(w), m_h(h), m_text(std::move(text)), m_fontSize(fontSize), m_color(color) {}

void UITextLabel::render(D2DRenderer& r, IDWriteTextFormat* fmt) {
    if (m_text.empty()) return;
    auto rect = getRect();
    auto myFmt = fmt ? nullptr : r.createTextFormat(m_fontSize);
    r.drawText(m_text, rect, fmt ? fmt : myFmt.Get(), m_color);
}

D2D1_RECT_F UITextLabel::getRect() const { return D2D1::RectF(m_x, m_y, m_x + m_w, m_y + m_h); }

// ===== UITreeView =====

UITreeView::UITreeView(float x, float y, float w, float h)
    : m_x(x), m_y(y), m_w(w), m_h(h) {}

void UITreeView::render(D2DRenderer& r) {
    auto clipRect = D2D1::RectF(m_x, m_y, m_x + m_w, m_y + m_h);
    r.drawFilledRoundedRectangle(clipRect, Colors::Surface(), Layout::CornerRadius);
    r.drawRoundedRectangle(clipRect, Colors::Border(), Layout::CornerRadius, 1.0f);

    r.pushClipRect(D2D1::RectF(m_x + 1, m_y + 1, m_x + m_w - 1, m_y + m_h - 1));
    float yOffset = m_y - m_scrollY;
    if (m_root) {
        drawItem(r, m_root, 0, yOffset);
    }
    r.popClipRect();
}

void UITreeView::drawItem(D2DRenderer& r, const std::shared_ptr<FileNode>& node, int depth, float& yOffset) {
    if (yOffset > m_y + m_h + m_itemHeight) return;
    if (yOffset + m_itemHeight < m_y - m_itemHeight) {
        yOffset += m_itemHeight;
        if (node->isDir && node->expanded) {
            for (auto& child : node->children) drawItem(r, child, depth + 1, yOffset);
        }
        return;
    }

    float indent = depth * m_indentSize;
    float itemX = m_x + 8 + indent;
    D2D1_RECT_F itemRect = D2D1::RectF(m_x, yOffset, m_x + m_w, yOffset + m_itemHeight);

    bool hovered = m_hoveredIndex >= 0 && m_flatList.size() > (size_t)m_hoveredIndex &&
                   m_flatList[m_hoveredIndex].first == node;
    if (hovered) {
        r.drawFilledRectangle(D2D1::RectF(m_x + 2, yOffset, m_x + m_w - 2, yOffset + m_itemHeight),
            Colors::SurfaceHover());
    } else if (node->selected) {
        D2D1_COLOR_F selBg = { 0.388f, 0.400f, 0.945f, 0.08f };
        r.drawFilledRectangle(D2D1::RectF(m_x + 2, yOffset, m_x + m_w - 2, yOffset + m_itemHeight), selBg);
    }

    float currX = itemX;
    float centerY = yOffset + m_itemHeight / 2;

    // Chevron arrow for directories
    if (node->isDir) {
        if (node->expanded) {
            r.drawLine(D2D1::Point2F(currX, centerY - 2), D2D1::Point2F(currX + 4, centerY + 2), Colors::TextMuted(), 1.2f);
            r.drawLine(D2D1::Point2F(currX + 4, centerY + 2), D2D1::Point2F(currX + 8, centerY - 2), Colors::TextMuted(), 1.2f);
        } else {
            r.drawLine(D2D1::Point2F(currX + 2, centerY - 4), D2D1::Point2F(currX + 6, centerY), Colors::TextMuted(), 1.2f);
            r.drawLine(D2D1::Point2F(currX + 6, centerY), D2D1::Point2F(currX + 2, centerY + 4), Colors::TextMuted(), 1.2f);
        }
        currX += 14;
    } else {
        currX += 14;
    }

    // Checkbox
    float cbSize = 14.0f;
    D2D1_RECT_F cbRect = D2D1::RectF(currX, centerY - cbSize / 2, currX + cbSize, centerY + cbSize / 2);
    D2D1_COLOR_F cbBg = node->selected ? Colors::Primary() : Colors::InputBg();
    D2D1_COLOR_F cbBorder = node->selected ? Colors::Primary() : Colors::Border();
    r.drawFilledRoundedRectangle(cbRect, cbBg, 3.0f);
    r.drawRoundedRectangle(cbRect, cbBorder, 3.0f, 1.0f);

    if (node->selected) {
        // Checkmark
        r.drawLine(D2D1::Point2F(currX + 3, centerY), D2D1::Point2F(currX + 6, centerY + 3), Colors::TextPrimary(), 1.5f);
        r.drawLine(D2D1::Point2F(currX + 6, centerY + 3), D2D1::Point2F(currX + 11, centerY - 3), Colors::TextPrimary(), 1.5f);
    }
    currX += cbSize + 8;

    // Icon (Vector Folder Icon vs Extension Specific File Icons)
    if (node->isDir) {
        D2D1_COLOR_F folderTab = { 0.753f, 0.518f, 0.988f, 1.0f }; // #c084fc (Light purple tab)
        D2D1_COLOR_F folderBody = Colors::FolderIcon();            // #a855f7 (Purple body)

        // Folder top tab
        D2D1_RECT_F tabRect = D2D1::RectF(currX, centerY - 6, currX + 6, centerY - 2);
        r.drawFilledRoundedRectangle(tabRect, folderTab, 1.5f);

        // Folder main body
        D2D1_RECT_F folderRect = D2D1::RectF(currX, centerY - 3, currX + 15, centerY + 6);
        r.drawFilledRoundedRectangle(folderRect, folderBody, 2.0f);

        // Front flap crease line
        r.drawLine(D2D1::Point2F(currX + 1, centerY - 1), D2D1::Point2F(currX + 14, centerY - 1), folderTab, 1.0f);
        currX += 18;
    } else {
        std::wstring nameLower = node->name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::towlower);

        D2D1_RECT_F iconBox = D2D1::RectF(currX, centerY - 7, currX + 16, centerY + 7);

        if (nameLower.ends_with(L".html") || nameLower.ends_with(L".htm")) {
            D2D1_COLOR_F htmlColor = { 0.976f, 0.451f, 0.086f, 1.0f }; // #f97316 (Orange)
            auto codeFmt = r.createTextFormat(10.0f, L"Consolas", DWRITE_FONT_WEIGHT_BOLD);
            r.drawText(L"<>", iconBox, codeFmt.Get(), htmlColor);
        } else if (nameLower.ends_with(L".css")) {
            D2D1_COLOR_F cssColor = { 0.220f, 0.741f, 0.973f, 1.0f }; // #38bdf8 (Cyan/Blue)
            auto codeFmt = r.createTextFormat(10.0f, L"Consolas", DWRITE_FONT_WEIGHT_BOLD);
            r.drawText(L"#", iconBox, codeFmt.Get(), cssColor);
        } else if (nameLower.ends_with(L".js") || nameLower.ends_with(L".jsx") || nameLower.ends_with(L".ts") || nameLower.ends_with(L".tsx")) {
            D2D1_COLOR_F jsColor = { 0.918f, 0.702f, 0.031f, 1.0f }; // #eab308 (Yellow)
            auto codeFmt = r.createTextFormat(9.0f, L"Consolas", DWRITE_FONT_WEIGHT_BOLD);
            r.drawText(L"JS", iconBox, codeFmt.Get(), jsColor);
        } else if (nameLower.ends_with(L".jpg") || nameLower.ends_with(L".jpeg") || nameLower.ends_with(L".png") ||
                   nameLower.ends_with(L".gif") || nameLower.ends_with(L".svg") || nameLower.ends_with(L".webp") || nameLower.ends_with(L".ico")) {
            D2D1_COLOR_F imgColor = { 0.204f, 0.827f, 0.600f, 1.0f }; // #34d399 (Emerald Green)
            r.drawRoundedRectangle(D2D1::RectF(currX + 1, centerY - 6, currX + 15, centerY + 6), imgColor, 2.0f, 1.2f);
            r.drawFilledEllipse(D2D1::Point2F(currX + 5, centerY - 2), 1.5f, 1.5f, imgColor);
            r.drawLine(D2D1::Point2F(currX + 3, centerY + 4), D2D1::Point2F(currX + 8, centerY - 1), imgColor, 1.2f);
            r.drawLine(D2D1::Point2F(currX + 8, centerY - 1), D2D1::Point2F(currX + 13, centerY + 4), imgColor, 1.2f);
        } else if (nameLower.ends_with(L".json") || nameLower.ends_with(L".yml") || nameLower.ends_with(L".yaml")) {
            D2D1_COLOR_F jsonColor = { 0.753f, 0.518f, 0.988f, 1.0f }; // #c084fc (Purple)
            auto codeFmt = r.createTextFormat(10.0f, L"Consolas", DWRITE_FONT_WEIGHT_BOLD);
            r.drawText(L"{}", iconBox, codeFmt.Get(), jsonColor);
        } else if (nameLower.ends_with(L".cpp") || nameLower.ends_with(L".h") || nameLower.ends_with(L".hpp") || nameLower.ends_with(L".c")) {
            D2D1_COLOR_F cppColor = { 0.376f, 0.647f, 0.980f, 1.0f }; // #60a5fa (Blue)
            auto codeFmt = r.createTextFormat(9.0f, L"Consolas", DWRITE_FONT_WEIGHT_BOLD);
            r.drawText(L"C++", iconBox, codeFmt.Get(), cppColor);
        } else if (nameLower.ends_with(L".md") || nameLower.ends_with(L".txt")) {
            D2D1_COLOR_F mdColor = { 0.796f, 0.835f, 0.882f, 1.0f }; // #cbd5e1 (Slate)
            auto codeFmt = r.createTextFormat(9.0f, L"Consolas", DWRITE_FONT_WEIGHT_BOLD);
            r.drawText(L"MD", iconBox, codeFmt.Get(), mdColor);
        } else {
            D2D1_COLOR_F fileColor = Colors::FileIcon();
            D2D1_RECT_F fileRect = D2D1::RectF(currX + 2, centerY - 6, currX + 12, centerY + 6);
            r.drawFilledRoundedRectangle(fileRect, fileColor, 1.5f);
        }
        currX += 18;
    }

    // Name label
    auto fmt = r.createTextFormat(12.0f);
    D2D1_RECT_F textRect = D2D1::RectF(currX, yOffset, m_x + m_w - 85, yOffset + m_itemHeight);
    r.drawTextClipped(node->name, textRect, fmt.Get(),
        node->isDir ? Colors::TextPrimary() : Colors::TextPrimary());

    // Size label (Right aligned)
    if (!node->isDir && node->size > 0) {
        auto sizeFmt = r.createTextFormat(11.0f);
        D2D1_RECT_F sizeRect = D2D1::RectF(m_x + m_w - 80, yOffset, m_x + m_w - 10, yOffset + m_itemHeight);
        r.drawText(Str::formatBytes(node->size), sizeRect, sizeFmt.Get(), Colors::TextMuted(), DWRITE_TEXT_ALIGNMENT_TRAILING);
    }

    yOffset += m_itemHeight;

    if (node->isDir && node->expanded) {
        for (auto& child : node->children) {
            drawItem(r, child, depth + 1, yOffset);
        }
    }
}

bool UITreeView::handleMouseDown(int x, int y) {
    if (!hitTest(x, y)) return false;

    rebuildFlatList();
    float clickY = y - m_y + m_scrollY;
    int index = (int)(clickY / m_itemHeight);
    if (index < 0 || index >= (int)m_flatList.size()) return false;

    auto& [node, depth] = m_flatList[index];
    float itemLeft = 8.0f + depth * m_indentSize;
    float clickRelX = x - m_x;

    if (node->isDir && clickRelX < itemLeft + 24) {
        node->expanded = !node->expanded;
        rebuildFlatList();
        return true;
    }

    if (!node->isDir || clickRelX >= itemLeft + 24) {
        node->selected = !node->selected;
        return true;
    }
    return false;
}

void UITreeView::handleMouseMove(int x, int y) {
    if (!hitTest(x, y)) { m_hoveredIndex = -1; return; }
    rebuildFlatList();
    float clickY = y - m_y + m_scrollY;
    int index = (int)(clickY / m_itemHeight);
    m_hoveredIndex = (index >= 0 && index < (int)m_flatList.size()) ? index : -1;
}

void UITreeView::handleWheel(int delta) {
    int maxScroll = getMaxScroll();
    m_scrollY = std::max(0.0f, std::min(m_scrollY - delta * 3.0f, (float)maxScroll));
}

void UITreeView::rebuildFlatList() {
    m_flatList.clear();
    if (!m_root) return;
    std::function<void(std::shared_ptr<FileNode>, int)> flatten = [&](std::shared_ptr<FileNode> n, int d) {
        m_flatList.push_back({ n, d });
        if (n->isDir && n->expanded) {
            for (auto& c : n->children) flatten(c, d + 1);
        }
    };
    flatten(m_root, 0);
}

int UITreeView::getSelectedCount() const {
    int count = 0;
    if (!m_root) return 0;
    std::function<void(std::shared_ptr<FileNode>)> countSel = [&](std::shared_ptr<FileNode> n) {
        if (!n->isDir && n->selected) count++;
        for (auto& c : n->children) countSel(c);
    };
    countSel(m_root);
    return count;
}

std::vector<std::wstring> UITreeView::getSelectedPaths() const {
    std::vector<std::wstring> paths;
    if (!m_root) return paths;
    std::function<void(std::shared_ptr<FileNode>)> collect = [&](std::shared_ptr<FileNode> n) {
        if (!n->isDir && n->selected) paths.push_back(n->path);
        for (auto& c : n->children) collect(c);
    };
    collect(m_root);
    return paths;
}

bool UITreeView::hitTest(int x, int y) const {
    return x >= m_x && x <= m_x + m_w && y >= m_y && y <= m_y + m_h;
}

D2D1_RECT_F UITreeView::getItemRect(int index) const {
    float y = m_y + index * m_itemHeight - m_scrollY;
    return D2D1::RectF(m_x, y, m_x + m_w, y + m_itemHeight);
}

int UITreeView::getMaxScroll() const {
    return std::max(0, (int)(m_flatList.size() * m_itemHeight) - (int)m_h + 20);
}

// ===== UILogPanel =====

UILogPanel::UILogPanel(float x, float y, float w, float h)
    : m_x(x), m_y(y), m_w(w), m_h(h) {}

void UILogPanel::render(D2DRenderer& r) {
    auto rect = D2D1::RectF(m_x, m_y, m_x + m_w, m_y + m_h);
    r.drawFilledRoundedRectangle(rect, Colors::Surface(), Layout::CornerRadius);
    r.drawRoundedRectangle(rect, Colors::Border(), Layout::CornerRadius, 1.0f);

    r.pushClipRect(D2D1::RectF(m_x + 1, m_y + 1, m_x + m_w - 1, m_y + m_h - 1));

    float yOffset = m_y + 4 - m_scrollY;
    auto fmt = r.createMonoTextFormat(11.0f);

    for (auto& entry : m_entries) {
        if (yOffset > m_y + m_h + m_itemHeight) break;
        if (yOffset + m_itemHeight >= m_y) {
            D2D1_COLOR_F color = Colors::TextMuted();
            switch (entry.level) {
                case LogLevel::Info:    color = Colors::TextSecondary(); break;
                case LogLevel::Warning: color = Colors::Warning(); break;
                case LogLevel::Error:   color = Colors::Error(); break;
                case LogLevel::Debug:   color = Colors::TextMuted(); break;
            }

            auto tp = entry.timestamp;
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
            int totalSec = (int)(ms / 1000);
            int h = (totalSec / 3600) % 24;
            int m = (totalSec / 60) % 60;
            int s = totalSec % 60;
            std::wstring prefix = std::format(L"[{:02}:{:02}:{:02}] ", h, m, s);

            D2D1_RECT_F textRect = D2D1::RectF(m_x + 10, yOffset, m_x + m_w - 10, yOffset + m_itemHeight);
            r.drawText(prefix + entry.message, textRect, fmt.Get(), color);
        }
        yOffset += m_itemHeight;
    }
    r.popClipRect();
}

void UILogPanel::addEntry(const LogEntry& entry) {
    m_entries.push_back(entry);
    if ((int)m_entries.size() > MAX_ENTRIES) m_entries.erase(m_entries.begin());
    int maxScroll = std::max(0, (int)m_entries.size() * (int)m_itemHeight - (int)m_h + 20);
    m_scrollY = (float)maxScroll;
}

void UILogPanel::handleWheel(int delta) {
    int maxScroll = std::max(0, (int)m_entries.size() * (int)m_itemHeight - (int)m_h + 20);
    m_scrollY = std::max(0.0f, std::min(m_scrollY - delta * 3.0f, (float)maxScroll));
}

bool UILogPanel::hitTest(int x, int y) const {
    return x >= m_x && x <= m_x + m_w && y >= m_y && y <= m_y + m_h;
}
