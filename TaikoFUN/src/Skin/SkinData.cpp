#include "SkinData.h"
#include "DxLib.h"
#include "Core/ChartData.h"

#include <memory>
#include <cassert>
#include <algorithm>
#include <array>
#include <cmath>



namespace {
	// Recolor the existing sprite pixels once at load time. Keep alpha, black
	// outlines/shadows and the original antialiasing between fill and border.
	void ApplyPosterNotePalette(BASEIMAGE& image) {
		using Color = std::array<double, 3>;
		const Color sourceBorder{ 255, 255, 240 };
		const Color white{ 255, 255, 255 };
		const auto dot = [](const Color& a, const Color& b) {
			return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
		};
		const int cellWidth = image.Width / 15;
		for (int cell = 1; cell <= 10; ++cell) {
			const bool don = cell == 1 || cell == 3;
			const bool katsu = cell == 2 || cell == 4;
			const Color sourceFill = don ? Color{ 255, 65, 40 }
				: katsu ? Color{ 40, 176, 255 } : Color{ 255, 201, 40 };
			const Color targetFill = don ? Color{ 218, 0, 16 } // #DA0010
				: katsu ? Color{ 0, 134, 174 }                 // #0086AE
				: Color{ 255, 214, 0 };                       // #FFD600
			const double ff = dot(sourceFill, sourceFill);
			const double fb = dot(sourceFill, sourceBorder);
			const double bb = dot(sourceBorder, sourceBorder);
			const double determinant = ff * bb - fb * fb;
			for (int y = 0; y < image.Height; ++y) {
				for (int x = cell * cellWidth; x < (cell + 1) * cellWidth; ++x) {
					int r, g, b, alpha;
					GetPixelBaseImage(&image, x, y, &r, &g, &b, &alpha);
					if (alpha == 0) continue;
					const Color pixel{ double(r), double(g), double(b) };
					const double pf = dot(pixel, sourceFill);
					const double pb = dot(pixel, sourceBorder);
					const double fill = std::clamp((pf * bb - pb * fb) / determinant, 0.0, 1.0);
					const double border = std::clamp((pb * ff - pf * fb) / determinant, 0.0, 1.0);
					const auto channel = [&](int index) {
						return std::clamp(int(std::lround(fill * targetFill[index] + border * white[index])), 0, 255);
					};
					SetPixelBaseImage(&image, x, y, channel(0), channel(1), channel(2), alpha);
				}
			}
		}
	}

	class SkinData
	{

		std::unordered_map<std::string, std::unique_ptr<imgData>> imgs;
		std::unordered_map<std::string, std::unique_ptr<SoundHandle>> sounds;

	public:

		void LoadSkin();

		void Unload();

		imgData& GetTexture(const std::string& key);
		SoundHandle& GetSound(const std::string& key);


	private:
		void LoadPlaySceneSkin();
		void LoadNotesImgs(const std::string& path);

		void LoadResultSceneSkin();
		void LoadSongSelectSceneSkin();

		void LoadSoundData();

		void emplaceImg(std::string key, std::string path);
		void emplaceImg(std::string key, int handle, int w, int h, std::string path);
		void emplaceSnd(std::string key, std::string path);

	};

	SkinData g_SkinData;
}

void SkinData::emplaceImg(std::string key, std::string path) {
	imgs.emplace(key, std::make_unique<imgData>(path));

}

void SkinData::emplaceImg(std::string key, int handle, int w, int h, std::string path) {
	imgs.emplace(key, std::make_unique<imgData>(handle, w, h, path));
}

void SkinData::emplaceSnd(std::string key, std::string path) {
	sounds.emplace(key, std::make_unique<SoundHandle>(path));
}

void SkinData::Unload() {
	imgs.clear();
	sounds.clear();
}

void SkinData::LoadSkin() {
	Unload();	// 既存のデータを開放
	


	LoadPlaySceneSkin(); // playシーンのスキン読み込み

	LoadNotesImgs("Resource/Image/Playing/note.png");	// ノーツ画像の読み込み (分割読み込み)



	LoadSoundData();








}


void SkinData::LoadPlaySceneSkin() {
	
	emplaceImg( "play/bg", "Resource/Image/SkinColor/bgstage.png" );
	emplaceImg("play/bg_clear", "Resource/Image/SkinColor/bgstage_clear.png");
	emplaceImg("play/minitaiko", "Resource/Image/SkinColor/minitaiko.png");
	emplaceImg("play/ScrollField/bg", "Resource/Image/Playing/scrollfield_bg.png");
	emplaceImg("play/ScrollField/don", "Resource/Image/Playing/scrollfield_don.png");
	emplaceImg("play/ScrollField/katsu", "Resource/Image/Playing/scrollfield_ka.png");
	emplaceImg("play/ScrollField/hit", "Resource/Image/Playing/scrollfield_hit.png");

	// Register the 27 transparent poster assets once, outside Draw.
	for (int assetIndex = 1; assetIndex <= 27; ++assetIndex) {
		const std::string assetName = "asset_" + std::string(assetIndex < 10 ? "0" : "")
			+ std::to_string(assetIndex);
		emplaceImg("play/Poster/" + assetName, "Resource/Image/Poster/" + assetName + ".png");
	}


}

void SkinData::LoadNotesImgs(const std::string& path) {
	BASEIMAGE image{};
	if (CreateBaseImageToFile(path.c_str(), &image) < 0) {
		assert(false && "Failed to load note sprite sheet");
		return;
	}
	// White song-title logos (06-10) and digits (21-30), rendered from SVG.
	for (int assetIndex = 6; assetIndex <= 30; ++assetIndex) {
		if (assetIndex > 10 && assetIndex < 21) continue;
		const std::string assetName = "asset_" + std::string(assetIndex < 10 ? "0" : "")
			+ std::to_string(assetIndex);
		emplaceImg("play/Font/" + assetName, "Resource/Font/PNG/" + assetName + ".png");
	}

	int handles[15];
	int DivNum = 15;
	int DivX = 15;
	int DivY = 1;
	int XSize = image.Width / DivX;
	int YSize = image.Height / DivY;
	ApplyPosterNotePalette(image);
	const int result = CreateDivGraphFromBaseImage(&image, DivNum, DivX, DivY, XSize, YSize, handles);
	ReleaseBaseImage(&image);
	if (result < 0) {
		assert(false && "Failed to create note textures");
		return;
	}
	DeleteGraph(handles[14]); // The final cell in the sheet is unused.

	emplaceImg("note/Judgeframe",	handles[0], XSize, YSize, path);			// 判定枠
	emplaceImg("note/Don",			handles[1], XSize, YSize, path);			// ドン
	emplaceImg("note/Katsu",		handles[2], XSize, YSize, path);			// カッ
	emplaceImg("note/BigDon",		handles[3], XSize, YSize, path);			// 大ドン
	emplaceImg("note/BigKatsu",		handles[4], XSize, YSize, path);			// 大カッ
	emplaceImg("note/RollHead",		handles[5], XSize, YSize, path);			// 連打(頭)
	emplaceImg("note/RollBody",		handles[6], XSize, YSize, path);			// 連打(体)
	emplaceImg("note/RollTail",		handles[7], XSize, YSize, path);			// 連打(尾)
	emplaceImg("note/BigRollHead",	handles[8], XSize, YSize, path);			// 大連打(頭)
	emplaceImg("note/BigRollBody",	handles[9], XSize, YSize, path);			// 大連打(体)
	emplaceImg("note/BigRollTail",	handles[10], XSize, YSize, path);			// 大連打(尾)
	emplaceImg("note/BalloonHead",	handles[11], XSize, YSize, path);			// 風船(頭)
	emplaceImg("note/BalloonTail",	handles[12], XSize, YSize, path);			// 風船(体)
	emplaceImg("note/Kusudama",		handles[13], XSize, YSize, path);			// くす玉

}




void SkinData::LoadSoundData() {
	emplaceSnd("Don", ("Resource/Sound/General/don.wav"));
	emplaceSnd("Katsu", ("Resource/Sound/General/ka.wav"));
}
/*
imgData& SkinData::GetTexture(const std::string& key) {
	return *imgs.at(key);
}
*/
imgData& SkinData::GetTexture(const std::string& key) {
	auto it = imgs.find(key);
	if (it == imgs.end()) {
		// エラーログ（デバッグ用）
		OutputDebugString(("Missing skin key: " + key + "\n").c_str());
		std::string errormsg = "Missing skin key: " + key;
		assert(false && errormsg.c_str());
		// フォールバック: 存在しない場合はデフォルト imgData を挿入して返す
		imgs.emplace(key, std::make_unique<imgData>()); // handle=-1 のプレースホルダ
		return *imgs.at(key);
	}
	return *it->second;
}

SoundHandle& SkinData::GetSound(const std::string& key) {
	return *sounds.at(key);
}

namespace Skin {
	void loadSkin() {
		g_SkinData.LoadSkin();
	}

	imgData& GetTexture(const std::string& key){

		return g_SkinData.GetTexture(key);
	}

	SoundHandle& GetSound(const std::string& key) {
		return g_SkinData.GetSound(key);
	}

	std::string GetNoteImageKey(NoteType type, bool isBig) {
		switch (type) {
		case NoteType::Don:		return (!isBig) ? "note/Don" : "note/BigDon";
		case NoteType::Katsu:	return (!isBig) ? "note/Katsu" : "note/BigKatsu";
		case NoteType::Judge:	return "note/Judgeframe";
		}
		return "";
	}

	std::string GetRollImageKey(RollPart part, bool isBig) {
		switch (part) {
		case RollPart::Head: return (!isBig) ? "note/RollHead" : "note/BigRollHead";
		case RollPart::Body: return (!isBig) ? "note/RollBody" : "note/BigRollBody";
		case RollPart::Tail: return (!isBig) ? "note/RollTail" : "note/BigRollTail";
		}

	}

	std::string GetBalloonImageKey(RollPart part) {
		switch (part) {
		case RollPart::Head: return "note/BalloonHead";
		case RollPart::Tail: return "note/BalloonTail";
		}
	}
}
