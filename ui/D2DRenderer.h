#pragma once
#include "pch.h"

class D2DRenderer {
public:
    D2DRenderer();
    ~D2DRenderer();

    // Delete copy/move
    D2DRenderer(const D2DRenderer&) = delete;
    D2DRenderer& operator=(const D2DRenderer&) = delete;
    D2DRenderer(D2DRenderer&&) = delete;
    D2DRenderer& operator=(D2DRenderer&&) = delete;

    // Initialize for a window
    bool initialize(HWND hwnd);

    // Begin/End frame
    void beginDraw();
    void endDraw();

    // Resize
    void resize(UINT width, UINT height);

    // Drawing primitives
    void drawRectangle(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color, float strokeWidth = 1.0f);
    void drawFilledRectangle(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color);
    void drawRoundedRectangle(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color, float radiusX, float radiusY, float strokeWidth = 1.0f);
    void drawFilledRoundedRectangle(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color, float radiusX, float radiusY);
    void drawEllipse(D2D1_POINT_2F center, float radiusX, float radiusY, const D2D1_COLOR_F& color, float strokeWidth = 1.0f);
    void drawFilledEllipse(D2D1_POINT_2F center, float radiusX, float radiusY, const D2D1_COLOR_F& color);
    void drawLine(D2D1_POINT_2F start, D2D1_POINT_2F end, const D2D1_COLOR_F& color, float strokeWidth = 1.0f);

    // Gradient
    void drawGradientRectangle(const D2D1_RECT_F& rect, const D2D1_COLOR_F& topColor, const D2D1_COLOR_F& bottomColor);

    // Text
    void drawText(std::wstring_view text, const D2D1_RECT_F& rect, IDWriteTextFormat* textFormat, const D2D1_COLOR_F& color);
    void drawText(std::wstring_view text, D2D1_POINT_2F origin, IDWriteTextFormat* textFormat, const D2D1_COLOR_F& color);

    // Clip region
    void pushClipRect(const D2D1_RECT_F& rect);
    void popClipRect();

    // Getters
    ID2D1HwndRenderTarget* getRenderTarget() const { return m_renderTarget.Get(); }
    ID2D1Factory* getFactory() const { return m_factory.Get(); }
    IDWriteFactory* getDWriteFactory() const { return m_dwriteFactory.Get(); }

    // Factory methods for resources
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> createBrush(const D2D1_COLOR_F& color);

private:
    void createDeviceResources();
    void discardDeviceResources();

    Microsoft::WRL::ComPtr<ID2D1Factory> m_factory;
    Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_renderTarget;
    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;

    // Cached brushes
    std::unordered_map<D2D1_COLOR_F, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>, 
        struct ColorHash> m_brushCache;

    bool m_initialized = false;

    struct ColorHash {
        size_t operator()(const D2D1_COLOR_F& c) const {
            size_t h = 0;
            h ^= std::hash<float>{}(c.r) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<float>{}(c.g) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<float>{}(c.b) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<float>{}(c.a) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
};
