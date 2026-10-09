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
	constexpr double burstTime = 0.24, rollBurstTime = 0.18, laneFlashTime = 0.18;
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
		default:              return "play/Font/asset_36";	// 不可（叩いた場合のみ。MISSは表示しない）
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
	processedJudgeEvents = 0;
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

void PlayEffects::OnJudge(const ChartPlayer& player, const JudgeEvent& event) {
	const double spin = (processedJudgeEvents * 37 % 360) * kPi / 180.0;
	switch (event.judgeType) {
	case JudgeType::GOOD:
	case JudgeType::OK: {
		judgePop = { event.judgeType, 0.0 };
		const HitColor color = event.noteType == NoteType::Katsu ? HitColor::Katsu : HitColor::Don;
		const bool big = event.isBig;
		bursts.push_back({ color, big, event.judgeType == JudgeType::GOOD, 0.0, spin });
		// 手動時は入力から発光するので、判定からの発光はオートプレイ時のみ
		if (player.isAutoplay()) {
			laneFlashes.push_back({ color, 0.0 });
		}
		break;
	}
	case JudgeType::BAD:
		judgePop = { event.judgeType, 0.0 };
		break;
	case JudgeType::MISS:
		// 見逃しでは判定文字を出さない。直前の文字も消して誤解を防ぐ。
		judgePop.age = 1e9;
		break;
	case JudgeType::ROLLHIT:
	case JudgeType::BALLOONHIT:
		bursts.push_back({ HitColor::Roll, false, false, 0.0, spin });
		if (player.isAutoplay()) {
			laneFlashes.push_back({ HitColor::Roll, 0.0 });
		}
		break;
	case JudgeType::BALLOONCLEAR:
		bursts.push_back({ HitColor::Roll, true, true, 0.0, spin });
		if (player.isAutoplay()) {
			laneFlashes.push_back({ HitColor::Roll, 0.0 });
		}
		break;
	}
	// 連打中に増えすぎないよう、古いものから捨てる
	if (bursts.size() > 8) bursts.erase(bursts.begin(), bursts.end() - 8);
	if (laneFlashes.size() > 8) laneFlashes.erase(laneFlashes.begin(), laneFlashes.end() - 8);
}

void PlayEffects::Update(const ChartPlayer& player, double dtSec) {
	dtSec = std::clamp(dtSec, 0.0, 0.1);

	// 再開時は過去の演出を消し、同じフレームの判定イベントはすべて受け取る。
	if (playbackGeneration != player.getPlaybackGeneration()) {
		Reset();
		playbackGeneration = player.getPlaybackGeneration();
	}
	for (const auto& event : player.getJudgeEvents()) {
		OnJudge(player, event);
		++processedJudgeEvents;
	}

	const int currentCombo = player.getPlayStats().combo;
	if (currentCombo > combo) {
		comboAge = 0.0;
		if (currentCombo / 100 > combo / 100) milestoneAge = 0.0;
	}
	else if (currentCombo < combo) {
		if (combo >= 10) { brokenCombo = combo; brokenAge = 0.0; }
		comboAge = milestoneAge = 1e9;
	}
	combo = currentCombo;

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

void PlayEffects::DrawHitEffects() const {
	const auto& star = Skin::GetTexture("play/Poster/asset_19");	// 白い星（赤い縁）
	const auto& streak = Skin::GetTexture("play/Poster/asset_12");	// 白い流線
	const auto& dot = Skin::GetTexture("play/Poster/asset_16");		// 黄色い円（赤い縁）

	// ── ヒット効果 ──
	for (const auto& b : bursts) {
		if (b.color == HitColor::Roll && !b.big) {
			// 連打：小さな黄色い円が弾けて消える
			const double t = b.age / rollBurstTime, e = EaseOutCubic(t);
			const double ox = std::cos(b.spin) * 26.0 * e, oy = std::sin(b.spin) * 26.0 * e;
			SetAlpha((1.0 - t) * 0.65);
			DrawCentered(dot, judgeX + ox, judgeY + oy, 0.08 + 0.06 * e, 0.08 + 0.06 * e, 0.0);
			continue;
		}
		const double t = b.age / burstTime, e = EaseOutCubic(t);
		const Rgb c = b.color == HitColor::Don ? donColor : b.color == HitColor::Katsu ? katsuColor : rollColor;
		const double size = (b.big ? 1.15 : 1.0) * (b.good ? 1.0 : 0.75);

		// 広がる輪
		SetAlpha((1.0 - t) * 0.6);
		DrawCircleAA(float(judgeX), float(judgeY), float((42.0 + 26.0 * e) * size), 64,
			ToColor(c), FALSE, float(6.0 * (1.0 - t) + 1.0));

		// 放射する流線（良のみ）。細い先端が外を向くように回転させる
		if (b.good) {
			for (int k = 0; k < 4; ++k) {
				const double angle = b.spin + k * kPi / 2.0;
				const double r = (48.0 + 28.0 * e) * size;
				SetAlpha(std::pow(1.0 - t, 1.5) * 0.55);
				SetTint(k % 2 == 0 ? white : c);
				const double s = 0.06 * size * (1.0 - 0.5 * t);
				DrawCentered(streak, judgeX + std::cos(angle) * r, judgeY + std::sin(angle) * r, s, s, angle + kPi);
			}
			SetTint(white);
		}

		// 回転しながら開く星
		const double starScale = (0.12 + 0.08 * e) * size;
		SetAlpha((t < 0.3 ? 1.0 : 1.0 - (t - 0.3) / 0.7) * 0.7);
		DrawCentered(star, judgeX, judgeY, starScale, starScale, b.spin + e * 0.6);
	}
	ResetDrawState();

	// 100コンボの星もノーツより下へ。コンボ枠の近くに収める。
	if (milestoneAge < milestoneTime) {
		const auto& frame = Skin::GetTexture("play/Poster/asset_09");
		const double centerX = comboX + comboW / 2.0;
		const double centerY = comboY + PairedHeight(frame, comboW) / 2.0;
		const double t = milestoneAge / milestoneTime, e = EaseOutCubic(t);
		SetAlpha((1.0 - t) * 0.7);
		DrawCentered(star, centerX, centerY, 0.10 + 0.15 * e, 0.10 + 0.15 * e, e * 0.8);
		ResetDrawState();
	}

}

void PlayEffects::DrawMiniDrum() const {
	// 円形素材を打面、青い帯と180度回転した複製を胴にする。
	// レーン始点x=325の手前（x=192〜312）に収める。
	const auto& body = Skin::GetTexture("play/Poster/asset_09");
	const auto& face = Skin::GetTexture("play/Poster/asset_16");
	constexpr double x = 246.0, y = judgeY;
	DrawCentered(body, 282.0, y, 60.0 / body.w, 96.0 / body.h, 0.0);
	DrawCentered(body, 282.0, y, 60.0 / body.w, 96.0 / body.h, kPi);
	DrawCentered(face, x, y, 108.0 / face.w, 108.0 / face.h, 0.0);
	DrawCircleAA(float(x), float(y), 44.0f, 64, GetColor(255, 250, 235), TRUE);
	DrawCircleAA(float(x), float(y), 39.0f, 64, ToColor(outlineColor), FALSE, 1.0f);
	DrawLine(int(x), int(y - 39), int(x), int(y + 39), ToColor(outlineColor), 1);
	ResetDrawState();
}

void PlayEffects::DrawOverlay() const {

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

	// 100コンボごと：枠が弾む。星はDrawHitEffectsでノーツより下に描く。
	double frameScale = 1.0;
	if (milestoneAge < milestoneTime) {
		const double e = EaseOutCubic(milestoneAge / milestoneTime);
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
