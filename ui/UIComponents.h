#pragma once
#include "pch.h"

// Base UI Component class
class UIComponent {
public:
    UIComponent(float x, float y, float width, float height);
    virtual ~UIComponent() = default;

    // Position and size
    virtual void setPosition(float x, float y);
    virtual void setSize(float width, float height);
    D2D1_RECT_F getBounds() const;

    // State
    virtual void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }
    virtual void setVisible(bool visible);
    bool isVisible() const { return m_visible; }

    // Events
    virtual void onMouseEnter() {}
    virtual void onMouseLeave() {}
    virtual void onMouseDown(int x, int y) {}
    virtual void onMouseUp(int x, int y) {}
    virtual void onMouseMove(int x, int y) {}
    virtual void onClick() {}

    // Rendering
    virtual void render(D2DRenderer& renderer) = 0;

    // Hit testing
    bool hitTest(int x, int y) const;

protected:
    float m_x, m_y;
    float m_width, m_height;
    bool m_enabled = true;
    bool m_visible = true;
    bool m_hovered = false;
};

// Button Component
class Button : public UIComponent {
public:
    Button(float x, float y, float width, float height, std::wstring text);

    void setText(std::wstring text) { m_text = std::move(text); }
    void setOnClick(std::function<void()> callback) { m_onClick = std::move(callback); }

    // Events
    void onMouseEnter() override;
    void onMouseLeave() override;
    void onMouseDown(int x, int y) override;
    void onMouseUp(int x, int y) override;
    void onClick() override;

    // Rendering
    void render(D2DRenderer& renderer) override;

private:
    std::wstring m_text;
    std::function<void()> m_onClick;
    bool m_pressed = false;

    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
};

// Input Box Component
class InputBox : public UIComponent {
public:
    InputBox(float x, float y, float width, float height, std::wstring placeholder = L"");

    void setText(std::wstring text) { m_text = std::move(text); }
    std::wstring getText() const { return m_text; }
    void setPlaceholder(std::wstring placeholder) { m_placeholder = std::move(placeholder); }

    // Events
    void onMouseDown(int x, int y) override;
    void onKeyDown(WCHAR key);

    // Rendering
    void render(D2DRenderer& renderer) override;

    void setFocused(bool focused) { m_focused = focused; }
    bool isFocused() const { return m_focused; }

private:
    std::wstring m_text;
    std::wstring m_placeholder;
    bool m_focused = false;
    int m_cursorPos = 0;

    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
};

// Progress Bar Component
class ProgressBar : public UIComponent {
public:
    ProgressBar(float x, float y, float width, float height);

    void setProgress(double progress);
    void setColor(D2D1_COLOR_F color) { m_color = color; }

    // Rendering
    void render(D2DRenderer& renderer) override;

private:
    double m_progress = 0.0;
    D2D1_COLOR_F m_color = Colors::ProgressBar;
};

// Text Label Component
class TextLabel : public UIComponent {
public:
    TextLabel(float x, float y, float width, float height, std::wstring text);

    void setText(std::wstring text) { m_text = std::move(text); }
    void setColor(D2D1_COLOR_F color) { m_color = color; }
    void setFontSize(float size) { m_fontSize = size; }

    // Rendering
    void render(D2DRenderer& renderer) override;

private:
    std::wstring m_text;
    D2D1_COLOR_F m_color = Colors::TextPrimary;
    float m_fontSize = 14.0f;

    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
};

// Log Panel Component
class LogPanel : public UIComponent {
public:
    LogPanel(float x, float y, float width, float height);

    void addLogEntry(const LogEntry& entry);
    void clearLogs();

    // Rendering
    void render(D2DRenderer& renderer) override;

private:
    std::vector<LogEntry> m_logs;
    int m_maxVisibleLines = 50;
    float m_lineHeight = 20.0f;
    int m_scrollOffset = 0;

    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;

    D2D1_COLOR_F getLogLevelColor(LogLevel level) const;
};

// Card Component (for file items)
class Card : public UIComponent {
public:
    Card(float x, float y, float width, float height);

    void setContent(std::wstring title, std::wstring subtitle = L"", 
                    D2D1_COLOR_F accentColor = Colors::Primary);

    // Events
    void onMouseEnter() override;
    void onMouseLeave() override;

    // Rendering
    void render(D2DRenderer& renderer) override;

private:
    std::wstring m_title;
    std::wstring m_subtitle;
    D2D1_COLOR_F m_accentColor;
    D2D1_COLOR_F m_hoverColor = Colors::SurfaceLight;
};

// Checkbox Component
class Checkbox : public UIComponent {
public:
    Checkbox(float x, float y, float width, float height, std::wstring label, bool checked = false);

    void setChecked(bool checked) { m_checked = checked; }
    bool isChecked() const { return m_checked; }
    void setOnChanged(std::function<void(bool)> callback) { m_onChanged = std::move(callback); }

    // Events
    void onClick() override;

    // Rendering
    void render(D2DRenderer& renderer) override;

private:
    std::wstring m_label;
    bool m_checked = false;
    std::function<void(bool)> m_onChanged;
};
