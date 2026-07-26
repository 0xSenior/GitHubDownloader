#pragma once
#include "pch.h"

class D2DRenderer {
public:
    D2DRenderer();
    ~D2DRenderer();
    D2DRenderer(const D2DRenderer&) = delete;
    D2DRenderer& operator=(const D2DRenderer&) = delete;

    bool initialize(HWND hwnd);
    void shutdown();
    void beginDraw();
    void endDraw();
    void resize(UINT width, UINT height);

    void drawFilledRectangle(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color);
    void drawRoundedRectangle(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color, float radius, float strokeWidth = 1.0f);
    void drawFilledRoundedRectangle(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color, float radius);
    void drawEllipse(D2D1_POINT_2F center, float rx, float ry, const D2D1_COLOR_F& color);
    void drawFilledEllipse(D2D1_POINT_2F center, float rx, float ry, const D2D1_COLOR_F& color);
    void drawLine(D2D1_POINT_2F a, D2D1_POINT_2F b, const D2D1_COLOR_F& color, float width = 1.0f);
    void drawGradientRectangle(const D2D1_RECT_F& rect, const D2D1_COLOR_F& top, const D2D1_COLOR_F& bottom);
    void drawSvgPath(const std::string& pathData, const D2D1_COLOR_F& color, float scale, float offsetX, float offsetY);

    void drawText(std::wstring_view text, const D2D1_RECT_F& rect, IDWriteTextFormat* fmt, const D2D1_COLOR_F& color, DWRITE_TEXT_ALIGNMENT align = DWRITE_TEXT_ALIGNMENT_LEADING);
    void drawTextClipped(std::wstring_view text, const D2D1_RECT_F& rect, IDWriteTextFormat* fmt, const D2D1_COLOR_F& color);

    void pushClipRect(const D2D1_RECT_F& rect);
    void popClipRect();

    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> createBrush(const D2D1_COLOR_F& color);
    Microsoft::WRL::ComPtr<IDWriteTextFormat> createTextFormat(float fontSize, std::wstring fontFamily = L"Segoe UI", DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL);
    Microsoft::WRL::ComPtr<IDWriteTextFormat> createMonoTextFormat(float fontSize, DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL);

    ID2D1HwndRenderTarget* getRT() const { return m_rt.Get(); }
    ID2D1Factory* getFactory() const { return m_factory.Get(); }
    IDWriteFactory* getDWrite() const { return m_dwrite.Get(); }

    D2D1_SIZE_F getSize() const;

private:
    Microsoft::WRL::ComPtr<ID2D1Factory> m_factory;
    Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_rt;
    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwrite;
    bool m_initialized = false;
};
