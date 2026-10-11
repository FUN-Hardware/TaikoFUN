#include "SongSelect.h"

#include "DxLib.h"
#include "File/FIndAllTJA.h"
#include "File/FileUtil.h"
#include "Core/General.h"
#include "Chart/CourseType.h"
#include "Core/Time.h"
#include "Input/Input.h"
#include "Skin/SkinData.h"
#include "Audio/Sounds.h"
#include "File/ChartLoader.h"
#include "Core/text.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <exception>
#include <stdexcept>

namespace fs = std::filesystem;

namespace {
	CourseType _parseCourseType( const std::string& str ) {	// 譜面のCOURSE: の項目を受け取り、対応するCourseTypeを返す
		static const std::unordered_map<std::string, CourseType> courseMap = {
			{ "easy",		CourseType::Easy },
			{ "0",			CourseType::Easy },
			{ "normal",		CourseType::Normal },
			{ "1",			CourseType::Normal },
			{ "hard",		CourseType::Hard },
			{ "2",			CourseType::Hard },
			{ "oni",			CourseType::Oni },
			{ "3",			CourseType::Oni },
			{ "expert",		CourseType::Oni },
			{ "edit",		CourseType::InnerOni },
			{ "inner",		CourseType::InnerOni },
			{ "inneroni",	CourseType::InnerOni },
			{ "4",			CourseType::InnerOni },
			{ "insane",		CourseType::InnerOni },

		};
		std::string lowerValue = str;
		std::transform( lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower ); // 小文字化
		auto result = courseMap.find( lowerValue );
		if ( result != courseMap.end() ) {
			return result->second;
		}

		return CourseType::Oni;
	}

	std::string getSongPath(const std::string& tjaPath, const std::string& songPath){
		fs::path tja( tjaPath );
		
		fs::path folder = tja.parent_path();

		return (folder / songPath).string();
	}
	
}





SongSelect::SongSelect( GameContext* ctx ) : ctx_(ctx){

}




void SongSelect::Init() {
	view_.Init();
	stopPreview();
	songList.clear();
	nowSelectedSongIdx = 0;
	nowSelectedCourseIdx = 0;
	choosingCourse = false;
	idleTimer_.Reset();
	returnGate_.Reset(ctx_->blockSongSelectInputUntilRelease);
	returnFadeLeft_ = ctx_->blockSongSelectInputUntilRelease ? DemoLogic::FadeSeconds : 0.0;
	ctx_->blockSongSelectInputUntilRelease = false;
	ctx_->playMode = PlayMode::Normal;

	auto pathList = FindAllTjaFiles("Songs");


	auto getValueAfterColon = []( const std::string& str ) {
		return string_util::trimWhitespace( str.substr( str.find( ':' ) + 1 ) );
	};


	for ( const auto& path : pathList ) {
		try {
		size_t lastFindCourseNum = 3;
		SongItemData sid;
		const auto rawLines = file_util::getAllLines(path);
		sid.tjaPath = path;

		for ( const auto& rawLine : rawLines ) {
			const auto& line = string_util::trimWhitespace(rawLine);

			if ( line.starts_with( "TITLE" ) ) {

				sid.title = getValueAfterColon( line );

			}
			else if ( line.starts_with( "WAVE" ) ) {

				sid.songPath = getSongPath(path, getValueAfterColon( line ) );

			}
			else if ( line.starts_with( "LEVEL" ) ) {

				sid.levels[lastFindCourseNum] = std::stof( getValueAfterColon( line ) );

			}
			else if ( line.starts_with( "COURSE" ) ) {

				CourseType c = _parseCourseType( getValueAfterColon( line ) );
				lastFindCourseNum = static_cast<int>(c);
				sid.Courses[lastFindCourseNum] = true;

			}
			else if ( line.starts_with( "DEMOSTART" ) ) {

				double demoStartSec = std::stod( getValueAfterColon( line ) );
				sid.DemoStartAt = demoStartSec * Time::TIME_US;

			}
		}



		if ( !sid.Courses[0] && !sid.Courses[1] && !sid.Courses[2] && !sid.Courses[3] && !sid.Courses[4] ) {
			sid.Courses[3] = true;
		}
		songList.push_back( sid );
		} catch (const std::exception&) {
			OutputDebugString(("Skipping invalid song metadata: " + path + "\n").c_str());
		}
	}
	const auto cursor = std::find_if(songList.begin(), songList.end(), [&](const SongItemData& song) {
		return song.tjaPath == ctx_->songSelectCursor.path;
	});
	if (cursor != songList.end()) {
		nowSelectedSongIdx = size_t(std::distance(songList.begin(), cursor));
		nowSelectedCourseIdx = size_t(std::clamp(int(ctx_->songSelectCursor.course), 0, 4));
	}

	playDemo();


}

void SongSelect::Update() {
	view_.Update(Time::deltaSec(), nowSelectedSongIdx, choosingCourse);
	const bool focused = GetWindowActiveFlag() == TRUE;
	if (!focused) { idleTimer_.Reset(); return; }
	returnFadeLeft_ = (std::max)(0.0, returnFadeLeft_ - (std::max)(0.0, Time::deltaSec()));
	if (intervalLeft > 0){ 
		intervalLeft -= Time::deltaUs();
		if ( intervalLeft <= 0 ) 
		{
			playDemo();
		}
	}

	const bool keyDown = Input::isAnyKeyDown(), keyPressed = Input::isAnyKeyTriggered();
	if (!returnGate_.Update(keyDown)) { idleTimer_.Reset(); return; }
	if (keyPressed) {
		demoUnavailable_ = false;
		errorMessage_.clear();
	}
	Input(); // Manual selection wins when input and idle expiry share a frame.
	rememberCursor();
	if (Request()) return;
	if (idleTimer_.Update(Time::deltaSec(), focused, keyDown, keyPressed,
		!choosingCourse && !songList.empty() && !demoUnavailable_)) {
		const auto course = DemoLogic::ChooseCourse(std::to_array(songList[nowSelectedSongIdx].Courses),
			int(nowSelectedCourseIdx));
		if (!course || !startPlay(PlayMode::Demo, size_t(*course))) demoUnavailable_ = true;
		idleTimer_.Reset();
	}

}


void SongSelect::Input() {
	if (songList.empty()) return;



	if ( Input::isKeyTriggered( KEY_INPUT_K ) ) {

		if ( choosingCourse ) {

			if (nowSelectedCourseIdx < 4) nowSelectedCourseIdx++;

		}
		else{
			nowSelectedSongIdx = (nowSelectedSongIdx < songList.size() - 1) ? ++nowSelectedSongIdx : 0;

			PlaySoundMem( Sounds::GetSoundHandle( SoundKey::Katsu ), DX_PLAYTYPE_BACK, true );
			intervalLeft = demoInterval;

		}


	}
	else if ( Input::isKeyTriggered( KEY_INPUT_D ) ) {


		if ( choosingCourse ) {

			if ( nowSelectedCourseIdx > 0 ) nowSelectedCourseIdx--;
			else choosingCourse = false;
		}
		else {

			nowSelectedSongIdx = (nowSelectedSongIdx > 0) ? --nowSelectedSongIdx : songList.size() - 1;

			PlaySoundMem( Sounds::GetSoundHandle( SoundKey::Katsu ), DX_PLAYTYPE_BACK, true );
			intervalLeft = demoInterval;
		}

	}

	else if ( Input::isKeyTriggered( KEY_INPUT_F ) || Input::isKeyTriggered( KEY_INPUT_J ) ) {

		if ( choosingCourse ) {
			
			if ( songList[nowSelectedSongIdx].Courses[nowSelectedCourseIdx] ) {
				
				PlaySoundMem( Sounds::GetSoundHandle( SoundKey::Don ), DX_PLAYTYPE_BACK, true );
				startPlay(PlayMode::Normal, nowSelectedCourseIdx);

			}

		}
		else {

			choosingCourse = true;
			PlaySoundMem( Sounds::GetSoundHandle( SoundKey::Don ), DX_PLAYTYPE_BACK, true );


		}



	}
}


void SongSelect::Draw() {
    view_.Draw(songList, nowSelectedSongIdx, nowSelectedCourseIdx, choosingCourse,
        demo && demo->songHandle.handle >= 0 && intervalLeft <= 0,
        errorMessage_, returnFadeLeft_);
}

void SongSelect::Finalize() {
	// Loading belongs to startPlay; shutdown and menu return must not load a chart.
	rememberCursor();
	stopPreview();
	view_.Finalize();
}


void SongSelect::playDemo() {
	stopPreview();
	if (songList.empty()) return;
	demo = std::make_unique<SongData>();
	demo->loadSong( songList[nowSelectedSongIdx].songPath.c_str() );
	if (demo->songHandle.handle >= 0)
		demo->playSongFrom( songList[nowSelectedSongIdx].DemoStartAt );
	
}

void SongSelect::stopPreview() {
	if (demo) demo->stopSong();
	demo.reset(); // Preserve the demo-overlap fix: destroy the old handle first.
}

void SongSelect::rememberCursor() {
	if (songList.empty()) return;
	ctx_->songSelectCursor.path = songList[nowSelectedSongIdx].tjaPath;
	ctx_->songSelectCursor.course = static_cast<CourseType>(nowSelectedCourseIdx);
}

bool SongSelect::startPlay(PlayMode mode, size_t courseIndex) {
	if (songList.empty() || courseIndex >= 5 || !songList[nowSelectedSongIdx].Courses[courseIndex]) return false;
	try {
		const auto& song = songList[nowSelectedSongIdx];
		if (!fs::is_regular_file(song.songPath)) throw std::runtime_error("Missing audio");
		auto chart = ChartLoad::load(song.tjaPath.c_str(), static_cast<CourseType>(courseIndex));
		if (!chart || chart->notes.empty() || chart->songData.songHandle.handle < 0)
			throw std::runtime_error("Chart or audio could not be loaded");
		rememberCursor();
		stopPreview();
		ctx_->selectedCourse.path = song.tjaPath;
		ctx_->selectedCourse.course = static_cast<CourseType>(courseIndex);
		ctx_->chartData = std::move(chart);
		ctx_->resultData.reset();
		ctx_->playMode = mode;
		RequestScene(SceneID::Play);
		return true;
	} catch (const std::exception&) {
		errorMessage_ = mode == PlayMode::Demo ? "この曲のデモを開始できません" : "この曲を開始できません";
		return false;
	}
}
