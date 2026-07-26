#pragma once
#include "pch.h"
#include "ui/D2DRenderer.h"
#include "ui/AnimManager.h"

struct UIEvent { int x, y; };

class UIButton {
public:
    UIButton(float x, float y, float w, float h, std::wstring text = L"");
    void render(D2DRenderer& r);
    bool handleMouseDown(int x, int y);
    bool handleMouseUp(int x, int y);
    void handleMouseMove(int x, int y);
    void setRect(float x, float y, float w, float h) { m_x = x; m_y = y; m_w = w; m_h = h; }
    void setText(std::wstring t) { m_text = std::move(t); }
    void setEnabled(bool e) { m_enabled = e; }
    void setStyle(D2D1_COLOR_F bg, D2D1_COLOR_F fg) { m_bgColor.setImmediate(bg); m_fgColor = fg; }
    bool hitTest(int x, int y) const;
    bool wasClicked(int x, int y);
    D2D1_RECT_F getRect() const;

private:
    float m_x, m_y, m_w, m_h;
    std::wstring m_text;
    bool m_enabled = true;
    bool m_hovered = false;
    bool m_pressed = false;
    AnimatedColor m_bgColor;
    D2D1_COLOR_F m_fgColor = Colors::TextPrimary();
    bool m_clicked = false;
};

class UIInput {
public:
    UIInput(float x, float y, float w, float h, std::wstring placeholder = L"");
    void render(D2DRenderer& r);
    bool handleMouseDown(int x, int y);
    void handleKeyDown(WCHAR key, bool ctrl = false);
    void setRect(float x, float y, float w, float h) { m_x = x; m_y = y; m_w = w; m_h = h; }
    void setPlaceholder(std::wstring t) { m_placeholder = std::move(t); }
    std::wstring getText() const { return m_text; }
    void setText(std::wstring t) { m_text = std::move(t); m_cursor = (int)m_text.size(); }
    void setFocused(bool f) { m_focused = f; }
    bool isFocused() const { return m_focused; }
    bool hitTest(int x, int y) const;
    D2D1_RECT_F getRect() const;

private:
    float m_x, m_y, m_w, m_h;
    std::wstring m_text;
    std::wstring m_placeholder;
    bool m_focused = false;
    bool m_hovered = false;
    int m_cursor = 0;
    AnimatedFloat m_focusAnim;
};

class UICheckbox {
public:
    UICheckbox(float x, float y, float size, std::wstring label = L"", bool checked = true);
    void render(D2DRenderer& r);
    bool handleMouseDown(int x, int y);
    bool isChecked() const { return m_checked; }
    void setChecked(bool c) { m_checked = c; }
    void setRect(float x, float y, float s) { m_x = x; m_y = y; m_size = s; }
    bool hitTest(int x, int y) const;
    bool wasToggled() { bool v = m_toggled; m_toggled = false; return v; }
    std::wstring getLabel() const { return m_label; }
    D2D1_RECT_F getRect() const;

private:
    float m_x, m_y, m_size;
    std::wstring m_label;
    bool m_checked;
    bool m_hovered = false;
    bool m_toggled = false;
    AnimatedFloat m_checkAnim;
};

class UIProgressBar {
public:
    UIProgressBar(float x, float y, float w, float h);
    void render(D2DRenderer& r);
    void setProgress(double p) { m_progress = p; m_animProgress.animateTo((float)p, 0.4f); }
    void setRect(float x, float y, float w, float h) { m_x = x; m_y = y; m_w = w; m_h = h; }
    void update(float dt) { m_animProgress.update(dt); }

private:
    float m_x, m_y, m_w, m_h;
    double m_progress = 0.0;
    AnimatedFloat m_animProgress;
};

class UITextLabel {
public:
    UITextLabel(float x, float y, float w, float h, std::wstring text = L"", float fontSize = 13.0f,
                D2D1_COLOR_F color = Colors::TextPrimary());
    void render(D2DRenderer& r, IDWriteTextFormat* fmt = nullptr);
    void setText(std::wstring t) { m_text = std::move(t); }
    void setColor(D2D1_COLOR_F c) { m_color = c; }
    void setRect(float x, float y, float w, float h) { m_x = x; m_y = y; m_w = w; m_h = h; }
    D2D1_RECT_F getRect() const;

private:
    float m_x, m_y, m_w, m_h;
    float m_fontSize;
    std::wstring m_text;
    D2D1_COLOR_F m_color;
};

class UITreeView {
public:
    UITreeView(float x, float y, float w, float h);
    void render(D2DRenderer& r);
    bool handleMouseDown(int x, int y);
    void handleMouseMove(int x, int y);
    void handleWheel(int delta);
    void setRect(float x, float y, float w, float h) { m_x = x; m_y = y; m_w = w; m_h = h; }
    void setRoot(std::shared_ptr<FileNode> root) { m_root = root; m_scrollY = 0; }
    std::shared_ptr<FileNode> getRoot() const { return m_root; }
    int getSelectedCount() const;
    std::vector<std::wstring> getSelectedPaths() const;
    bool hitTest(int x, int y) const;

private:
    float m_x, m_y, m_w, m_h;
    std::shared_ptr<FileNode> m_root;
    float m_scrollY = 0;
    float m_itemHeight = 26.0f;
    float m_indentSize = 18.0f;
    float m_checkboxSize = 16.0f;
    int m_hoveredIndex = -1;
    std::vector<std::pair<std::shared_ptr<FileNode>, int>> m_flatList;

    void rebuildFlatList();
    void drawItem(D2DRenderer& r, const std::shared_ptr<FileNode>& node, int depth, float& yOffset);
    D2D1_RECT_F getItemRect(int index) const;
    int getMaxScroll() const;
};

class UILogPanel {
public:
    UILogPanel(float x, float y, float w, float h);
    void render(D2DRenderer& r);
    void addEntry(const LogEntry& entry);
    void clear() { m_entries.clear(); m_scrollY = 0; }
    void setRect(float x, float y, float w, float h) { m_x = x; m_y = y; m_w = w; m_h = h; }
    void handleWheel(int delta);
    bool hitTest(int x, int y) const;

private:
    float m_x, m_y, m_w, m_h;
    std::vector<LogEntry> m_entries;
    float m_scrollY = 0;
    float m_itemHeight = 18.0f;
    static constexpr int MAX_ENTRIES = 200;
};
