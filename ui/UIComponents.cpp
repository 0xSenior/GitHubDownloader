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
        r.drawText(m_text, rect, fmt.Get(), Colors::TextPrimary(), D2D1_TEXT_ALIGNMENT_CENTER);
    }
}

bool UIButton::handleMouseDown(int x, int y) {
    if (hitTest(x, y) && m_enabled) { m_pressed = true; return true; }
    return false;
}

bool UIButton::handleMouseUp(int x, int y) {
    if (m_pressed) { m_pressed = false; return true; }
    return false;
}

void UIButton::handleMouseMove(int x, int y) { m_hovered = hitTest(x, y); }

bool UIButton::hitTest(int x, int y) const {
    return x >= m_x && x <= m_x + m_w && y >= m_y && y <= m_y + m_h;
}

bool UIButton::wasClicked(int x, int y) {
    if (m_clicked) { m_clicked = false; return true; }
    return false;
}

D2D1_RECT_F UIButton::getRect() const { return D2D1::RectF(m_x, m_y, m_x + m_w, m_y + m_h); }

// ===== UIInput =====

UIInput::UIInput(float x, float y, float w, float h, std::wstring placeholder)
    : m_x(x), m_y(y), m_w(w), m_h(h), m_placeholder(std::move(placeholder)) {}

void UIInput::render(D2DRenderer& r) {
    auto rect = getRect();
    D2D1_COLOR_F border = m_focused ? Colors::BorderFocus() : (m_hovered ? Colors::TextMuted() : Colors::Border());
    r.drawFilledRoundedRectangle(rect, Colors::InputBg(), Layout::CornerRadius);
    r.drawRoundedRectangle(rect, border, Layout::CornerRadius, m_focused ? 1.5f : 1.0f);

    if (m_focused) {
        float glow = 0.15f;
        D2D1_COLOR_F glowColor = Colors::Primary();
        glowColor.a = glow;
        D2D1_RECT_F glowRect = D2D1::RectF(m_x - 2, m_y - 2, m_x + m_w + 2, m_y + m_h + 2);
        r.drawRoundedRectangle(glowRect, glowColor, Layout::CornerRadius + 2, 2.0f);
    }

    float textPad = 12.0f;
    D2D1_RECT_F textRect = D2D1::RectF(m_x + textPad, m_y, m_x + m_w - textPad, m_y + m_h);

    if (m_text.empty() && !m_focused) {
        auto fmt = r.createTextFormat(13.0f);
        r.drawText(m_placeholder, textRect, fmt.Get(), Colors::TextMuted());
    } else {
        auto fmt = r.createTextFormat(13.0f);
        r.drawTextClipped(m_text, textRect, fmt.Get(), Colors::TextPrimary());
    }

    if (m_focused) {
        auto monoFmt = r.createMonoTextFormat(13.0f);
        std::wstring beforeCursor = m_text.substr(0, m_cursor);
        D2D1_SIZE_F sz;
        monoFmt->MeasureText(beforeCursor.c_str(), (UINT32)beforeCursor.size(), &sz);
        float cursorX = m_x + textPad + sz.width;
        D2D1_COLOR_F cursorColor = Colors::TextPrimary();
        auto now = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        if ((ms / 530) % 2 == 0) {
            r.drawLine(D2D1::Point2F(cursorX, m_y + 6), D2D1::Point2F(cursorX, m_y + m_h - 6),
                cursorColor, 1.5f);
        }
    }
}

bool UIInput::handleMouseDown(int x, int y) {
    if (hitTest(x, y)) { m_focused = true; return true; }
    m_focused = false;
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
    float boxCenterX = m_x + m_size / 2;
    float boxCenterY = m_y + m_size / 2;
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

bool UICheckbox::wasToggled() { bool v = m_toggled; m_toggled = false; return v; }

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
    }

    float iconX = itemX;
    float iconY = yOffset + m_itemHeight / 2;
    float iconR = 5.0f;

    if (node->isDir) {
        D2D1_COLOR_F folderColor = Colors::FolderIcon();
        r.drawFilledEllipse(D2D1::Point2F(iconX + 6, iconY), iconR, iconR * 0.7f, folderColor);
        float arrowX = itemX + 18;
        if (node->expanded) {
            r.drawLine(D2D1::Point2F(arrowX, iconY - 2), D2D1::Point2F(arrowX + 3, iconY + 2), Colors::TextMuted(), 1.0f);
            r.drawLine(D2D1::Point2F(arrowX + 3, iconY + 2), D2D1::Point2F(arrowX + 6, iconY - 2), Colors::TextMuted(), 1.0f);
        } else {
            r.drawLine(D2D1::Point2F(arrowX, iconY - 3), D2D1::Point2F(arrowX + 3, iconY), Colors::TextMuted(), 1.0f);
            r.drawLine(D2D1::Point2F(arrowX + 3, iconY), D2D1::Point2F(arrowX, iconY + 3), Colors::TextMuted(), 1.0f);
        }
    } else {
        D2D1_COLOR_F fileColor = Colors::FileIcon();
        D2D1_RECT_F fileIcon = D2D1::RectF(iconX + 2, iconY - 5, iconX + 10, iconY + 5);
        r.drawFilledRoundedRectangle(fileIcon, fileColor, 1.5f);
    }

    float textX = itemX + 26;
    auto fmt = r.createTextFormat(12.0f);
    D2D1_RECT_F textRect = D2D1::RectF(textX, yOffset, m_x + m_w - 80, yOffset + m_itemHeight);
    r.drawTextClipped(node->name, textRect, fmt.Get(),
        node->isDir ? Colors::TextPrimary() : Colors::TextSecondary());

    if (!node->isDir && node->size > 0) {
        auto sizeFmt = r.createTextFormat(11.0f, L"Cascadia Code");
        D2D1_RECT_F sizeRect = D2D1::RectF(m_x + m_w - 75, yOffset, m_x + m_w - 8, yOffset + m_itemHeight);
        r.drawText(Str::formatBytes(node->size), sizeRect, sizeFmt.Get(), Colors::TextMuted(), D2D1_TEXT_ALIGNMENT_TRAILING);
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
