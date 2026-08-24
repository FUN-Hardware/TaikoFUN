#include "DxLib.h"
#include "PlayScene.h"
#include "Debug/FPS.h"

#include "Input/Input.h"
#include "Skin/SkinData.h"
#include "Core/General.h"


namespace SkinLayout {

	Vector2d ScrollField{ 325, 254 - 65 };
	int NoteSize = 128;

};



PlayScene::PlayScene() {
	CD.loadSong("Resource/Debug/シャイニングスター/シャイニングスター.ogg", 158, 3.297);
	//CD.loadSong("Resource/Debug/カンケーガール.mp3", 185.0, 4.2);
	double soundVol = 0.8;
	ChangeVolumeSoundMem(255 * soundVol, CD.songData.songHandle.handle);
	for (int i = 0; i < 100; i++) {
		NoteType rndNote = static_cast<NoteType>(GetRand(1));
		CD.notes.push_back({ 158 , (long long)(240.0 / CD.bpm * 1000000.0) * i, rndNote});
	}

}


void PlayScene::Update() {
	
	this->Input();
	CD.Update();


}


void PlayScene::Draw() {

	


	DrawGraph(0, 0, Skin::GetTexture("play/bg").handle, true);

	// レーン
	DrawGraph(SkinLayout::ScrollField.x, SkinLayout::ScrollField.y, Skin::GetTexture("play/ScrollField/bg").handle, true);

	DrawExtendGraph(SkinLayout::ScrollField.x, SkinLayout::ScrollField.y, SkinLayout::ScrollField.x + SkinLayout::NoteSize, SkinLayout::ScrollField.y + SkinLayout::NoteSize, Skin::GetTexture(GetNoteImageKey(NoteType::Judge)).handle, true);
	//DrawGraph(SkinLayout::ScrollField.x, SkinLayout::ScrollField.y, Skin::GetTexture(GetNoteImageKey(NoteType::Judge)).handle, true);

	int noteX;
	long long noteRelativeTime; // 曲の再生位置によるノーツの相対時間(us)
	/// ノーツ仮描画
	size_t index = 0;
	const size_t notesIndex = CD.nextNoteIndex;
	for (const auto& note : CD.notes) {

		noteRelativeTime = CD.noteRelativeTime(index);
		noteX = ((noteRelativeTime / 1000000.0) / (240.0 / note.bpm)) * 960.0 + SkinLayout::ScrollField.x;
		// 240/BPM = 1小節の秒数。1小節当たり960pxとする。よって、(相対時間)/(240/BPM) * 960 = ノーツのX座標
		NoteType drawType = note.type;
		if (noteX < 1300 && noteX+SkinLayout::NoteSize > 0 && !note.isJudged)DrawExtendGraph(noteX, SkinLayout::ScrollField.y, noteX+SkinLayout::NoteSize, SkinLayout::ScrollField.y+SkinLayout::NoteSize, Skin::GetTexture(GetNoteImageKey(drawType)).handle, true); // 画面内のみ描画
		index++;
	}



	int strW = 0;
	int debugstrY = 0;
	std::string fps = std::to_string(FPS::getFps());
	strW = GetDrawFormatStringWidth(fps.c_str());
	DrawBox(0, debugstrY, strW, debugstrY+16, GetColor(0, 0, 0), true);
	DrawFormatString(0, debugstrY, GetColor(255, 255, 255), fps.c_str());
	debugstrY += 16;


	std::string songhandle = std::to_string(CD.songData.songHandle.handle);
	strW = GetDrawFormatStringWidth(songhandle.c_str());
	DrawBox(0, debugstrY, strW, debugstrY+16, GetColor(0, 0, 0), true);
	DrawFormatString(0, debugstrY, GetColor(255, 255, 255), songhandle.c_str());
	debugstrY += 16;


}

void PlayScene::Input() {

	CD.Input();
}