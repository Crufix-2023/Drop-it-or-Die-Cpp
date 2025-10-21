#ifndef ANIMATION_SYSTEM_H
#define ANIMATION_SYSTEM_H

#include <TGUI/TGUI.hpp>
#include <functional>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <cmath>

// Типы easing-функций
enum class EasingType {
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
    BounceIn,
    BounceOut,
    ElasticIn,
    ElasticOut,
    BackIn,
    BackOut
};

// Easing-функции
namespace EasingFunctions {
    static float linear(float t) { return t; }
    
    static float easeIn(float t) { return t * t; }
    
    static float easeOut(float t) { return 1 - (1 - t) * (1 - t); }
    
    static float easeInOut(float t) { 
        return t < 0.5 ? 2 * t * t : 1 - std::pow(-2 * t + 2, 2) / 2; 
    }
    
    static float bounceIn(float t) {
        return 1 - bounceIn(1 - t);
    }
    
    static float bounceOut(float t) {
        if (t < 1 / 2.75f) {
            return 7.5625f * t * t;
        } else if (t < 2 / 2.75f) {
            t -= 1.5f / 2.75f;
            return 7.5625f * t * t + 0.75f;
        } else if (t < 2.5 / 2.75f) {
            t -= 2.25f / 2.75f;
            return 7.5625f * t * t + 0.9375f;
        } else {
            t -= 2.625f / 2.75f;
            return 7.5625f * t * t + 0.984375f;
        }
    }
    
    static float elasticIn(float t) {
        if (t <= 0) return 0;
        if (t >= 1) return 1;
        return -std::pow(2, 10 * t - 10) * std::sin((t * 10 - 10.75f) * (2 * M_PI) / 3);
    }
    
    static float elasticOut(float t) {
        if (t <= 0) return 0;
        if (t >= 1) return 1;
        return std::pow(2, -10 * t) * std::sin((t * 10 - 0.75f) * (2 * M_PI) / 3) + 1;
    }
    
    static float backIn(float t) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1;
        return c3 * t * t * t - c1 * t * t;
    }
    
    static float backOut(float t) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1;
        return 1 + c3 * std::pow(t - 1, 3) + c1 * std::pow(t - 1, 2);
    }
    
    static float applyEasing(EasingType type, float t) {
        switch (type) {
            case EasingType::Linear: return linear(t);
            case EasingType::EaseIn: return easeIn(t);
            case EasingType::EaseOut: return easeOut(t);
            case EasingType::EaseInOut: return easeInOut(t);
            case EasingType::BounceIn: return bounceIn(t);
            case EasingType::BounceOut: return bounceOut(t);
            case EasingType::ElasticIn: return elasticIn(t);
            case EasingType::ElasticOut: return elasticOut(t);
            case EasingType::BackIn: return backIn(t);
            case EasingType::BackOut: return backOut(t);
            default: return linear(t);
        }
    }
}

struct AnimationStep {
    tgui::Widget::Ptr widget;
    sf::Vector2f startPos;
    sf::Vector2f targetPos;
    float duration;
    EasingType easingType;
    std::chrono::steady_clock::time_point startTime;
    bool completed = false;
    std::atomic<bool> cancelled{false};
    std::function<void()> onComplete;
    
    AnimationStep() : easingType(EasingType::Linear) {}
    AnimationStep(AnimationStep&& other) noexcept 
        : widget(std::move(other.widget))
        , startPos(other.startPos)
        , targetPos(other.targetPos)
        , duration(other.duration)
        , easingType(other.easingType)
        , startTime(other.startTime)
        , completed(other.completed)
        , cancelled(other.cancelled.load())
        , onComplete(std::move(other.onComplete))
    {}
    
    AnimationStep& operator=(AnimationStep&& other) noexcept {
        if (this != &other) {
            widget = std::move(other.widget);
            startPos = other.startPos;
            targetPos = other.targetPos;
            duration = other.duration;
            easingType = other.easingType;
            startTime = other.startTime;
            completed = other.completed;
            cancelled.store(other.cancelled.load());
            onComplete = std::move(other.onComplete);
        }
        return *this;
    }
    
    AnimationStep(const AnimationStep&) = delete;
    AnimationStep& operator=(const AnimationStep&) = delete;
};

class AnimationSystem {
private:
    static std::vector<AnimationStep> activeAnimations;
    static std::atomic<bool> systemActive;
    static std::mutex animationMutex;
    static bool isAnimating;

public:
    static void initialize() {
        systemActive = true;
        isAnimating = false;
    }
    
    static void shutdown() {
        std::lock_guard<std::mutex> lock(animationMutex);
        systemActive = false;
        for (auto& step : activeAnimations) {
            step.cancelled = true;
        }
        activeAnimations.clear();
        isAnimating = false;
    }
    
    // Простая анимация перемещения с easing
    static void move(tgui::Widget::Ptr widget, sf::Vector2f targetPos, float duration = 1.0f, EasingType easing = EasingType::Linear) {
        if (!systemActive || !widget) return;
        
        AnimationStep step;
        step.widget = widget;
        step.startPos = widget->getPosition();
        step.targetPos = targetPos;
        step.duration = duration;
        step.easingType = easing;
        step.startTime = std::chrono::steady_clock::now();
        
        std::lock_guard<std::mutex> lock(animationMutex);
        activeAnimations.push_back(std::move(step));
        isAnimating = true;
    }
    
    // Анимация с callback при завершении
    static void moveWithCallback(tgui::Widget::Ptr widget, sf::Vector2f targetPos, float duration, 
                                std::function<void()> callback, EasingType easing = EasingType::Linear) {
        if (!systemActive || !widget) return;
        
        AnimationStep step;
        step.widget = widget;
        step.startPos = widget->getPosition();
        step.targetPos = targetPos;
        step.duration = duration;
        step.easingType = easing;
        step.startTime = std::chrono::steady_clock::now();
        step.onComplete = callback;
        
        std::lock_guard<std::mutex> lock(animationMutex);
        activeAnimations.push_back(std::move(step));
        isAnimating = true;
    }
    
    // Цепочка анимаций с одинаковым временем и easing для всех шагов
    static void sequence(tgui::Widget::Ptr widget, std::initializer_list<sf::Vector2f> positions, 
                        float stepDuration = 1.0f, EasingType easing = EasingType::Linear) {
        if (!systemActive || !widget || positions.size() == 0) return;
        
        auto posList = std::vector<sf::Vector2f>(positions);
        auto durations = std::vector<float>(posList.size(), stepDuration);
        auto easings = std::vector<EasingType>(posList.size(), easing);
        sequenceAdvanced(widget, posList, durations, easings);
    }
    
    // Цепочка анимаций с разным временем для каждого шага
    static void sequenceAdvanced(tgui::Widget::Ptr widget, std::initializer_list<sf::Vector2f> positions, 
                                std::initializer_list<float> durations, std::initializer_list<EasingType> easings = {}) {
        if (!systemActive || !widget || positions.size() == 0 || positions.size() != durations.size()) return;
        
        auto posVec = std::vector<sf::Vector2f>(positions);
        auto durVec = std::vector<float>(durations);
        auto easeVec = std::vector<EasingType>(easings);
        if (easeVec.size() != posVec.size()) {
            easeVec = std::vector<EasingType>(posVec.size(), EasingType::Linear);
        }
        sequenceAdvanced(widget, posVec, durVec, easeVec);
    }
    
    // Цепочка анимаций с разным временем и easing (версия с векторами)
    static void sequenceAdvanced(tgui::Widget::Ptr widget, const std::vector<sf::Vector2f>& positions, 
                                const std::vector<float>& durations, const std::vector<EasingType>& easings = {}) {
        if (!systemActive || !widget || positions.empty() || positions.size() != durations.size()) return;
        
        std::vector<EasingType> actualEasings = easings;
        if (actualEasings.size() != positions.size()) {
            actualEasings = std::vector<EasingType>(positions.size(), EasingType::Linear);
        }
        
        float totalDelay = 0;
        
        for (size_t i = 0; i < positions.size(); ++i) {
            AnimationStep step;
            step.widget = widget;
            step.targetPos = positions[i];
            step.duration = durations[i];
            step.easingType = actualEasings[i];
            step.startTime = std::chrono::steady_clock::now() + 
                            std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                                std::chrono::duration<float>(totalDelay));
            
            std::lock_guard<std::mutex> lock(animationMutex);
            activeAnimations.push_back(std::move(step));
            
            totalDelay += durations[i];
        }
        
        isAnimating = true;
    }
    
    // Цепочка анимаций с callback после каждого шага
    static void sequenceWithCallbacks(tgui::Widget::Ptr widget, 
                                     std::initializer_list<sf::Vector2f> positions, 
                                     std::initializer_list<float> durations,
                                     std::initializer_list<std::function<void()>> callbacks,
                                     std::initializer_list<EasingType> easings = {}) {
        if (!systemActive || !widget || positions.size() == 0 || 
            positions.size() != durations.size() || positions.size() != callbacks.size()) return;
        
        auto posVec = std::vector<sf::Vector2f>(positions);
        auto durVec = std::vector<float>(durations);
        auto cbVec = std::vector<std::function<void()>>(callbacks);
        auto easeVec = std::vector<EasingType>(easings);
        if (easeVec.size() != posVec.size()) {
            easeVec = std::vector<EasingType>(posVec.size(), EasingType::Linear);
        }
        
        float totalDelay = 0;
        
        for (size_t i = 0; i < posVec.size(); ++i) {
            AnimationStep step;
            step.widget = widget;
            step.targetPos = posVec[i];
            step.duration = durVec[i];
            step.easingType = easeVec[i];
            step.startTime = std::chrono::steady_clock::now() + 
                            std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                                std::chrono::duration<float>(totalDelay));
            step.onComplete = cbVec[i];
            
            std::lock_guard<std::mutex> lock(animationMutex);
            activeAnimations.push_back(std::move(step));
            
            totalDelay += durVec[i];
        }
        
        isAnimating = true;
    }
    
    static void updateAnimations() {
        if (!systemActive) return;
        
        std::lock_guard<std::mutex> lock(animationMutex);
        
        if (activeAnimations.empty()) {
            isAnimating = false;
            return;
        }
        
        auto currentTime = std::chrono::steady_clock::now();
        std::vector<AnimationStep> remainingAnimations;
        
        for (auto& step : activeAnimations) {
            if (step.completed || step.cancelled || step.widget == nullptr) {
                continue;
            }
            
            // Если это отложенная анимация, проверяем время старта
            if (step.startTime > currentTime) {
                remainingAnimations.push_back(std::move(step));
                continue;
            }
            
            // Для активных анимаций устанавливаем стартовую позицию при первом обновлении
            if (step.startPos == sf::Vector2f(0, 0)) {
                step.startPos = step.widget->getPosition();
            }
            
            auto elapsed = std::chrono::duration<float>(currentTime - step.startTime).count();
            float progress = std::min(elapsed / step.duration, 1.0f);
            
            // Применяем easing-функцию
            float easedProgress = EasingFunctions::applyEasing(step.easingType, progress);
            
            sf::Vector2f newPos;
            newPos.x = step.startPos.x + (step.targetPos.x - step.startPos.x) * easedProgress;
            newPos.y = step.startPos.y + (step.targetPos.y - step.startPos.y) * easedProgress;
            
            step.widget->setPosition(newPos.x, newPos.y);
            
            if (progress >= 1.0f) {
                step.completed = true;
                step.widget->setPosition(step.targetPos.x, step.targetPos.y);
                
                // Вызываем callback если есть
                if (step.onComplete) {
                    step.onComplete();
                }
            } else {
                remainingAnimations.push_back(std::move(step));
            }
        }
        
        activeAnimations = std::move(remainingAnimations);
        isAnimating = !activeAnimations.empty();
    }
    
    static bool isBusy() {
        return isAnimating && systemActive;
    }
    
    static void stop(tgui::Widget::Ptr widget = nullptr) {
        std::lock_guard<std::mutex> lock(animationMutex);
        if (widget) {
            std::vector<AnimationStep> remainingAnimations;
            for (auto& step : activeAnimations) {
                if (step.widget != widget) {
                    remainingAnimations.push_back(std::move(step));
                } else {
                    step.cancelled = true;
                }
            }
            activeAnimations = std::move(remainingAnimations);
        } else {
            for (auto& step : activeAnimations) {
                step.cancelled = true;
            }
            activeAnimations.clear();
        }
        isAnimating = !activeAnimations.empty();
    }
};

std::vector<AnimationStep> AnimationSystem::activeAnimations;
std::atomic<bool> AnimationSystem::systemActive{false};
std::mutex AnimationSystem::animationMutex;
bool AnimationSystem::isAnimating = false;

#endif