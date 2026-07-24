#include "pch.h"
#include "D2DRenderer.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

D2DRenderer::D2DRenderer() = default;

D2DRenderer::~D2DRenderer() {
    discardDeviceResources();
}

bool D2DRenderer::initialize(HWND hwnd) {
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        m_factory.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(m_dwriteFactory.GetAddressOf())
    );
    if (FAILED(hr)) return false;

    RECT rc;
    GetClientRect(hwnd, &rc);

    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndRtProps = D2D1::HwndRenderTargetProperties(
        hwnd,
        D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)
    );

    hr = m_factory->CreateHwndRenderTarget(rtProps, hwndRtProps, m_renderTarget.GetAddressOf());
    if (FAILED(hr)) return false;

    m_renderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
    m_initialized = true;
    return true;
}

void D2DRenderer::beginDraw() {
    if (m_renderTarget) {
        m_renderTarget->BeginDraw();
        m_renderTarget->Clear(Colors::Background);
    }
}

void D2DRenderer::endDraw() {
    if (m_renderTarget) {
        HRESULT hr = m_renderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET) {
            discardDeviceResources();
            m_initialized = false;
        }
    }
}

void D2DRenderer::resize(UINT width, UINT height) {
    if (m_renderTarget) {
        m_renderTarget->Resize(D2D1::SizeU(width, height));
    }
}

Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> D2DRenderer::createBrush(const D2D1_COLOR_F& color) {
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
    if (m_renderTarget) {
        m_renderTarget->CreateColorBrush(color, brush.GetAddressOf());
    }
    return brush;
}

void D2DRenderer::drawRectangle(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color, float strokeWidth) {
    auto brush = createBrush(color);
    if (brush) {
        m_renderTarget->DrawRectangle(rect, brush.Get(), strokeWidth);
    }
}

void D2DRenderer::drawFilledRectangle(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color) {
    auto brush = createBrush(color);
    if (brush) {
        m_renderTarget->FillRectangle(rect, brush.Get());
    }
}

void D2DRenderer::drawRoundedRectangle(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color, 
    float radiusX, float radiusY, float strokeWidth) {
    auto brush = createBrush(color);
    if (brush) {
        D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(rect, radiusX, radiusY);
        m_renderTarget->DrawRoundedRectangle(roundedRect, brush.Get(), strokeWidth);
    }
}

void D2DRenderer::drawFilledRoundedRectangle(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color, 
    float radiusX, float radiusY) {
    auto brush = createBrush(color);
    if (brush) {
        D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(rect, radiusX, radiusY);
        m_renderTarget->FillRoundedRectangle(roundedRect, brush.Get());
    }
}

void D2DRenderer::drawEllipse(D2D1_POINT_2F center, float radiusX, float radiusY, 
    const D2D1_COLOR_F& color, float strokeWidth) {
    auto brush = createBrush(color);
    if (brush) {
        D2D1_ELLIPSE ellipse = D2D1::Ellipse(center, radiusX, radiusY);
        m_renderTarget->DrawEllipse(ellipse, brush.Get(), strokeWidth);
    }
}

void D2DRenderer::drawFilledEllipse(D2D1_POINT_2F center, float radiusX, float radiusY, 
    const D2D1_COLOR_F& color) {
    auto brush = createBrush(color);
    if (brush) {
        D2D1_ELLIPSE ellipse = D2D1::Ellipse(center, radiusX, radiusY);
        m_renderTarget->FillEllipse(ellipse, brush.Get());
    }
}

void D2DRenderer::drawLine(D2D1_POINT_2F start, D2D1_POINT_2F end, 
    const D2D1_COLOR_F& color, float strokeWidth) {
    auto brush = createBrush(color);
    if (brush) {
        m_renderTarget->DrawLine(start, end, brush.Get(), strokeWidth);
    }
}

void D2DRenderer::drawGradientRectangle(const D2D1_RECT_F& rect, 
    const D2D1_COLOR_F& topColor, const D2D1_COLOR_F& bottomColor) {
    Microsoft::WRL::ComPtr<ID2D1GradientStopCollection> stopCollection;
    Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> gradientBrush;

    D2D1_GRADIENT_STOP stops[2] = {
        { 1.0f, topColor },
        { 0.0f, bottomColor }
    };

    m_renderTarget->CreateGradientStopCollection(stops, 2, stopCollection.GetAddressOf());

    m_renderTarget->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(
            D2D1::Point2F(rect.left, rect.top),
            D2D1::Point2F(rect.left, rect.bottom)
        ),
        stopCollection.Get(),
        gradientBrush.GetAddressOf()
    );

    if (gradientBrush) {
        m_renderTarget->FillRectangle(rect, gradientBrush.Get());
    }
}

void D2DRenderer::drawText(std::wstring_view text, const D2D1_RECT_F& rect, 
    IDWriteTextFormat* textFormat, const D2D1_COLOR_F& color) {
    auto brush = createBrush(color);
    if (brush && textFormat) {
        m_renderTarget->DrawText(
            text.data(),
            static_cast<UINT32>(text.length()),
            textFormat,
            rect,
            brush.Get()
        );
    }
}

void D2DRenderer::drawText(std::wstring_view text, D2D1_POINT_2F origin, 
    IDWriteTextFormat* textFormat, const D2D1_COLOR_F& color) {
    auto brush = createBrush(color);
    if (brush && textFormat) {
        m_renderTarget->DrawText(
            text.data(),
            static_cast<UINT32>(text.length()),
            textFormat,
            D2D1::RectF(origin.x, origin.y, origin.x + 2000.0f, origin.y + 1000.0f),
            brush.Get()
        );
    }
}

void D2DRenderer::pushClipRect(const D2D1_RECT_F& rect) {
    m_renderTarget->PushAxisAlignedClip(rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

void D2DRenderer::popClipRect() {
    m_renderTarget->PopAxisAlignedClip();
}

void D2DRenderer::createDeviceResources() {
    // Resources are created lazily
}

void D2DRenderer::discardDeviceResources() {
    m_renderTarget.Reset();
    m_factory.Reset();
    m_dwriteFactory.Reset();
    m_brushCache.clear();
    m_initialized = false;
}
