#include "SongSelect.h"

#include "DxLib.h"
#include "File/FindAllTJA.h"
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

	auto pathList = FindAllTjaFiles("Songs");


	auto getValueAfterColon = []( const std::string& str ) {
		return string_util::trimWhitespace( str.substr( str.find( ':' ) + 1 ) );
	};

	SongItemData sid;

	std::string Title = "";
	float lvs[5];
	bool courses[5];
	size_t lastFindCourseNum = 3;
	long long DEMO = 0;


	for ( const auto& path : pathList ) {

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
	}

	playDemo();


}

void SongSelect::Update() {
	if (intervalLeft > 0){ 
		intervalLeft -= Time::deltaUs();
		if ( intervalLeft <= 0 ) playDemo();
	}

	Input();

}


void SongSelect::Input() {



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
				RequestScene( SceneID::Play );

			}

		}
		else {

			choosingCourse = true;
			PlaySoundMem( Sounds::GetSoundHandle( SoundKey::Don ), DX_PLAYTYPE_BACK, true );


		}



	}
}


void SongSelect::Draw() {

	int w, h;
	GetWindowSize(&w, &h);
	DrawBox( 0, 0, w, h, GetColor( 64, 84, 163 ), TRUE );

	
	int arrowY = nowSelectedSongIdx * 16;
	DrawFormatString( 0,
				  arrowY,
				  GetColor( 255, 255, 255 ),
				  "→"
	);


	int y = 0;
	int i = 0;
	for ( const auto& song : songList ) {

		DrawFormatString( 16,
						  y,
						  GetColor( 0, 0, 0 ),
						  "%s", song.title.c_str()
							);
			y += 16;
	
			if ( choosingCourse && nowSelectedSongIdx == i ) {

				std::string dif;

				if ( nowSelectedCourseIdx == 0 ) dif += "　→"; else dif += "　　";
				dif += (songList[nowSelectedSongIdx].Courses[0]) ? "簡単" : "　　　";
				if ( nowSelectedCourseIdx == 1 ) dif += "　→"; else dif += "　　";
				dif += (songList[nowSelectedSongIdx].Courses[1]) ? "普通" : "　　　";
				if ( nowSelectedCourseIdx == 2 ) dif += "　→"; else dif += "　　";
				dif += (songList[nowSelectedSongIdx].Courses[2]) ? "難しい" : "　　　";
				if ( nowSelectedCourseIdx == 3 ) dif += "　→"; else dif += "　　";
				dif += (songList[nowSelectedSongIdx].Courses[3]) ? "おに" : "　　　";
				if ( nowSelectedCourseIdx == 4 ) dif += "　→"; else dif += "　　";
				dif += (songList[nowSelectedSongIdx].Courses[4]) ? "おに裏" : "　　　";


				DrawFormatString2Right( w, 0, GetColor( 0, 0, 0 ), dif );

			}

				i++;
	}

	




}

void SongSelect::Finalize() {
	
	sc_.path = songList[nowSelectedSongIdx].tjaPath;
//	sc_.course = songList[nowSelectedSongIdx].Courses;

	ctx_->selectedCourse.course = static_cast<CourseType>(nowSelectedCourseIdx);
	//ctx_->selectedCourse.course = songList[nowSelectedSongIdx]
	ctx_->chartData = ChartLoad::load( sc_.path.c_str(), ctx_->selectedCourse.course );


}


void SongSelect::playDemo() {

	demo = std::make_unique<SongData>();
	demo->loadSong( songList[nowSelectedSongIdx].songPath.c_str() );
	demo->playSongFrom( songList[nowSelectedSongIdx].DemoStartAt );
	
}
