#include "pch.h"
#include "ui/D2DRenderer.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

D2DRenderer::D2DRenderer() = default;
D2DRenderer::~D2DRenderer() { shutdown(); }

bool D2DRenderer::initialize(HWND hwnd) {
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_factory.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(m_dwrite.GetAddressOf()));
    if (FAILED(hr)) return false;

    RECT rc;
    GetClientRect(hwnd, &rc);

    hr = m_factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
        m_rt.GetAddressOf());
    if (FAILED(hr)) return false;

    m_rt->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
    m_initialized = true;
    return true;
}

void D2DRenderer::shutdown() {
    m_rt.Reset();
    m_factory.Reset();
    m_dwrite.Reset();
    m_initialized = false;
}

void D2DRenderer::beginDraw() {
    if (m_rt) { m_rt->BeginDraw(); m_rt->Clear(Colors::Background()); }
}

void D2DRenderer::endDraw() {
    if (m_rt) {
        HRESULT hr = m_rt->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET) { shutdown(); }
    }
}

void D2DRenderer::resize(UINT w, UINT h) { if (m_rt) m_rt->Resize(D2D1::SizeU(w, h)); }

D2D1_SIZE_F D2DRenderer::getSize() const { return m_rt ? m_rt->GetSize() : D2D1::SizeF(0, 0); }

Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> D2DRenderer::createBrush(const D2D1_COLOR_F& c) {
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> b;
    if (m_rt) m_rt->CreateColorBrush(c, b.GetAddressOf());
    return b;
}

Microsoft::WRL::ComPtr<IDWriteTextFormat> D2DRenderer::createTextFormat(float fontSize, std::wstring fontFamily, DWRITE_FONT_WEIGHT weight) {
    Microsoft::WRL::ComPtr<IDWriteTextFormat> fmt;
    if (m_dwrite) {
        m_dwrite->CreateTextFormat(fontFamily.c_str(), nullptr, weight, DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-us", fmt.GetAddressOf());
    }
    return fmt;
}

Microsoft::WRL::ComPtr<IDWriteTextFormat> D2DRenderer::createMonoTextFormat(float fontSize, DWRITE_FONT_WEIGHT weight) {
    return createTextFormat(fontSize, L"Cascadia Code", weight);
}

void D2DRenderer::drawFilledRectangle(const D2D1_RECT_F& r, const D2D1_COLOR_F& c) {
    auto b = createBrush(c);
    if (b) m_rt->FillRectangle(r, b.Get());
}

void D2DRenderer::drawRoundedRectangle(const D2D1_RECT_F& r, const D2D1_COLOR_F& c, float radius, float sw) {
    auto b = createBrush(c);
    if (b) m_rt->DrawRoundedRectangle(D2D1::RoundedRect(r, radius, radius), b.Get(), sw);
}

void D2DRenderer::drawFilledRoundedRectangle(const D2D1_RECT_F& r, const D2D1_COLOR_F& c, float radius) {
    auto b = createBrush(c);
    if (b) m_rt->FillRoundedRectangle(D2D1::RoundedRect(r, radius, radius), b.Get());
}

void D2DRenderer::drawEllipse(D2D1_POINT_2F center, float rx, float ry, const D2D1_COLOR_F& c) {
    auto b = createBrush(c);
    if (b) m_rt->DrawEllipse(D2D1::Ellipse(center, rx, ry), b.Get());
}

void D2DRenderer::drawFilledEllipse(D2D1_POINT_2F center, float rx, float ry, const D2D1_COLOR_F& c) {
    auto b = createBrush(c);
    if (b) m_rt->FillEllipse(D2D1::Ellipse(center, rx, ry), b.Get());
}

void D2DRenderer::drawLine(D2D1_POINT_2F a, D2D1_POINT_2F b, const D2D1_COLOR_F& c, float w) {
    auto br = createBrush(c);
    if (br) m_rt->DrawLine(a, b, br.Get(), w);
}

void D2DRenderer::drawGradientRectangle(const D2D1_RECT_F& r, const D2D1_COLOR_F& top, const D2D1_COLOR_F& bot) {
    Microsoft::WRL::ComPtr<ID2D1GradientStopCollection> stops;
    Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> gbr;
    D2D1_GRADIENT_STOP gs[2] = { { 1.0f, top }, { 0.0f, bot } };
    m_rt->CreateGradientStopCollection(gs, 2, stops.GetAddressOf());
    m_rt->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(D2D1::Point2F(r.left, r.top), D2D1::Point2F(r.left, r.bottom)),
        stops.Get(), gbr.GetAddressOf());
    if (gbr) m_rt->FillRectangle(r, gbr.Get());
}

void D2DRenderer::drawText(std::wstring_view text, const D2D1_RECT_F& r, IDWriteTextFormat* fmt,
    const D2D1_COLOR_F& c, D2D1_TEXT_ALIGNMENT align) {
    if (!fmt) return;
    fmt->SetTextAlignment(align);
    fmt->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    auto b = createBrush(c);
    if (b) m_rt->DrawText(text.data(), (UINT32)text.length(), fmt, r, b.Get());
}

void D2DRenderer::drawTextClipped(std::wstring_view text, const D2D1_RECT_F& r, IDWriteTextFormat* fmt,
    const D2D1_COLOR_F& c) {
    if (!fmt) return;
    fmt->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    fmt->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    fmt->SetTrimming(DWRITE_TRIMMING_GRANULARITY_CHARACTER, nullptr);
    auto b = createBrush(c);
    if (b) m_rt->DrawText(text.data(), (UINT32)text.length(), fmt, r, b.Get());
}

void D2DRenderer::pushClipRect(const D2D1_RECT_F& r) {
    m_rt->PushAxisAlignedClip(r, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

void D2DRenderer::popClipRect() { m_rt->PopAxisAlignedClip(); }
