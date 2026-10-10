#include "Result.h"
#include "DxLib.h"
#include "Input/Input.h"
#include "Core/Time.h"
#include "Skin/SkinData.h"
#include <cmath>
#include <cstdio>

namespace {
constexpr double Pi = 3.14159265358979323846;
bool DonDown() { return Input::isKeyDown(KEY_INPUT_F) || Input::isKeyDown(KEY_INPUT_J); }
const char* CourseLabel(CourseType course) {
    switch (course) {
    case CourseType::Easy: return "かんたん";
    case CourseType::Normal: return "ふつう";
    case CourseType::Hard: return "むずかしい";
    case CourseType::InnerOni: return "おに（裏）";
    default: return "おに";
    }
}
// Clockwise -45 to 225 degrees. Even a perfect score keeps the top opening.
void Arc(double fraction, unsigned int color) {
    if (fraction <= 0.0) return;
    constexpr double cx = 954, cy = 410, radius = 194, halfWidth = 15;
    const double sweep = std::clamp(fraction, 0.0, 1.0) * 1.5 * Pi;
    const int segments = (std::max)(1, int(std::ceil(sweep * 60)));
    const auto x = [](double r, double a) { return int(std::lround(cx + r * std::cos(a))); };
    const auto y = [](double r, double a) { return int(std::lround(cy + r * std::sin(a))); };
    for (int i = 0; i < segments; ++i) {
        const double a = -Pi / 4 + sweep * i / segments;
        const double b = -Pi / 4 + sweep * (i + 1) / segments;
        DrawTriangle(x(radius-halfWidth,a), y(radius-halfWidth,a), x(radius+halfWidth,a),
            y(radius+halfWidth,a), x(radius+halfWidth,b), y(radius+halfWidth,b), color, TRUE);
        DrawTriangle(x(radius-halfWidth,a), y(radius-halfWidth,a), x(radius+halfWidth,b),
            y(radius+halfWidth,b), x(radius-halfWidth,b), y(radius-halfWidth,b), color, TRUE);
    }
    for (const double a : {-Pi / 4, -Pi / 4 + sweep})
        DrawCircle(x(radius,a), y(radius,a), int(halfWidth), color, TRUE);
}
}

void ResultScene::Init() {
    data_ = ctx_ ? ctx_->resultData : nullptr;
    if (data_) summary_ = ResultLogic::Calculate(data_->goodCount, data_->okCount,
        data_->badCount, data_->missCount, data_->rollHitCount, data_->balloonHitCount);
    animation_.Reset(DonDown(), bool(data_));
    labelFont_ = CreateFontToHandle("メイリオ", 25, 6, DX_FONTTYPE_ANTIALIASING);
    titleFont_ = CreateFontToHandle("メイリオ", 40, 8, DX_FONTTYPE_ANTIALIASING);
    numberFont_ = CreateFontToHandle("メイリオ", 60, 8, DX_FONTTYPE_ANTIALIASING);
    rankFont_ = CreateFontToHandle("メイリオ", 100, 9, DX_FONTTYPE_ANTIALIASING);
}
void ResultScene::Finalize() {
    for (int* font : {&labelFont_, &titleFont_, &numberFont_, &rankFont_}) {
        if (*font >= 0) DeleteFontToHandle(*font);
        *font = -1;
    }
    data_.reset();
}
void ResultScene::Update() { Input(); }
void ResultScene::Input() {
    if (animation_.Update(Time::deltaSec(), Input::isNoteKeyTriggered(NoteType::Don), DonDown())) {
        if (ctx_) ctx_->resultData.reset();
        RequestScene(SceneID::SongSelect);
    }
}
void ResultScene::Text(int x, int y, const std::string& text, int font, unsigned int color) {
    if (font >= 0) DrawStringToHandle(x, y, text.c_str(), color, font);
    else DrawString(x, y, text.c_str(), color);
}
void ResultScene::CenterText(int x, int y, const std::string& text, int font, unsigned int color) {
    const int width = font >= 0 ? GetDrawStringWidthToHandle(text.c_str(), int(text.size()), font)
        : GetDrawStringWidth(text.c_str(), int(text.size()));
    Text(x - width / 2, y, text, font, color);
}
void ResultScene::Draw() {
    const auto ink = GetColor(61, 32, 28), accent = GetColor(218, 0, 16);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    DrawGraph(0, 0, Skin::GetTexture("result/bg").handle, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 235);
    DrawRoundRect(40, 25, 1240, 175, 22, 22, GetColor(255, 252, 235), TRUE);
    DrawRoundRect(40, 195, 650, 630, 22, 22, GetColor(255, 252, 235), TRUE);
    DrawRoundRect(675, 195, 1240, 630, 22, 22, GetColor(255, 252, 235), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    if (!data_) {
        CenterText(640, 300, "リザルトデータがありません", titleFont_, ink);
        CenterText(640, 657, "ドン（F / J）で選曲へ", labelFont_, ink);
        return;
    }
    Text(65, 40, "演奏結果", labelFont_, accent);
    // Measure and shrink the actual UTF-8 title; do not split multibyte glyphs.
    const int width = titleFont_ >= 0 ? GetDrawStringWidthToHandle(data_->title.c_str(),
        int(data_->title.size()), titleFont_) : 0;
    const double scale = width > 1120 ? 1120.0 / width : 1.0;
    if (titleFont_ >= 0)
        DrawExtendStringToHandle(65, 77, scale, scale, data_->title.c_str(), ink, titleFont_);
    else Text(65, 77, data_->title, titleFont_, ink);
    char level[80];
    std::snprintf(level, sizeof(level), "%s   ★ %.0f", CourseLabel(data_->course), data_->level);
    Text(65, 132, level, labelFont_, ink);
    Text(75, 218, "スコア", labelFont_, ink);
    Text(75, 250, std::to_string(data_->score), numberFont_, accent);
    const auto row = [&](int y, const char* label, std::int64_t value) {
        Text(80, y, label, labelFont_, ink);
        const auto text = std::to_string(value);
        const int w = labelFont_ >= 0 ? GetDrawStringWidthToHandle(text.c_str(), int(text.size()), labelFont_) : 0;
        Text(605 - w, y, text, labelFont_, ink);
    };
    row(348, "良", data_->goodCount);
    row(397, "可", data_->okCount);
    row(446, "不可", summary_.bad);
    row(507, "最大コンボ", data_->maxCombo);
    row(556, "連打数", summary_.roll);
    Arc(1.0, GetColor(224, 207, 185));
    Arc(summary_.Fraction() * animation_.Progress(), accent);
    CenterText(954, 238, "精度", labelFont_, ink);
    CenterText(954, 325, ResultLogic::RankText(animation_.DisplayRank(summary_)), rankFont_, accent);
    const int hundredths = animation_.DisplayHundredths(summary_);
    char accuracy[24];
    std::snprintf(accuracy, sizeof(accuracy), "%d.%02d%%", hundredths / 100, hundredths % 100);
    CenterText(954, 457, accuracy, titleFont_, ink);
    CenterText(640, 657, animation_.Complete() ? "ドン（F / J）で選曲へ" :
        "ドン（F / J）で演出をスキップ", labelFont_, ink);
    const int alpha = int(std::lround(animation_.FadeOpacity() * 255));
    if (alpha > 0) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}
