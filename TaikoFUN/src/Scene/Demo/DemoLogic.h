#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <optional>

enum class PlayMode { Normal, Demo };

// Timing and input policies without DxLib dependencies.
namespace DemoLogic {
inline constexpr double IdleSeconds = 30.0;
inline constexpr double LingerSeconds = 1.0;
inline constexpr double FadeSeconds = 0.4;

class IdleTimer {
public:
    void Reset() { elapsed_ = 0.0; eligibleLastFrame_ = false; }
    bool Update(double delta, bool focused, bool keyDown, bool keyPressed, bool canStart) {
        if (!focused || keyDown || keyPressed || !canStart) {
            Reset();
            return false;
        }
        // ProcessMessage may pause updates while unfocused. Never count that gap.
        if (!eligibleLastFrame_ || !std::isfinite(delta) || delta < 0.0 || delta > 1.0) {
            elapsed_ = 0.0;
            eligibleLastFrame_ = true;
            return false;
        }
        elapsed_ = (std::min)(IdleSeconds, elapsed_ + delta);
        return elapsed_ >= IdleSeconds;
    }
private:
    double elapsed_ = 0.0;
    bool eligibleLastFrame_ = false;
};

// Consumes the cancellation press and every overlapping held key on return.
class ReleaseGate {
public:
    void Reset(bool block) { blocked_ = block; }
    bool Update(bool keyDown) {
        if (!blocked_) return true;
        if (!keyDown) blocked_ = false;
        return false;
    }
private:
    bool blocked_ = false;
};

inline std::optional<int> ChooseCourse(const std::array<bool, 5>& courses, int preferred) {
    if (preferred >= 0 && preferred < int(courses.size()) && courses[preferred]) return preferred;
    for (int i = 0; i < int(courses.size()); ++i) if (courses[i]) return i;
    return std::nullopt;
}

enum class Exit { None, Cancelled, Finished };
class Session {
public:
    void Reset() { entryElapsed_ = endElapsed_ = 0.0; ending_ = false; }
    Exit Update(double delta, bool chartFinished, bool cancel) {
        if (cancel) return Exit::Cancelled; // User input wins even at the finish boundary.
        const double dt = std::isfinite(delta) ? (std::max)(0.0, delta) : 0.0;
        entryElapsed_ = (std::min)(FadeSeconds, entryElapsed_ + dt);
        if (chartFinished && !ending_) {
            ending_ = true;
            endElapsed_ = 0.0;
        } else if (ending_) {
            endElapsed_ = (std::min)(LingerSeconds, endElapsed_ + dt);
        }
        return ending_ && endElapsed_ >= LingerSeconds ? Exit::Finished : Exit::None;
    }
    double FadeOpacity() const {
        const double entry = 1.0 - entryElapsed_ / FadeSeconds;
        const double ending = ending_ ? std::clamp(
            (endElapsed_ - (LingerSeconds - FadeSeconds)) / FadeSeconds, 0.0, 1.0) : 0.0;
        return (std::max)(entry, ending);
    }
private:
    double entryElapsed_ = 0.0, endElapsed_ = 0.0;
    bool ending_ = false;
};
}
