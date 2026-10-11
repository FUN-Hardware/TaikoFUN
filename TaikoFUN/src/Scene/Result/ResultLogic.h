#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

// Independent of DxLib so the result rules and input lifecycle can be tested.
namespace ResultLogic {
enum class Rank { C, B, A, S, SS };
inline const char* RankText(Rank rank) {
    switch (rank) {
    case Rank::SS: return "SS";
    case Rank::S: return "S";
    case Rank::A: return "A";
    case Rank::B: return "B";
    default: return "C";
    }
}

struct Summary {
    std::int64_t numerator = 0;   // 2 * Good + OK
    std::int64_t denominator = 0; // 2 * all regular judgments
    std::int64_t bad = 0;
    std::int64_t roll = 0;
    double Fraction() const {
        return denominator > 0 ? double(numerator) / double(denominator) : 0.0;
    }
    int Hundredths() const {
        return denominator > 0 ? int(numerator * 10000 / denominator) : 0;
    }
    Rank FinalRank() const {
        if (denominator == 0) return Rank::C;
        if (numerator == denominator) return Rank::SS;
        if (numerator * 100 >= denominator * 90) return Rank::S;
        if (numerator * 100 >= denominator * 80) return Rank::A;
        if (numerator * 100 >= denominator * 60) return Rank::B;
        return Rank::C;
    }
};

inline Summary Calculate(int good, int ok, int bad, int miss, int roll, int balloon) {
    const auto count = [](int n) { return std::int64_t((std::max)(0, n)); };
    const auto g = count(good), o = count(ok), b = count(bad), m = count(miss);
    return {2 * g + o, 2 * (g + o + b + m), b + m, count(roll) + count(balloon)};
}

class Animation {
public:
    static constexpr double FadeSeconds = 0.4;
    static constexpr double GaugeSeconds = 3.0;
    void Reset(bool donDown, bool animate = true) {
        elapsed_ = animate ? 0.0 : FadeSeconds + GaugeSeconds;
        armed_ = !donDown;
    }
    // A press on the completion frame still skips, and can never also exit.
    bool Update(double delta, bool donPressed, bool donDown) {
        const bool wasComplete = Complete();
        elapsed_ = (std::min)(FadeSeconds + GaugeSeconds,
            elapsed_ + (std::max)(0.0, delta));
        if (!donDown) armed_ = true;
        if (!donPressed || !armed_) return false;
        armed_ = false; // Both Don keys must be released before another action.
        if (!wasComplete) {
            elapsed_ = FadeSeconds + GaugeSeconds;
            return false;
        }
        return true;
    }
    bool Complete() const { return elapsed_ >= FadeSeconds + GaugeSeconds; }
    double FadeOpacity() const { return 1.0 - (std::min)(1.0, elapsed_ / FadeSeconds); }
    double Progress() const {
        const double t = std::clamp((elapsed_ - FadeSeconds) / GaugeSeconds, 0.0, 1.0);
        return 1.0 - (1.0 - t) * (1.0 - t) * (1.0 - t);
    }
    int DisplayHundredths(const Summary& summary) const {
        if (Complete()) return summary.Hundredths();
        return (std::min)(summary.Hundredths(), int(std::floor(summary.Fraction() * Progress() * 10000)));
    }
    Rank DisplayRank(const Summary& summary) const {
        if (Complete()) return summary.FinalRank();
        const int accuracy = DisplayHundredths(summary);
        if (accuracy >= 9000) return Rank::S;
        if (accuracy >= 8000) return Rank::A;
        if (accuracy >= 6000) return Rank::B;
        return Rank::C;
    }
private:
    double elapsed_ = 0.0;
    bool armed_ = true;
};

class PlayEndTransition {
public:
    static constexpr double LingerSeconds = 5.0;
    void Reset() { elapsed_ = 0.0; }
    void Update(double delta) {
        elapsed_ = (std::min)(LingerSeconds, elapsed_ + (std::max)(0.0, delta));
    }
    bool Complete() const { return elapsed_ >= LingerSeconds; }
    double FadeOpacity() const {
        return std::clamp((elapsed_ - (LingerSeconds - Animation::FadeSeconds)) /
            Animation::FadeSeconds, 0.0, 1.0);
    }
private:
    double elapsed_ = 0.0;
};
}
