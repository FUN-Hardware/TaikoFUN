#include "ChartDrawer.h"

#include "DxLib.h"
#include "Scene/Play/Logic/ChartPlayer.h"
#include "SkinLayout.h"
#include "Skin/SkinData.h"

#include <algorithm>
#include <cmath>

namespace {
	const char* WhiteSongTitleKey(const std::string& title, const std::string& chartPath) {
		if (title == "シャイニングスター") return "play/Font/asset_06";
		if (title == "8OROCHI(裏)") return "play/Font/asset_07";
		if (title == "ドンカマ2000") return "play/Font/asset_08";
		if (title == "万戈イムー一ノ十") return "play/Font/asset_09";
		if (title == "続・〆ドレー2000") return "play/Font/asset_10";
		// This chart's title can still be Shift-JIS; its ASCII path identifies it.
		if (chartPath.find("8OROCHI") != std::string::npos
			|| chartPath.find("8orochi-ura") != std::string::npos) return "play/Font/asset_07";
		return nullptr;
	}

	float WhiteFontWidth(const imgData& image, int height) {
		return float(image.w) * height / image.h;
	}

	void DrawWhiteFontAsset(float x, float y, int height, const imgData& image) {
		const int previousDrawMode = GetDrawMode();
		SetDrawMode(DX_DRAWMODE_BILINEAR);
		DrawExtendGraphF(x, y, x + WhiteFontWidth(image, height), y + height, image.handle, TRUE);
		SetDrawMode(previousDrawMode);
	}

	void DrawWhiteNumberRight(int right, int y, int height, const std::string& digits) {
		static const char* keys[] = {
			"play/Font/asset_30", "play/Font/asset_21", "play/Font/asset_22",
			"play/Font/asset_23", "play/Font/asset_24", "play/Font/asset_25",
			"play/Font/asset_26", "play/Font/asset_27", "play/Font/asset_28", "play/Font/asset_29"
		};
		float x = float(right);
		for (const char digit : digits) x -= WhiteFontWidth(Skin::GetTexture(keys[digit - '0']), height);
		for (const char digit : digits) {
			const auto& image = Skin::GetTexture(keys[digit - '0']);
			DrawWhiteFontAsset(x, float(y), height, image);
			x += WhiteFontWidth(image, height);
		}
	}
}

ChartDrawer::ChartDrawer(ChartData& cd, PlayEffects& effects)
	: cd_(cd), effects_(effects),
	  bg_(std::make_unique<BackgroundRenderer>()),
	  nr_(std::make_unique<NoteRenderer>(cd)) {
}

void ChartDrawer::Init() {
	bg_->Init();
}

void ChartDrawer::Update(const ChartPlayer& player) {
	bg_->Update(player);
}

void ChartDrawer::Draw(const ChartPlayer& player) {
	bg_->Draw(player);

	const auto& lane = Skin::GetTexture("play/ScrollField/bg");
	DrawBox(SkinLayout::ScrollField.x, SkinLayout::ScrollField.y,
		SkinLayout::ScrollField.x + lane.w, SkinLayout::ScrollField.y + lane.h,
		GetColor(32, 42, 82), TRUE);
	effects_.DrawLane();
	effects_.DrawHitEffects();
	effects_.DrawGogoFire(player);

	// ノーツをレーン内に収め、左のミニ太鼓に重ねない。
	RECT previousDrawArea{};
	GetDrawArea(&previousDrawArea);
	const int clipLeft = (std::max)(int(previousDrawArea.left), int(SkinLayout::ScrollField.x));
	const int clipTop = (std::max)(int(previousDrawArea.top), int(SkinLayout::ScrollField.y));
	const int clipRight = (std::min)(int(previousDrawArea.right), int(SkinLayout::ScrollField.x + lane.w));
	const int clipBottom = (std::min)(int(previousDrawArea.bottom), int(SkinLayout::ScrollField.y + lane.h));
	if (clipLeft < clipRight && clipTop < clipBottom) {
		SetDrawArea(clipLeft, clipTop, clipRight, clipBottom);

		DrawExtendGraph(SkinLayout::ScrollField.x, SkinLayout::ScrollField.y,
			SkinLayout::ScrollField.x + SkinLayout::NoteSize,
			SkinLayout::ScrollField.y + SkinLayout::NoteSize,
			Skin::GetTexture(Skin::GetNoteImageKey(NoteType::Judge, false)).handle, TRUE);

		nr_->Draw();
	}
	SetDrawArea(previousDrawArea.left, previousDrawArea.top, previousDrawArea.right, previousDrawArea.bottom);
	effects_.DrawMiniDrum();
	effects_.DrawOverlay();

	// 白文字の曲名。専用画像のない曲は既存フォントで表示する。
	if (const char* titleKey = WhiteSongTitleKey(cd_.Title, cd_.tjaPath)) {
		const auto& image = Skin::GetTexture(titleKey);
		const int height = GetFontSize();
		if (image.handle >= 0 && image.w > 0 && image.h > 0) {
			DrawWhiteFontAsset(0.0f, 100.0f, height, image);
		}
	} else {
		DrawFormatString(0, 100, GetColor(255, 255, 255), "%s", cd_.Title.c_str());
	}

	int width = 0, height = 0;
	GetWindowSize(&width, &height);
	const auto stats = player.getPlayStats();
	if (stats.rollHitCount > 0) {
		DrawWhiteNumberRight(width, 0, GetFontSize(), std::to_string(stats.rollHitCount));
	}
}
