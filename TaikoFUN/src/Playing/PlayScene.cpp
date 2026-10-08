#include "DxLib.h"
#include "PlayScene.h"
#include "Debug/FPS.h"

#include "Input/Input.h"
#include "Skin/SkinData.h"
#include "Core/General.h"
#include "Core/ChartScanner.h"
#include "File/ChartLoader.h"
#include "File/FindAllTJA.h"
#include "Core/text.h"

#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>

namespace SkinLayout {

	Vector2d ScrollField{ 325, 254 - 65 };
	int NoteSize = 128;

};
namespace fs = std::filesystem;

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


namespace {
    // Provisional settings approved in the motion preview, in 1280 x 720 units.
    constexpr double posterWidth = 1280.0, posterHeight = 720.0;
    constexpr double posterPi = 3.14159265358979323846;
    constexpr double posterDelay = 3.0, posterRampEnd = 130.0;
    constexpr double posterInitialRatio = 0.05, posterSpacing = 900.0;
    constexpr double posterTurnWidth = 180.0;
    constexpr double posterLayerSpeeds[] = {80.0, 120.0, 160.0, 200.0};
    constexpr size_t posterArcSegments = 4096;
    struct PosterPoint { double x, y; };
    constexpr PosterPoint posterCenters[] = {{1350.0, 1000.0}, {1480.0, 1120.0}, {1650.0, 1380.0}};
    struct PosterObject {
        const char* textureKey;
        double centerX, centerY, scale, angle;
        double width, height;
        int group, layer;
        bool linear;
    };
    // Initial Illustrator placement; row order is retained within each layer.
    constexpr PosterObject posterObjects[] = {
		{ "play/Poster/asset_15", 1034.195, 109.651, 0.2995909, -0.0003670, 444.5929, 193.8353, 0, 1, false },
		{ "play/Poster/asset_21", 961.019, 330.086, 0.2992947, 0.0002969, 444.1533, 193.6437, 1, 1, false },
		{ "play/Poster/asset_20", 1108.183, 548.989, 0.3029486, 0.0041856, 453.8170, 200.2490, 2, 0, false },
		{ "play/Poster/asset_21", 1226.996, 689.222, 0.2992947, 0.0002969, 444.1533, 193.6437, 2, 1, false },
		{ "play/Poster/asset_14", 615.113, 20.317, 0.2999821, -0.0003110, 294.8824, 143.9914, 0, 1, false },
		{ "play/Poster/asset_11", 986.058, 366.132, 0.3013538, 0.0002829, 396.5816, 129.2808, 1, 2, false },
		{ "play/Poster/asset_01", 36.939, 224.976, 0.2997734, 0.0000913, 299.7734, 410.0900, 0, 1, false },
		{ "play/Poster/asset_09", 434.234, 352.713, 0.3007351, -0.0001660, 396.0681, 129.0154, 1, 0, false },
		{ "play/Poster/asset_03", 130.547, 453.533, 0.2995656, -0.0000136, 340.9057, 451.4454, 0, 1, false },
		{ "play/Poster/asset_04", 101.423, 697.673, 0.2992135, 0.0000295, 259.1189, 312.6781, 0, 1, false },
		{ "play/Poster/asset_05", 107.632, 755.024, 0.3007861, 0.0009299, 203.6322, 371.4708, 0, 1, false },
		{ "play/Poster/asset_02", 291.529, 167.828, 0.2980859, 0.0005070, 292.7204, 77.8004, 0, 1, false },
		{ "play/Poster/asset_27", 447.360, 541.580, 0.2990149, -0.0000342, 393.5036, 128.5764, 1, 0, false },
		{ "play/Poster/asset_24", 588.274, 658.577, 0.2994814, -0.0000756, 344.1041, 78.1646, 1, 1, false },
		{ "play/Poster/asset_07", 403.644, 704.668, 0.2995873, 0.0003515, 165.6718, 360.1039, 0, 0, false },
		{ "play/Poster/asset_08", 532.207, 439.459, 0.3003504, -0.0001426, 300.3504, 385.3496, 1, 1, false },
		{ "play/Poster/asset_26", 502.177, 782.204, 0.2988821, 0.0005257, 258.8319, 312.6307, 1, 1, false },
		{ "play/Poster/asset_22", 863.131, 677.467, 0.3004110, 0.0002281, 395.3409, 128.8763, 2, 1, false },
		{ "play/Poster/asset_25", 612.105, 776.522, 0.3101166, 0.0006123, 215.2209, 387.0255, 1, 1, false },
		{ "play/Poster/asset_12", 482.569, 253.995, 0.2824497, -0.0071309, 279.9077, 51.9707, 1, 3, false },
		{ "play/Poster/asset_13", 645.810, 196.391, 0.4194045, -0.0238265, 100.2377, 21.3896, 0, 2, false },
		{ "play/Poster/asset_17", 735.711, 397.092, 0.3037163, 0.0010179, 317.6872, 413.3579, 1, 1, false },
		{ "play/Poster/asset_10", 233.093, 377.730, 0.2950039, 0.0000291, 289.9888, 49.8557, 0, 2, false },
		{ "play/Poster/asset_23", 723.077, 561.846, 0.3290511, 0.0000000, 90.1600, 90.1600, 1, 2, true },
		{ "play/Poster/asset_23", 836.843, 462.325, 0.3117810, 0.0000000, 85.4280, 85.4280, 1, 2, true },
		{ "play/Poster/asset_23", 941.817, 370.495, 0.3003978, 0.0000000, 82.3090, 82.3090, 1, 2, true },
		{ "play/Poster/asset_23", 1036.721, 287.474, 0.2936350, 0.0000000, 80.4560, 80.4560, 1, 2, true },
		{ "play/Poster/asset_16", 1119.608, 214.966, 0.3053002, 0.0000000, 79.6834, 79.3781, 0, 2, true },
		{ "play/Poster/asset_16", 1186.978, 156.031, 0.3040449, 0.0000000, 79.3557, 79.0517, 0, 2, true },
		{ "play/Poster/asset_16", 1228.082, 120.074, 0.3038760, 0.0000000, 79.3116, 79.0078, 0, 2, true },
		{ "play/Poster/asset_19", 941.696, 369.034, 0.2831072, -0.0119363, 143.2522, 143.2522, 1, 3, true },
		{ "play/Poster/asset_06", 237.078, 739.845, 0.2982489, -0.0002463, 187.3003, 297.3542, 0, 1, false },
		{ "play/Poster/asset_16", 1281.828, 182.539, 0.3038760, 0.0000000, 79.3116, 79.0078, 0, 1, false },
		{ "play/Poster/asset_16", 1296.053, 251.391, 0.3038760, 0.0000000, 79.3116, 79.0078, 1, 1, false },
    };
    struct PosterOrbit {
        double centerX = 0.0, centerY = 0.0;
        double radiusX = 0.0, radiusY = 0.0, initialAngle = 0.0;
        double minimum = 0.0, length = 0.0;
        std::vector<double> arc;
    };
    struct PosterInstance { size_t sourceIndex; double offset; };
    struct PosterMotion {
        std::vector<PosterOrbit> orbits;
        std::vector<PosterInstance> instances;
        PosterPoint lineDirection;
    };
    double PosterWrap(double value, double length) {
        return std::fmod(std::fmod(value, length) + length, length);
    }
    PosterPoint PosterGuide(double t) {
        const double u = 1.0 - t;
        return {3*u*u*t*522.06 + 3*u*t*t*1043.28 + t*t*t*1280.0,
                3*u*t*t*233.93 + t*t*t*720.0};
    }
    double PosterGuideY(double x) {
        double lo = 0.0, hi = 1.0;
        for (int i = 0; i < 15; ++i) {
            const double t = (lo + hi) / 2.0;
            if (PosterGuide(t).x < x) lo = t; else hi = t;
        }
        return PosterGuide((lo + hi) / 2.0).y;
    }
    double PosterOrbitAngle(const PosterOrbit& orbit, double distance) {
        const double d = PosterWrap(distance, orbit.length);
        const auto upper = std::upper_bound(orbit.arc.begin(), orbit.arc.end(), d);
        const size_t lo = (std::min)(static_cast<size_t>(upper - orbit.arc.begin() - 1), posterArcSegments - 1);
        const double fraction = (d - orbit.arc[lo]) / (orbit.arc[lo + 1] - orbit.arc[lo]);
        return orbit.initialAngle + (lo + fraction) * (2.0 * posterPi / posterArcSegments);
    }
    PosterPoint PosterPosition(const PosterObject& object, const PosterOrbit& orbit,
                               const PosterPoint& direction, double distance) {
        if (object.linear) {
            const double shift = PosterWrap(distance - orbit.minimum, orbit.length) + orbit.minimum;
            return {object.centerX + direction.x * shift, object.centerY + direction.y * shift};
        }
        const double angle = PosterOrbitAngle(orbit, distance);
        return {orbit.centerX + orbit.radiusX * std::cos(angle),
                orbit.centerY + orbit.radiusY * std::sin(angle)};
    }
    double PosterDrawAngle(const PosterObject& object, const PosterOrbit& orbit, const PosterPoint& point) {
        if (object.linear) return object.angle;
        const double initial = std::atan2(orbit.centerY - object.centerY, orbit.centerX - object.centerX);
        const double current = std::atan2(orbit.centerY - point.y, orbit.centerX - point.x);
        return object.angle + std::atan2(std::sin(current - initial), std::cos(current - initial));
    }
    bool PosterIntersects(const PosterObject& object, const PosterPoint& point, double angle, double margin) {
        const double c = std::abs(std::cos(angle)), s = std::abs(std::sin(angle));
        const double x = (c * object.width + s * object.height) / 2.0;
        const double y = (s * object.width + c * object.height) / 2.0;
        return point.x + x + margin > 0.0 && point.x - x - margin < posterWidth &&
               point.y + y + margin > 0.0 && point.y - y - margin < posterHeight;
    }
    PosterMotion BuildPosterMotion() {
        PosterMotion motion;
        const double dx = posterObjects[29].centerX - posterObjects[23].centerX;
        const double dy = posterObjects[29].centerY - posterObjects[23].centerY;
        const double magnitude = std::hypot(dx, dy);
        motion.lineDirection = {dx / magnitude, dy / magnitude};
        const double halfSpan = std::ceil((std::hypot(posterWidth, posterHeight) + posterSpacing) / posterSpacing) * posterSpacing;
        motion.orbits.reserve(std::size(posterObjects));
        for (const auto& object : posterObjects) {
            PosterOrbit orbit;
            if (object.linear) {
                orbit.minimum = -halfSpan;
                orbit.length = halfSpan * 2.0;
            } else {
                const auto& center = posterCenters[object.group];
                orbit.centerX = center.x; orbit.centerY = center.y;
                double below = std::clamp(0.5 + (object.centerY - PosterGuideY(std::clamp(object.centerX, 0.0, posterWidth))) / posterTurnWidth, 0.0, 1.0);
                below = below * below * (3.0 - 2.0 * below);
                const double tangent = (5.0 + 40.0 * below) * posterPi / 180.0;
                const double ratio = std::clamp(std::sqrt((std::max)(1.0, center.x - object.centerX) /
                    ((std::max)(1.0, center.y - object.centerY) * std::tan(tangent))), 0.55, 4.5);
                orbit.radiusX = std::hypot(object.centerX - center.x, ratio * (object.centerY - center.y));
                orbit.radiusY = orbit.radiusX / ratio;
                orbit.initialAngle = std::atan2((object.centerY - center.y) / orbit.radiusY, (object.centerX - center.x) / orbit.radiusX);
                orbit.arc.resize(posterArcSegments + 1, 0.0);
                PosterPoint previous = {orbit.radiusX * std::cos(orbit.initialAngle), orbit.radiusY * std::sin(orbit.initialAngle)};
                for (size_t k = 1; k <= posterArcSegments; ++k) {
                    const double angle = orbit.initialAngle + k * (2.0 * posterPi / posterArcSegments);
                    const PosterPoint next = {orbit.radiusX * std::cos(angle), orbit.radiusY * std::sin(angle)};
                    orbit.arc[k] = orbit.arc[k - 1] + std::hypot(next.x - previous.x, next.y - previous.y);
                    previous = next;
                }
                orbit.length = orbit.arc.back();
            }
            motion.orbits.push_back(std::move(orbit));
        }
        for (size_t i = 0; i < std::size(posterObjects); ++i) {
            const auto& object = posterObjects[i]; const auto& orbit = motion.orbits[i];
            const size_t count = std::max<size_t>(2, static_cast<size_t>(std::ceil(orbit.length / posterSpacing)));
            const double gap = orbit.length / count;
            motion.instances.push_back({i, 0.0});
            for (size_t k = 1; k < count; ++k) {
                const double offset = k * gap;
                bool visible = false;
                if (object.linear) {
                    for (const auto& other : posterObjects) if (other.linear) {
                        const auto point = PosterPosition(other, orbit, motion.lineDirection, offset);
                        if (PosterIntersects(other, point, other.angle, 20.0)) { visible = true; break; }
                    }
                } else {
                    const auto point = PosterPosition(object, orbit, motion.lineDirection, offset);
                    visible = PosterIntersects(object, point, PosterDrawAngle(object, orbit, point), 20.0);
                }
                if (!visible) motion.instances.push_back({i, offset});
            }
        }
        std::stable_sort(motion.instances.begin(), motion.instances.end(), [](const auto& a, const auto& b) {
            const int layerA = posterObjects[a.sourceIndex].layer, layerB = posterObjects[b.sourceIndex].layer;
            return layerA != layerB ? layerA < layerB : a.sourceIndex < b.sourceIndex;
        });
        return motion;
    }
    const PosterMotion& GetPosterMotion() {
        static const PosterMotion motion = BuildPosterMotion();
        return motion;
    }
    double PosterDistance(int layer, double elapsedSeconds) {
        const double duration = posterRampEnd - posterDelay;
        const double tau = std::clamp(elapsedSeconds - posterDelay, 0.0, duration);
        return posterLayerSpeeds[layer] * (posterInitialRatio * tau + (1.0 - posterInitialRatio) * tau * tau * tau /
            (3.0 * duration * duration) + (std::max)(0.0, elapsedSeconds - posterRampEnd));
    }
}

void PlayScene::UpdatePosterBackground(double elapsedSeconds) {
    const auto& motion = GetPosterMotion();
    posterDrawObjects.clear();
    posterDrawObjects.reserve(motion.instances.size());
    for (const auto& instance : motion.instances) {
        const auto& object = posterObjects[instance.sourceIndex];
        const auto& orbit = motion.orbits[instance.sourceIndex];
        const auto point = PosterPosition(object, orbit, motion.lineDirection,
            PosterDistance(object.layer, elapsedSeconds) + instance.offset);
        posterDrawObjects.push_back({instance.sourceIndex, point.x, point.y, PosterDrawAngle(object, orbit, point)});
    }
}


PlayScene::PlayScene() {
	UpdatePosterBackground(0.0);

	//	ChartLoad::load(u8"(Songs/続・〆ドレー2000/続・〆ドレー2000.tja)", CD, CourseType::Oni);
	//	ChartLoad::load("Songs/シャイニングスター/シャイニングスター.tja", CD, CourseType::Oni);
	debug = "走査対象のパス: " + fs::absolute( "Songs" ).string(); // 絶対パスに変換して出力
	tempTjaPath = FindAllTjaFiles( "Songs" );
	ChartLoad::load( tempTjaPath[4].c_str(), CD, CourseType::Oni );
		//CD.loadSong("Resource/Debug/カンケーガール.mp3", 185.0, 4.2);
	double soundVol = 0.8;
	ChangeVolumeSoundMem( 255 * soundVol, CD.songData.songHandle.handle );
	/*
	for (int i = 0; i < 100; i++) {
		NoteType rndNote = static_cast<NoteType>(GetRand(1));
		CD.notes.push_back({ 158 , (long long)(240.0 / CD.bpm * 1000000.0) * i, rndNote});
	}
	*/
	//extern ChartScanner CD;
	CD.notes;


}


void PlayScene::Update() {

	this->Input();
	CD.Update();
	// Remove the chart offset so the background follows elapsed song playback.
	const double backgroundSeconds = CD.songData.playing
		? (std::max)(0.0, (CD.nowSongTime + CD.songData.offsetTime) / 1000000.0) : 0.0;
	UpdatePosterBackground(backgroundSeconds);
	effects.Update(CD, Time::deltaSec());


}


void PlayScene::Draw() {



	if (CD.isGogoTime) {
		DrawGraph( 0, 0, Skin::GetTexture( "play/bg_clear" ).handle, true );
	}
	else {
		// Background state is generated by Update; Draw only reads it.
		int backgroundWidth = 0, backgroundHeight = 0;
		GetDrawScreenSize(&backgroundWidth, &backgroundHeight);
		const double backgroundScale = backgroundHeight / posterHeight;
		DrawBox(0, 0, backgroundWidth, backgroundHeight, GetColor(64, 84, 163), TRUE);
		for (const auto& state : posterDrawObjects) {
			const auto& object = posterObjects[state.sourceIndex];
			const auto& texture = Skin::GetTexture(object.textureKey);
			DrawRotaGraphF(
				static_cast<float>(state.centerX / posterWidth * backgroundWidth),
				static_cast<float>(state.centerY / posterHeight * backgroundHeight),
				object.scale * backgroundScale, state.angle, texture.handle, TRUE);
		}
	}

	// レーン
	const auto& lane = Skin::GetTexture("play/ScrollField/bg");
	DrawBox(SkinLayout::ScrollField.x, SkinLayout::ScrollField.y,
		SkinLayout::ScrollField.x + lane.w, SkinLayout::ScrollField.y + lane.h,
		GetColor(32, 42, 82), TRUE); // #202A52: darkened background blue
	effects.DrawLane();

	DrawExtendGraph( SkinLayout::ScrollField.x,
					 SkinLayout::ScrollField.y,
					 SkinLayout::ScrollField.x + SkinLayout::NoteSize,
					 SkinLayout::ScrollField.y + SkinLayout::NoteSize,
					 Skin::GetTexture( Skin::GetNoteImageKey( NoteType::Judge, false ) ).handle,
					 true );

	int noteX;
	long long noteRelativeTime; // 曲の再生位置によるノーツの相対時間(us)
	/// ノーツ描画
	////////////////　ヘルパー関数
	auto isDrawable = []( const int x, const bool isJudged ) {
		int winx, winy;
		GetWindowSize( &winx, &winy );

		bool isInsideScreen = (x < winx) &&
			(x > -SkinLayout::NoteSize);
		return isInsideScreen && !isJudged;
	};

	auto isRangeDrawable = []( const int x1, const int x2, const bool isJudged ) {
		int winx, winy;
		GetWindowSize( &winx, &winy );

		bool isInsideScreen = (x1 < winx) && (x2 > 0);
		return isInsideScreen && !isJudged;
	};
	auto getNoteXFromRelativeTime = [this]( double relativeTimeSec, double bpm, double scroll ) { // 汎用
		return (relativeTimeSec / (240.0 / bpm)) * 900.0 * scroll + SkinLayout::ScrollField.x;
	};

	auto getNoteX = [this]( const Note& note ) {
		return ((CD.noteRelativeTime( note.idx ) / 1000000.0) / (240.0 / note.bpm)) * 900.0 * note.scroll + SkinLayout::ScrollField.x;
	};

	auto DrawNote = [&getNoteX]( const Note& note, const long long& relativeTimeUs, std::string& textureKey ) {
		double noteX = getNoteX( note );
		DrawExtendGraph( noteX,
						 SkinLayout::ScrollField.y,
						 noteX + SkinLayout::NoteSize,
						 SkinLayout::ScrollField.y + SkinLayout::NoteSize,
						 Skin::GetTexture( textureKey ).handle,
						 true );
	};

	auto DrawBalloon = [this, &getNoteXFromRelativeTime]( const Note& note, std::string& HeadImgKey, std::string& TailImgKey ) {
		/// 風船ノーツの描画関数。
		//  
		// 風船ノーツは連打中に判定枠にとどまるため、連打開始前と連打終了後で座標計算に使う時間を変える必要がある


		double noteX = 0;
		long long rel = CD.noteRelativeTime( note.idx );
		long long dur = note.duration;
		if ( rel > 0 ) {	// 連打開始前
			noteX = getNoteXFromRelativeTime( rel / 1000000.0, note.bpm, note.scroll ); // 相対時間をそのまま渡す
		}
		else if ( rel + note.duration < 0 ) {	// 連打終了後
			noteX = getNoteXFromRelativeTime( (rel + dur) / 1000000.0, note.bpm, note.scroll ); // 相対時間をそのまま渡す
		}
		else { // 連打中
			noteX = getNoteXFromRelativeTime( 0.0, note.bpm, note.scroll ); // 相対時間を0で渡して、判定枠にとどめる
		}

		if ( note.isJudged ) return; // 判定済みなら描画しない
		DrawExtendGraph( noteX,
						 SkinLayout::ScrollField.y,
						 noteX + SkinLayout::NoteSize,
						 SkinLayout::ScrollField.y + SkinLayout::NoteSize,
						 Skin::GetTexture( HeadImgKey ).handle,
						 true );
		noteX += SkinLayout::NoteSize;
		DrawExtendGraph( noteX,
						 SkinLayout::ScrollField.y,
						 noteX + SkinLayout::NoteSize,
						 SkinLayout::ScrollField.y + SkinLayout::NoteSize,
						 Skin::GetTexture( TailImgKey ).handle,
						 true );
	};


	//////////////////
	size_t index = 0;
	const size_t notesIndex = CD.nextNoteIndex;

	std::unordered_set<size_t> drawnRolls;
	for ( size_t i = CD.notes.size(); i-- > 0; ) {
		const auto& note = CD.notes[i];
		noteRelativeTime = CD.noteRelativeTime( CD.notes.size() - 1 - index );
		noteX = getNoteX( note );
		// 240/BPM = 1小節の秒数。1小節当たり960pxとする。よって、(相対時間)/(240/BPM) * 960 = ノーツのX座標
		NoteType drawType = note.type;
		if ( note.type == NoteType::None ) {
			index++;
			continue; // 0(空白ノーツは描画しない)
		}
		std::string targetNoteTextureKey;

		switch ( drawType ) {
			case NoteType::Don: case NoteType::Katsu:
				if ( !isDrawable( noteX, note.isJudged ) )  break;
				targetNoteTextureKey = Skin::GetNoteImageKey( drawType, note.isBig );
				DrawNote( note, noteRelativeTime, targetNoteTextureKey );
				break;
			//

			case NoteType::RollHead:
			case NoteType::RollTail:
			{											// 連打ノーツの描画。 連打頭と連打尾の両方を通すが、二重描画を防ぐため描画済みかどうかを検知する。

				if ( drawnRolls.contains( note.rollId ) ) break;	// 既に描画済みの連打はスルー

				bool isRollHead = (note.type == NoteType::RollHead);

				const Note& rollHead = isRollHead ? note : CD.notes[note.pairRollIndex];
				const Note& rollTail = !isRollHead ? note : CD.notes[note.pairRollIndex];


				size_t rollHeadIdx = rollTail.pairRollIndex;
				size_t rollTailIdx = rollHead.pairRollIndex;

				double rollHeadX = getNoteX( rollHead );
				double rollTailX = getNoteX( rollTail );

				bool drawRoll = (isRangeDrawable( rollHeadX, rollTailX + SkinLayout::NoteSize, false ));
				bool isBig = rollHead.isBig;

				if ( !drawRoll ) break;


				std::string headImgKey = Skin::GetRollImageKey( Skin::RollPart::Head, isBig );
				std::string tailImgKey = Skin::GetRollImageKey( Skin::RollPart::Tail, isBig );
				std::string bodyImgKey = Skin::GetRollImageKey( Skin::RollPart::Body, isBig );


				DrawExtendGraph( rollTailX,
				 SkinLayout::ScrollField.y,
				 rollTailX + SkinLayout::NoteSize,
				 SkinLayout::ScrollField.y + SkinLayout::NoteSize,
				 Skin::GetTexture( tailImgKey ).handle,
				 true );

				DrawExtendGraph( rollHeadX + SkinLayout::NoteSize / 2,
								 SkinLayout::ScrollField.y,
								 rollTailX + SkinLayout::NoteSize / 2,
								 SkinLayout::ScrollField.y + SkinLayout::NoteSize,
								 Skin::GetTexture( bodyImgKey ).handle,
								 true );


				DrawExtendGraph( rollHeadX,
								 SkinLayout::ScrollField.y,
								 rollHeadX + SkinLayout::NoteSize,
								 SkinLayout::ScrollField.y + SkinLayout::NoteSize,
								 Skin::GetTexture( headImgKey ).handle,
								 true );



				break;

			}

			case NoteType::BalloonHead:
			{
				std::string balloonHeadImgKey = Skin::GetBalloonImageKey( Skin::RollPart::Head );
				std::string balloonTailImgKey = Skin::GetBalloonImageKey( Skin::RollPart::Tail );

				DrawBalloon( note, balloonHeadImgKey, balloonTailImgKey );
				break;
			}


		}

		index++;
	}
	CD.notes; // デバッグで内部数値を確認する用
	effects.DrawOverlay();


	int winx, winy;
	GetWindowSize( &winx, &winy );




	//////
	if ( lastRollIdx != SIZE_MAX ) {
		DrawWhiteNumberRight(winx, 0, GetFontSize(), std::to_string(CD.notes[lastRollIdx].rollHitCount));
	}

	if ( CD.nextNoteIndex < CD.notes.size() ) {
		DrawFormatString2Right( winx,
								16,
								GetColor( 255, 255, 255 ),
								std::to_string( static_cast<int>( CD.notes[CD.nextNoteIndex].type ) )
		);

	
	}

	std::string s;
	for (const auto& note : CD.notes) {
		if ( note.type != NoteType::BalloonHead ) continue;
		s = "Balloon ID: " + std::to_string(note.balloonId) + ", Required: " + std::to_string(note.requiredHits) + ", Hit: " + std::to_string(note.balloonHitCount);
		break;
	}
	DrawFormatString2Right( winx,
								32,
								GetColor( 0, 0, 0 ),
								s );

	s = std::string("isGOGO: ") + ((CD.isGogoTime) ? "true" : "false");
	DrawFormatString2Right( winx,
							48,
							GetColor( 0, 0, 0 ),
							s );

	//DrawExtendGraph( 0,
	//			 0,
	//			 winx,
	//			 winy,
	//			 Skin::GetTexture( Skin::GetRollImageKey( Skin::RollPart::Head, false ) ).handle,
	//			 true );


	std::string str = "";
	int strW = 0;
	int debugstrY = 0;
	std::string fps = std::to_string( FPS::getFps() );
	strW = GetDrawFormatStringWidth( fps.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), fps.c_str() );
	debugstrY += 16;


	std::string songhandle = std::to_string( CD.songData.songHandle.handle );
	strW = GetDrawFormatStringWidth( songhandle.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), songhandle.c_str() );
	debugstrY += 16;
	////
	str = debug;
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;

	str = "[0]: " + tempTjaPath[0];
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;

	str = "[1]: " + tempTjaPath[1];
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;

	str = "[2]: " + tempTjaPath[2];
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;

	str = "[3]: " + tempTjaPath[3];
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;


	str = "Songs/シャイニングスター/シャイニングスター.tja"; // 自分で、"/"だけを使って、ハードコードする
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), "%s", str.c_str() );
	debugstrY += 16;
	// tempTjaPathなど、これまでの複雑な経路を、一旦すべて無視する
	DrawWhiteFontAsset(0, 100, GetFontSize(), Skin::GetTexture("play/Font/asset_06"));
	////
	str = "path: " + CD.tjaPath;
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;
	////
	str = "TITLE: " + CD.Title;
	if (const char* titleKey = WhiteSongTitleKey(CD.Title, CD.tjaPath)) {
		const auto& titleImage = Skin::GetTexture(titleKey);
		const int prefixWidth = GetDrawFormatStringWidth("TITLE: ");
		strW = prefixWidth + int(std::ceil(WhiteFontWidth(titleImage, GetFontSize())));
		DrawBox(0, debugstrY, strW, debugstrY + 16, GetColor(0, 0, 0), true);
		DrawFormatString(0, debugstrY, GetColor(255, 255, 255), "TITLE: ");
		DrawWhiteFontAsset(float(prefixWidth), float(debugstrY), GetFontSize(), titleImage);
	} else {
		strW = GetDrawFormatStringWidth(str.c_str());
		DrawBox(0, debugstrY, strW, debugstrY + 16, GetColor(0, 0, 0), true);
		DrawFormatString(0, debugstrY, GetColor(255, 255, 255), "%s", str.c_str());
	}
	debugstrY += 16;

	str = "SUBTITLE: " + CD.subTitle;
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;

	str = "BPM: " + std::to_string( CD.bpm );
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;

	str = "LEVEL: " + std::to_string( CD.level );
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;

	extern std::vector<ChartScanner::ChartFile> ChartList;
	for ( const auto& chartfile : ChartList ) {
		std::string path = chartfile.ChartPath.string();
		strW = GetDrawFormatStringWidth( path.c_str() );
		DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
		DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), path.c_str() );
		debugstrY += 16;
	}

}

void PlayScene::Input() {

	CD.Input();
}
