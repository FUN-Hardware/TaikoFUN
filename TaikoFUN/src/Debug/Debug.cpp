#include "Debug.h"
#include "DxLib.h"
#include "Input/Input.h"
#include "Fps.h"
#include "Core/ChartData.h"
#include <string>



#define NowTime GetNowHiPerformanceCount()


Debug::Debug() {
	songTime = 0;
	songHandle = 0;
	currentTime = 0;
	songPlaying = false;
	chartData.loadSong("Resource/Debug/カンケーガール.mp3", 183.0, 4.4);
	double soundVol = 0.8;
	ChangeVolumeSoundMem(255 * soundVol, chartData.songData.songHandle.handle);
	for (int i = 0; i < 100; i++) {
		chartData.notes.push_back({ 80, (long long)(240.0 / chartData.bpm * 1000000.0) * i, 1 });
	}
	soundHandles[0] = LoadSoundMem("Resource/Sound/General/don.wav");
	soundHandles[1] = LoadSoundMem("Resource/Sound/General/ka.wav");


}


Debug::~Debug() {
	DeleteSoundMem(songHandle);

}

void Debug::Update() {

	this->Input();
	calledUpdate = true;
	l.Update();


}

void Debug::Draw() {


	DrawFormatString(0, 0, GetColor(255, 255, 255), "FPS: %d", FPS::getFps());


	int strY = 16;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "Current Time: %lld", NowTime);
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "keyState_SPACE: %s", Input::isKeyDown(KEY_INPUT_SPACE) ? "TRUE" : "FALSE");
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "songHandle: %d", chartData.songData.songHandle.handle);
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "song Start at: %lld", chartData.songData.songStartTime);
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "song offset at: %lld", chartData.songData.offsetTime);
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "songTime us: %lld", chartData.songData._songProgTime);
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "songTime from offset us: %lld", chartData.songData._songProgTimefromOffset);
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "songTime sec: %lf", chartData.songData._songProgTime / 1000000.0);
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "BPM: %.2f", chartData.bpm);
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), ": %d", songHandle);
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "songHandle: %d", songHandle);
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "CheckSoundMem: %s", CheckSoundMem(chartData.songData.songHandle.handle) ? "TRUE" : "FALSE");
	strY += 20;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "playingSongFlag: %s", songPlaying ? "TRUE" : "FALSE");
	strY += 20;


	int w, h;
	GetWindowSize(&w, &h);



	DrawFormatString(10, h - 450, GetColor(255, 255, 255), "WindowSize h: %d", h);
	strY += 20;

	l.Draw();

	strY = h - 200;
	DrawFormatString(10, strY, GetColor(255, 255, 255), "a");

	int noteX;
	long long noteRelativeTime; // 曲の再生位置によるノーツの相対時間(us)
	/// ノーツ仮描画
	for (const auto& note : chartData.notes) {

		noteRelativeTime = note.absTime - chartData.songData.songProgTimefromOffset();
		noteX = ((noteRelativeTime/1000000.0) / (240.0/note.bpm)) * 960.0;
		// 240/BPM = 1小節の秒数。1小節当たり960pxとする。よって、(相対時間)/(240/BPM) * 960 = ノーツのX座標
		DrawFormatString(noteX, strY, GetColor(255, 255, 255), "s"); // 画面内のみ描画
	}

}




void Debug::Input() {



	if (Input::isKeyTriggered(KEY_INPUT_SPACE)) {


		if (songPlaying) {

			StopSoundMem(chartData.songData.songHandle.handle);

		}
		else {

			chartData.songData.playSong();
		
		}

		songPlaying = !songPlaying;
	}
	if (Input::isKeyTriggered(KEY_INPUT_KANJI)) {

		chartData.songData.playSong(true);

		songPlaying = true;

	}


	std::string type, time, success;
	//// ドン・カツ判定
	bool hit = false;
	if (Input::isKeyTriggered(KEY_INPUT_J) || Input::isKeyTriggered(KEY_INPUT_F)) {
		PlaySoundMem(soundHandles[0], DX_PLAYTYPE_BACK, TRUE);
		type = "Don";
		hit = true;

	}
	if (Input::isKeyTriggered(KEY_INPUT_K) || Input::isKeyTriggered(KEY_INPUT_D)) {
		
		PlaySoundMem(soundHandles[1], DX_PLAYTYPE_BACK, TRUE);
		type = "Katsu";
		hit = true;
	}
	if (hit) {
		time = chartData.songData.songProgTimefromOffset() > 0 ? std::to_string(chartData.songData.songProgTimefromOffset()) : "0";
		
	}

}