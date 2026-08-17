#include "Debug.h"
#include "DxLib.h"
#include "Input/Input.h"
#include "Fps.h"
#include "Core/ChartData.h"

#define NowTime GetNowHiPerformanceCount()


Debug::Debug() {
	songTime = 0;
	songHandle = 0;
	currentTime = 0;
	songPlaying = false;
	chartData.loadSong("Resource/Debug/カンケーガール.mp3", 185.0);
}


Debug::~Debug() {
	DeleteSoundMem(songHandle);

}

void Debug::Update() {

	this->Input();
	calledUpdate = true;


}

void Debug::Draw() {

	DrawFormatString(0, 0, GetColor(255, 255, 255), "FPS: %d", FPS::getFps());


	int strY = 16;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "Current Time: %lld", NowTime);
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "keyState_SPACE: %s", Input::isKeyDown(KEY_INPUT_SPACE) ? "TRUE" : "FALSE");
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "songHandle: %d", chartData.songData.songHandle);
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "BPM: %.2f", chartData.bpm);
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), ": %d", songHandle);
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "songHandle: %d", songHandle);
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "is song Playing: %s", songPlaying ? "TRUE" : "FALSE");
	strY += 20;


	int w, h;
	GetWindowSize(&w, &h);

	if(CheckHitKey(KEY_INPUT_SPACE)){
		DrawFormatString(10, h - 16, GetColor(255, 255, 255), "is song Playing: %s", songPlaying ? "TRUE" : "FALSE");
	}


	DrawFormatString(10, h-450, GetColor(255, 255, 255), "WindowSize h: %d", h);
	strY += 20;

	
}




void Debug::loadSong() {

	


	soundHandles[0] = LoadSoundMem("Resource/Sound/General/don.wav");
	soundHandles[1] = LoadSoundMem("Resource/Sound/General/ka.wav");
}

void Debug::Input() {



	if (Input::isKeyTriggered(KEY_INPUT_SPACE)) {


		if (songPlaying) {

			StopSoundMem(songHandle);

		}
		else {

			PlaySoundMem(songHandle, DX_PLAYTYPE_BACK, false);
		
		}

		songPlaying = !songPlaying;
	}
	if (Input::isKeyTriggered(KEY_INPUT_KANJI)) {

			StopSoundMem(songHandle);
			PlaySoundMem(songHandle, DX_PLAYTYPE_BACK, TRUE);
			songPlaying = true;

	}


	if (Input::isKeyTriggered(KEY_INPUT_J) || Input::isKeyTriggered(KEY_INPUT_F)) {
		
		PlaySoundMem(soundHandles[0], DX_PLAYTYPE_BACK, TRUE);
	}
	if (Input::isKeyTriggered(KEY_INPUT_K) || Input::isKeyTriggered(KEY_INPUT_D)) {
		
		PlaySoundMem(soundHandles[1], DX_PLAYTYPE_BACK, TRUE);

	}


}