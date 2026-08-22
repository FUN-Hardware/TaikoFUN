#include "Debug.h"
#include "DxLib.h"
#include "Input/Input.h"
#include "Fps.h"
#include "Core/ChartData.h"
#include <string>
#include "Core/text.h"
#include "Skin/SkinData.h"

#define NowTime GetNowHiPerformanceCount()


Debug::Debug() {
	songTime = 0;
	songHandle = 0;
	currentTime = 0;
	songPlaying = false;
	chartData.loadSong("Resource/Debug/カンケーガール.mp3", 185.0, 4.2);
	double soundVol = 0.8;
	ChangeVolumeSoundMem(255 * soundVol, chartData.songData.songHandle.handle);
	for (int i = 0; i < 100; i++) {
		chartData.notes.push_back({ 80, (long long)(240.0 / chartData.bpm * 1000000.0) * i, NoteType::Don });
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
	chartData.Update();
	l.Update();


}

void Debug::Draw() {


	DrawFormatString(0, 0, GetColor(255, 255, 255), "FPS: %d", FPS::getFps());

	int strY = 16;

	switch(debugPage){
	case 0:
		DrawFormatString(10, strY, GetColor(255, 255, 255), "Current Time: %lld, %ld", NowTime, NowTime/1000000);
		strY += 20;
		DrawFormatString(10, strY, GetColor(255, 255, 255), "keyState_SPACE: %s", Input::isKeyDown(KEY_INPUT_SPACE) ? "TRUE" : "FALSE");
		strY += 20;
		DrawFormatString(10, strY, GetColor(255, 255, 255), "songHandle: %d", chartData.songData.songHandle.handle);
		strY += 20;
		DrawFormatString(10, strY, GetColor(255, 255, 255), "song Start at: %lld", chartData.songData.songStartTime);
		strY += 20;
		DrawFormatString(10, strY, GetColor(255, 255, 255), "song offset at: %lld", chartData.songData.offsetTime);
		strY += 20;
		DrawFormatString(10, strY, GetColor(255, 255, 255), "songTime us: %lld", chartData.songData.getSongCurrentTimeUs());
		strY += 20;
		DrawFormatString(10, strY, GetColor(255, 255, 255), "songTime from offset us: %lld", chartData.songData.getSongCurrentTimeUs(true));
		strY += 20;
		DrawFormatString(10, strY, GetColor(255, 255, 255), "GetSoundCurrentTime us: %lf", GetSoundCurrentTime(chartData.songData.songHandle.handle)/ 1000.0);
		strY += 20;
		DrawFormatString(10, strY, GetColor(255, 255, 255), "songTime sec: %lf", chartData.songData._songProgTime / 1000000.0);
		strY += 20;
		DrawFormatString(10, strY, GetColor(255, 255, 255), "Last paused time stamp : %lld", chartData.songData.lastPausedTimeStamp);
		strY += 20;
		DrawFormatString(10, strY, GetColor(255, 255, 255), "Total paused duration: %lf", chartData.songData.totalPausedDuration / 1000000.0);
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
		break;
	case 1:
		DrawFormatString(10, strY, GetColor(255, 255, 255), "Score: %d", chartData.score);
		strY += 20;
		DrawFormatString(10, strY, GetColor(255, 255, 255), "next Note: %d", chartData.nextNoteIndex);
		strY += 20;
		DrawFormatString(10, strY, GetColor(255, 255, 255), "GOOD: %d\nOK: %d\nBAD: %d\nMISS: %d", chartData.good, chartData.ok, chartData.bad, chartData.miss);
		strY += 20;

	}
	int w, h;
	GetWindowSize(&w, &h);

	std::string str;
	for (const auto& l : chartData.judgelogs) {
		switch (l.type) {
		case JudgeType::GOOD:
			str = "GOOD";
			break;
		case JudgeType::OK:
			str = "OK";
			break;
		case JudgeType::BAD:
			str = "BAD";
			break;
		case JudgeType::MISS:
			str = "MISS";
		}
		str += "\n";
	}

	DrawFormatString2Right(w, 0, GetColor(255, 255, 255), str.c_str());


	DrawFormatString(10, h - 300, GetColor(255, 255, 255), "WindowSize h: %d", h);
	strY += 20;

	l.Draw();

	strY = h - 200;

	/*
	{//判定領域の表示 
//		((noteRelativeTime / 1000000.0) / (240.0 / note.bpm)) * 960.0;
		{	// 不可判定
			int boxX = 10, boxY = strY, boxW;
			int boxH = 16;
			boxW = ((chartData.judgeBAD / 1000000.0) / (240 / chartData.bpm)) * 960.0;
			boxX -= boxW / 2;
			boxX += GetDrawFormatStringWidth("〇") / 2;
			DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, GetColor(0, 0, 255), true);

		}

		{	// 可判定
			int boxX = 10, boxY = strY, boxW;
			int boxH = 16;
			boxW = ((chartData.judgeOK / 1000000.0) / (240 / chartData.bpm)) * 960.0;
			boxX -= boxW / 2;
			boxX += GetDrawFormatStringWidth("〇") / 2;
			DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, GetColor(255, 255, 255), true);
		}


		{
			// 良判定
			int boxX = 10, boxY = strY, boxW;
			int boxH = 16;
			boxW = ((chartData.judgeGOOD / 1000000.0) / (240 / chartData.bpm)) * 960.0;
			boxX -= boxW / 2;
			boxX += GetDrawFormatStringWidth("〇") / 2;
			DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, GetColor(255, 255, 0), true);
		}
	}
	*/

//	DrawFormatString(10, strY, GetColor(255, 255, 255), "〇");//判定枠

	DrawGraph(10, strY, Skin::GetTexture(GetNoteImageKey(NoteType::Judge)).handle, true);
	int noteX;
	long long noteRelativeTime; // 曲の再生位置によるノーツの相対時間(us)
	/// ノーツ仮描画
	size_t index = 0;
	const size_t notesIndex = chartData.nextNoteIndex;
	for (const auto& note : chartData.notes) {

		noteRelativeTime = chartData.noteRelativeTime(index);
		noteX = ((noteRelativeTime/1000000.0) / (240.0/note.bpm)) * 960.0;
		// 240/BPM = 1小節の秒数。1小節当たり960pxとする。よって、(相対時間)/(240/BPM) * 960 = ノーツのX座標
		if(noteX < 1300 && noteX > 0)DrawGraph(noteX, strY, Skin::GetTexture(GetNoteImageKey(NoteType::Don)).handle, true); // 画面内のみ描画
		index++;
	}

}




void Debug::Input() {

	if (Input::isKeyTriggered(KEY_INPUT_RIGHT)) debugPage++;
	if (Input::isKeyTriggered(KEY_INPUT_LEFT)) (debugPage > 0) ? debugPage-- : debugPage = 0;


	if (Input::isKeyTriggered(KEY_INPUT_SPACE)) {


		if (songPlaying) {

			chartData.songData.stopSong();

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
		time = chartData.songData.getSongCurrentTimeUs(true) > 0 ? std::to_string(chartData.songData.getSongCurrentTimeUs(true)) : "0";
		
		//l.add(type + ", " + time + ", " + success, 10.0);
	}

	chartData.Input();



	/// 

}