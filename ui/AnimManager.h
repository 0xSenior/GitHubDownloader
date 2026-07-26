#pragma once
#include "pch.h"

namespace Easing {
    inline float linear(float t) { return t; }
    inline float easeOutCubic(float t) { float f = t - 1.0f; return f * f * f + 1.0f; }
    inline float easeOutQuart(float t) { float f = t - 1.0f; return 1.0f - f * f * f * f; }
    inline float easeOutExpo(float t) { return t >= 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t); }
    inline float easeInOutCubic(float t) {
        return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    }
}

class AnimatedFloat {
public:
    AnimatedFloat(float initial = 0.0f) : m_value(initial), m_target(initial) {}

    void animateTo(float target, float duration = 0.3f,
                   std::function<float(float)> easing = Easing::easeOutCubic) {
        m_start = m_value;
        m_target = target;
        m_duration = duration;
        m_elapsed = 0.0f;
        m_easing = std::move(easing);
        m_running = duration > 0.0f;
    }

    void setImmediate(float value) {
        m_value = value;
        m_target = value;
        m_running = false;
    }

    void update(float dt) {
        if (!m_running) return;
        m_elapsed += dt;
        float t = std::min(m_elapsed / m_duration, 1.0f);
        m_value = m_start + (m_target - m_start) * m_easing(t);
        if (t >= 1.0f) {
            m_value = m_target;
            m_running = false;
        }
    }

    float getValue() const { return m_value; }
    float getTarget() const { return m_target; }
    bool isAnimating() const { return m_running; }

private:
    float m_value, m_target;
    float m_start = 0.0f;
    float m_duration = 0.0f;
    float m_elapsed = 0.0f;
    bool m_running = false;
    std::function<float(float)> m_easing = Easing::easeOutCubic;
};

class AnimatedColor {
public:
    AnimatedColor(D2D1_COLOR_F initial = Colors::Background()) : m_value(initial), m_target(initial) {}

    void animateTo(D2D1_COLOR_F target, float duration = 0.25f) {
        m_start = m_value;
        m_target = target;
        m_duration = duration;
        m_elapsed = 0.0f;
        m_running = duration > 0.0f;
    }

    void setImmediate(D2D1_COLOR_F c) { m_value = c; m_target = c; m_running = false; }

    void update(float dt) {
        if (!m_running) return;
        m_elapsed += dt;
        float t = std::min(m_elapsed / m_duration, 1.0f);
        float et = Easing::easeOutCubic(t);
        m_value.r = m_start.r + (m_target.r - m_start.r) * et;
        m_value.g = m_start.g + (m_target.g - m_start.g) * et;
        m_value.b = m_start.b + (m_target.b - m_start.b) * et;
        m_value.a = m_start.a + (m_target.a - m_start.a) * et;
        if (t >= 1.0f) { m_value = m_target; m_running = false; }
    }

    D2D1_COLOR_F getColor() const { return m_value; }
    bool isAnimating() const { return m_running; }

private:
    D2D1_COLOR_F m_value, m_target, m_start;
    float m_duration = 0.0f, m_elapsed = 0.0f;
    bool m_running = false;
};
