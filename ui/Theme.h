#pragma once
#include "pch.h"

class Theme {
public:
    static Theme& getInstance() {
        static Theme instance;
        return instance;
    }

    // Getters for colors
    D2D1_COLOR_F getBackground() const { return Colors::Background; }
    D2D1_COLOR_F getSurface() const { return Colors::Surface; }
    D2D1_COLOR_F getSurfaceLight() const { return Colors::SurfaceLight; }
    D2D1_COLOR_F getPrimary() const { return Colors::Primary; }
    D2D1_COLOR_F getPrimaryHover() const { return Colors::PrimaryHover; }
    D2D1_COLOR_F getSecondary() const { return Colors::Secondary; }
    D2D1_COLOR_F getSuccess() const { return Colors::Success; }
    D2D1_COLOR_F getError() const { return Colors::Error; }
    D2D1_COLOR_F getWarning() const { return Colors::Warning; }
    D2D1_COLOR_F getTextPrimary() const { return Colors::TextPrimary; }
    D2D1_COLOR_F getTextSecondary() const { return Colors::TextSecondary; }
    D2D1_COLOR_F getTextMuted() const { return Colors::TextMuted; }
    D2D1_COLOR_F getBorder() const { return Colors::Border; }
    D2D1_COLOR_F getInputBackground() const { return Colors::InputBackground; }
    D2D1_COLOR_F getProgressBar() const { return Colors::ProgressBar; }
    D2D1_COLOR_F getProgressBarTrack() const { return Colors::ProgressBarTrack; }

    // Animation colors
    D2D1_COLOR_F lerp(const D2D1_COLOR_F& a, const D2D1_COLOR_F& b, float t) const {
        return D2D1::ColorF(
            a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t,
            a.a + (b.a - a.a) * t
        );
    }

    // Typography
    std::wstring getFontFamily() const { return L"Segoe UI Variable"; }
    std::wstring getMonospaceFontFamily() const { return L"Cascadia Code"; }

private:
    Theme() = default;
};
