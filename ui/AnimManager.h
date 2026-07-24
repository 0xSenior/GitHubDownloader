#pragma once
#include "pch.h"

// Easing functions
namespace Easing {
    float linear(float t);
    float easeInQuad(float t);
    float easeOutQuad(float t);
    float easeInOutQuad(float t);
    float easeOutCubic(float t);
    float easeOutExpo(float t);
}

// Animation class
class Animation {
public:
    Animation(float startValue, float endValue, float duration);
    ~Animation() = default;

    void start();
    void stop();
    void update(float deltaTime);
    bool isRunning() const { return m_running; }

    float getCurrentValue() const { return m_currentValue; }
    float getProgress() const { return m_progress; }

    void setEasingFunction(std::function<float(float)> easing) { m_easing = std::move(easing); }

private:
    float m_startValue;
    float m_endValue;
    float m_duration;
    float m_currentValue = 0.0f;
    float m_progress = 0.0f;
    bool m_running = false;
    std::chrono::steady_clock::time_point m_startTime;
    std::function<float(float)> m_easing = Easing::easeOutCubic;
};

// Animation Manager
class AnimManager {
public:
    static AnimManager& getInstance() {
        static AnimManager instance;
        return instance;
    }

    // Create animation
    std::shared_ptr<Animation> createAnimation(float start, float end, float duration);

    // Update all animations
    void update(float deltaTime);

    // Cancel all
    void cancelAll();

private:
    AnimManager() = default;
    std::vector<std::weak_ptr<Animation>> m_animations;
};

// Animated Value helper
class AnimatedFloat {
public:
    AnimatedFloat(float initialValue = 0.0f);

    void animateTo(float targetValue, float duration = 0.3f);
    float getValue() const { return m_value; }
    void update(float deltaTime);

private:
    float m_value;
    std::shared_ptr<Animation> m_animation;
};
