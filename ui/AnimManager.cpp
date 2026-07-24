#include "pch.h"
#include "AnimManager.h"

// Easing functions
namespace Easing {
    float linear(float t) {
        return t;
    }

    float easeInQuad(float t) {
        return t * t;
    }

    float easeOutQuad(float t) {
        return t * (2.0f - t);
    }

    float easeInOutQuad(float t) {
        return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
    }

    float easeOutCubic(float t) {
        float f = t - 1.0f;
        return f * f * f + 1.0f;
    }

    float easeOutExpo(float t) {
        return (t == 1.0f) ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
    }
}

// Animation
Animation::Animation(float startValue, float endValue, float duration)
    : m_startValue(startValue)
    , m_endValue(endValue)
    , m_duration(duration)
    , m_currentValue(startValue)
{
}

void Animation::start() {
    m_running = true;
    m_progress = 0.0f;
    m_startTime = std::chrono::steady_clock::now();
}

void Animation::stop() {
    m_running = false;
}

void Animation::update(float deltaTime) {
    if (!m_running) return;

    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - m_startTime).count();
    
    m_progress = std::min(elapsed / m_duration, 1.0f);
    float easedProgress = m_easing(m_progress);
    
    m_currentValue = m_startValue + (m_endValue - m_startValue) * easedProgress;

    if (m_progress >= 1.0f) {
        m_currentValue = m_endValue;
        m_running = false;
    }
}

// AnimManager
std::shared_ptr<Animation> AnimManager::createAnimation(float start, float end, float duration) {
    auto anim = std::make_shared<Animation>(start, end, duration);
    m_animations.push_back(anim);
    return anim;
}

void AnimManager::update(float deltaTime) {
    for (auto it = m_animations.begin(); it != m_animations.end(); ) {
        if (auto anim = it->lock()) {
            anim->update(deltaTime);
            ++it;
        } else {
            it = m_animations.erase(it);
        }
    }
}

void AnimManager::cancelAll() {
    for (auto& weak : m_animations) {
        if (auto anim = weak.lock()) {
            anim->stop();
        }
    }
    m_animations.clear();
}

// AnimatedFloat
AnimatedFloat::AnimatedFloat(float initialValue)
    : m_value(initialValue)
{
}

void AnimatedFloat::animateTo(float targetValue, float duration) {
    m_animation = AnimManager::getInstance().createAnimation(m_value, targetValue, duration);
    m_animation->start();
}

void AnimatedFloat::update(float deltaTime) {
    if (m_animation && m_animation->isRunning()) {
        m_animation->update(deltaTime);
        m_value = m_animation->getCurrentValue();
    }
}
