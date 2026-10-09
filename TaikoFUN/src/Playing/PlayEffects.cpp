#include "DxLib.h"
#include "PlayEffects.h"
#include "Skin/SkinData.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace {
	constexpr double kPi = 3.14159265358979323846;

	// 1280x720 基準の配置。PlayScene の SkinLayout（レーン 325,189 / ノーツ 128px）に合わせる。
	constexpr double judgeX = 325.0 + 64.0, judgeY = 189.0 + 64.0;	// 判定枠の中心
	constexpr int laneX = 325, laneY = 189, laneW = 955, laneH = 130;
	constexpr double judgeTextY = 159.0, judgeTextHeight = 44.0;	// レーンのすぐ上
	// 10/04 の HUD 案（コンボ枠 x=47, y=345, 幅270）
	constexpr double comboX = 47.0, comboY = 345.0, comboW = 270.0;

	// 時間（秒）
	constexpr double burstTime = 0.32, rollBurstTime = 0.22, laneFlashTime = 0.18;
	constexpr double judgeTime = 0.5, comboPopTime = 0.16, milestoneTime = 0.45, brokenTime = 0.4;

	struct Rgb { int r, g, b; };
	constexpr Rgb donColor{ 218, 0, 16 };		// #DA0010
	constexpr Rgb katsuColor{ 0, 134, 174 };	// #0086AE
	constexpr Rgb rollColor{ 255, 214, 0 };		// #FFD600
	constexpr Rgb outlineColor{ 32, 42, 82 };	// #202A52（レーンと同じ）
	constexpr Rgb white{ 255, 255, 255 };

	double Clamp01(double v) { return std::clamp(v, 0.0, 1.0); }
	double EaseOutCubic(double t) { t = Clamp01(t); return 1.0 - std::pow(1.0 - t, 3.0); }

	void SetAlpha(double alpha) {
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(int(std::lround(alpha * 255.0)), 0, 255));
	}
	void SetTint(const Rgb& c) { SetDrawBright(c.r, c.g, c.b); }
	void ResetDrawState() {
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		SetDrawBright(255, 255, 255);
	}
	unsigned int ToColor(const Rgb& c) { return GetColor(c.r, c.g, c.b); }

	// 画像の中心を (x, y) に置いて拡大・回転して描く
	void DrawCentered(const imgData& img, double x, double y, double scaleX, double scaleY, double angle) {
		DrawRotaGraph3F(float(x), float(y), float(img.w / 2.0), float(img.h / 2.0),
			scaleX, scaleY, angle, img.handle, TRUE);
	}

	// HUD 案と同じ「画像＋180度回転した複製」を同じ枠に重ねる形
	double PairedHeight(const imgData& img, double width) { return width * img.h / img.w; }
	void DrawPaired(const imgData& img, double x, double y, double width, double scale) {
		const double cx = x + width / 2.0, cy = y + PairedHeight(img, width) / 2.0;
		const double ext = width / img.w * scale;
		DrawCentered(img, cx, cy, ext, ext, 0.0);
		DrawCentered(img, cx, cy, ext, ext, kPi);
	}

	const imgData& DigitImage(char digit) {
		static const char* keys[] = {
			"play/Font/asset_30", "play/Font/asset_21", "play/Font/asset_22",
			"play/Font/asset_23", "play/Font/asset_24", "play/Font/asset_25",
			"play/Font/asset_26", "play/Font/asset_27", "play/Font/asset_28", "play/Font/asset_29"
		};
		return Skin::GetTexture(keys[digit - '0']);
	}

	// 白数字を中央揃えで描く。拡大は数字の下端を基準にする（縦に弾む）。
	void DrawDigits(int value, double centerX, double baseY, double height, double scaleX, double scaleY) {
		const std::string digits = std::to_string(value);
		double total = 0.0;
		for (const char d : digits) { const auto& img = DigitImage(d); total += img.w * height / img.h; }
		double x = centerX - total * scaleX / 2.0;
		for (const char d : digits) {
			const auto& img = DigitImage(d);
			const double w = img.w * height / img.h * scaleX;
			const double ext = height / img.h;
			DrawRotaGraph3F(float(x + w / 2.0), float(baseY), float(img.w / 2.0), float(img.h),
				ext * scaleX, ext * scaleY, 0.0, img.handle, TRUE);
			x += w;
		}
	}

	const char* JudgeTextKey(JudgeType type) {
		switch (type) {
		case JudgeType::GOOD: return "play/Font/asset_34";	// 良
		case JudgeType::OK:   return "play/Font/asset_35";	// 可
		default:              return "play/Font/asset_36";	// 不可（叩かずに通過した場合も不可）
		}
	}
	Rgb JudgeTextColor(JudgeType type) {
		switch (type) {
		case JudgeType::GOOD: return rollColor;
		case JudgeType::OK:   return white;
		default:              return Rgb{ 150, 170, 230 };
		}
	}
}

void PlayEffects::Reset() {
	processedJudgeLogs = 0;
	bursts.clear();
	laneFlashes.clear();
	judgePop.age = 1e9;
	combo = 0;
	comboAge = milestoneAge = brokenAge = 1e9;
	brokenCombo = 0;
}

void PlayEffects::TriggerLaneFlash(NoteType noteType) {
	HitColor color = (noteType == NoteType::Katsu) ? HitColor::Katsu : HitColor::Don;
	laneFlashes.push_back({ color, 0.0 });
	if (laneFlashes.size() > 8) {
		laneFlashes.erase(laneFlashes.begin(), laneFlashes.end() - 8);
	}
}

void PlayEffects::OnJudge(const ChartData& cd, const JudgeLog& log) {
	const double spin = (processedJudgeLogs * 37 % 360) * kPi / 180.0;
	switch (log.type) {
	case JudgeType::GOOD:
	case JudgeType::OK: {
		judgePop = { log.type, 0.0 };
		// 判定ログにはノーツの種類が無いので、直前に判定されたドン/カツを探して色を決める
		HitColor color = HitColor::Don;
		bool big = false;
		if (!cd.notes.empty()) {
			size_t i = (std::min)(cd.nextNoteIndex, cd.notes.size() - 1);
			for (int steps = 0; steps < 16; ++steps) {
				const Note& note = cd.notes[i];
				if ((note.type == NoteType::Don || note.type == NoteType::Katsu) && note.isJudged) {
					color = note.type == NoteType::Don ? HitColor::Don : HitColor::Katsu;
					big = note.isBig;
					break;
				}
				if (i == 0) break;
				--i;
			}
		}
		bursts.push_back({ color, big, log.type == JudgeType::GOOD, 0.0, spin });
		// 手動時は打鍵で発光済みなので、判定からの発光はオートプレイ時のみ
		if (cd.autoPlay) {
			laneFlashes.push_back({ color, 0.0 });
		}
		break;
	}
	case JudgeType::BAD:
	case JudgeType::MISS:
		judgePop = { log.type, 0.0 };
		break;
	case JudgeType::ROLLHIT:
	case JudgeType::BALLOONHIT:
		bursts.push_back({ HitColor::Roll, false, false, 0.0, spin });
		if (cd.autoPlay) {
			laneFlashes.push_back({ HitColor::Roll, 0.0 });
		}
		break;
	case JudgeType::BALLOONCLEAR:
		bursts.push_back({ HitColor::Roll, true, true, 0.0, spin });
		if (cd.autoPlay) {
			laneFlashes.push_back({ HitColor::Roll, 0.0 });
		}
		break;
	}
	// 連打中に増えすぎないよう、古いものから捨てる
	if (bursts.size() > 24) bursts.erase(bursts.begin(), bursts.end() - 24);
	if (laneFlashes.size() > 8) laneFlashes.erase(laneFlashes.begin(), laneFlashes.end() - 8);
}

void PlayEffects::Update(const ChartData& cd, double dtSec) {
	dtSec = std::clamp(dtSec, 0.0, 0.1);

	// リスタート（判定ログが消えた）を検出したら演出も初期化
	if (cd.judgelogs.size() < processedJudgeLogs) Reset();
	for (; processedJudgeLogs < cd.judgelogs.size(); ++processedJudgeLogs) {
		OnJudge(cd, cd.judgelogs[processedJudgeLogs]);
	}

	if (cd.combo > combo) {
		comboAge = 0.0;
		if (cd.combo / 100 > combo / 100) milestoneAge = 0.0;
	}
	else if (cd.combo < combo) {
		if (combo >= 10) { brokenCombo = combo; brokenAge = 0.0; }
		comboAge = milestoneAge = 1e9;
	}
	combo = cd.combo;

	for (auto& b : bursts) b.age += dtSec;
	for (auto& f : laneFlashes) f.age += dtSec;
	judgePop.age += dtSec;
	comboAge += dtSec;
	milestoneAge += dtSec;
	brokenAge += dtSec;

	std::erase_if(bursts, [](const HitBurst& b) {
		return b.age >= (b.color == HitColor::Roll && !b.big ? rollBurstTime : burstTime);
	});
	std::erase_if(laneFlashes, [](const LaneFlash& f) { return f.age >= laneFlashTime; });
}

void PlayEffects::DrawLane() const {
	// 叩いた色でレーンが光り、判定枠から右へ向かって薄くなる
	constexpr int bands = 10;
	for (const auto& f : laneFlashes) {
		const Rgb c = f.color == HitColor::Don ? donColor : f.color == HitColor::Katsu ? katsuColor : rollColor;
		const double strength = (1.0 - Clamp01(f.age / laneFlashTime)) * (f.color == HitColor::Roll ? 0.25 : 0.45);
		for (int i = 0; i < bands; ++i) {
			SetAlpha(strength * (1.0 - double(i) / bands));
			DrawBox(laneX + laneW * i / bands, laneY, laneX + laneW * (i + 1) / bands, laneY + laneH, ToColor(c), TRUE);
		}
	}
	ResetDrawState();
}

void PlayEffects::DrawOverlay() const {
	const auto& star = Skin::GetTexture("play/Poster/asset_19");	// 白い星（赤い縁）
	const auto& streak = Skin::GetTexture("play/Poster/asset_12");	// 白い流線
	const auto& dot = Skin::GetTexture("play/Poster/asset_16");		// 黄色い円（赤い縁）

	// ── ヒット効果 ──
	for (const auto& b : bursts) {
		if (b.color == HitColor::Roll && !b.big) {
			// 連打：小さな黄色い円が弾けて消える
			const double t = b.age / rollBurstTime, e = EaseOutCubic(t);
			const double ox = std::cos(b.spin) * 26.0 * e, oy = std::sin(b.spin) * 26.0 * e;
			SetAlpha(1.0 - t);
			DrawCentered(dot, judgeX + ox, judgeY + oy, 0.12 + 0.12 * e, 0.12 + 0.12 * e, 0.0);
			continue;
		}
		const double t = b.age / burstTime, e = EaseOutCubic(t);
		const Rgb c = b.color == HitColor::Don ? donColor : b.color == HitColor::Katsu ? katsuColor : rollColor;
		const double size = (b.big ? 1.3 : 1.0) * (b.good ? 1.0 : 0.75);

		// 広がる輪
		SetAlpha((1.0 - t) * 0.9);
		DrawCircleAA(float(judgeX), float(judgeY), float((62.0 + 58.0 * e) * size), 64,
			ToColor(c), FALSE, float(12.0 * (1.0 - t) + 1.0));

		// 放射する流線（良のみ）。細い先端が外を向くように回転させる
		if (b.good) {
			for (int k = 0; k < 8; ++k) {
				const double angle = b.spin + k * kPi / 4.0;
				const double r = (70.0 + 75.0 * e) * size;
				SetAlpha(std::pow(1.0 - t, 1.5));
				SetTint(k % 2 == 0 ? white : c);
				const double s = 0.1 * size * (1.0 - 0.5 * t);
				DrawCentered(streak, judgeX + std::cos(angle) * r, judgeY + std::sin(angle) * r, s, s, angle + kPi);
			}
			SetTint(white);
		}

		// 回転しながら開く星
		const double starScale = (0.18 + 0.16 * e) * size;
		SetAlpha(t < 0.4 ? 1.0 : 1.0 - (t - 0.4) / 0.6);
		DrawCentered(star, judgeX, judgeY, starScale, starScale, b.spin + e * 0.6);
	}
	ResetDrawState();

	// ── 判定文字（良・可・不可）──
	if (judgePop.age < judgeTime) {
		const auto& img = Skin::GetTexture(JudgeTextKey(judgePop.type));
		const double t = judgePop.age;
		const double pop = 1.0 + 0.35 * (1.0 - EaseOutCubic(t / 0.08));
		const double y = judgeTextY - 14.0 * EaseOutCubic(t / judgeTime);
		const double alpha = t < 0.35 ? 1.0 : 1.0 - (t - 0.35) / (judgeTime - 0.35);
		const double ext = judgeTextHeight / img.h * pop;
		// 背景が派手なので、レーンと同じ紺色で縁取りする
		static const int offsets[][2] = { {3,0},{-3,0},{0,3},{0,-3},{2,2},{-2,2},{2,-2},{-2,-2} };
		SetAlpha(alpha);
		SetTint(outlineColor);
		for (const auto& o : offsets) DrawCentered(img, judgeX + o[0], y + o[1], ext, ext, 0.0);
		SetTint(JudgeTextColor(judgePop.type));
		DrawCentered(img, judgeX, y, ext, ext, 0.0);
		ResetDrawState();
	}

	// ── コンボ ──
	const auto& frame = Skin::GetTexture("play/Poster/asset_09");	// 青い帯（HUD 案と同じ）
	const double frameH = PairedHeight(frame, comboW);
	const double digitH = comboW * 0.13;
	const double centerX = comboX + comboW / 2.0, centerY = comboY + frameH / 2.0;
	const double baseY = centerY + digitH / 2.0;

	// 100コンボごと：枠の後ろで星が開き、枠が弾む
	double frameScale = 1.0;
	if (milestoneAge < milestoneTime) {
		const double t = milestoneAge / milestoneTime, e = EaseOutCubic(t);
		SetAlpha(1.0 - t);
		DrawCentered(star, centerX, centerY, 0.15 + 0.45 * e, 0.15 + 0.45 * e, e * 0.8);
		ResetDrawState();
		frameScale = 1.0 + 0.08 * (1.0 - e);
	}
	DrawPaired(frame, comboX, comboY, comboW, frameScale);

	if (combo > 0) {
		const double p = 1.0 - EaseOutCubic(comboAge / comboPopTime);
		if (milestoneAge < milestoneTime) SetTint(rollColor);
		DrawDigits(combo, centerX, baseY, digitH, 1.0 + 0.1 * p, 1.0 + 0.3 * p);
		SetTint(white);
	}
	// 途切れたコンボは下へ落ちて消える
	if (brokenAge < brokenTime) {
		const double t = brokenAge / brokenTime;
		SetAlpha(1.0 - t);
		SetTint(Rgb{ 150, 170, 230 });
		DrawDigits(brokenCombo, centerX, baseY + 40.0 * t * t, digitH, 1.0, 1.0);
	}
	ResetDrawState();
}
