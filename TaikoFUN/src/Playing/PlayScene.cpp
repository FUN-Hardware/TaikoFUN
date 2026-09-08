#include "DxLib.h"
#include "PlayScene.h"
#include "Debug/FPS.h"

#include "Input/Input.h"
#include "Skin/SkinData.h"
#include "Core/General.h"
#include "Core/ChartScanner.h"
#include "File/ChartLoader.h"
#include "File/FindAllTJA.h"

namespace SkinLayout {

	Vector2d ScrollField{ 325, 254 - 65 };
	int NoteSize = 128;

};
namespace fs = std::filesystem;


PlayScene::PlayScene() {

	//	ChartLoad::load(u8"(Songs/続・〆ドレー2000/続・〆ドレー2000.tja)", CD, CourseType::Oni);
	//	ChartLoad::load("Songs/シャイニングスター/シャイニングスター.tja", CD, CourseType::Oni);
	debug = "走査対象のパス: " + fs::absolute("Songs").string(); // 絶対パスに変換して出力
	tempTjaPath = FindAllTjaFiles("Songs");
	ChartLoad::load(tempTjaPath[1].c_str(), CD, CourseType::Oni);
		//CD.loadSong("Resource/Debug/カンケーガール.mp3", 185.0, 4.2);
	double soundVol = 0.8;
	ChangeVolumeSoundMem(255 * soundVol, CD.songData.songHandle.handle);
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
	std::vector<Note> InversedNotes(CD.notes.rbegin(), CD.notes.rend()); // 逆順にすることで、後ろのノーツから描画するようにする
	for (const auto& note : InversedNotes) {

		noteRelativeTime = CD.noteRelativeTime(CD.notes.size() - 1 - index);
		noteX = ((noteRelativeTime / 1000000.0) / (240.0 / note.bpm)) * 900.0 * note.scroll + SkinLayout::ScrollField.x;
		// 240/BPM = 1小節の秒数。1小節当たり960pxとする。よって、(相対時間)/(240/BPM) * 960 = ノーツのX座標
		NoteType drawType = note.type;
		if (note.type == NoteType::None){
			index++;
			continue; // 0(空白ノーツは描画しない)
		}
		if (noteX < 1300 && noteX+SkinLayout::NoteSize > 0 && !note.isJudged)DrawExtendGraph(noteX, SkinLayout::ScrollField.y, noteX+SkinLayout::NoteSize, SkinLayout::ScrollField.y+SkinLayout::NoteSize, Skin::GetTexture(GetNoteImageKey(drawType)).handle, true); // 画面内のみ描画
		index++;
	}
	CD.notes; // デバッグで内部数値を確認する用

	std::string str = "";
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
	////
	str = debug;
	strW = GetDrawFormatStringWidth(str.c_str());
	DrawBox(0, debugstrY, strW, debugstrY + 16, GetColor(0, 0, 0), true);
	DrawFormatString(0, debugstrY, GetColor(255, 255, 255), str.c_str());
	debugstrY += 16;

	str = "[0]: " + tempTjaPath[0];
	strW = GetDrawFormatStringWidth(str.c_str());
	DrawBox(0, debugstrY, strW, debugstrY + 16, GetColor(0, 0, 0), true);
	DrawFormatString(0, debugstrY, GetColor(255, 255, 255), str.c_str());
	debugstrY += 16;

	str = "[1]: " + tempTjaPath[1];
	strW = GetDrawFormatStringWidth(str.c_str());
	DrawBox(0, debugstrY, strW, debugstrY + 16, GetColor(0, 0, 0), true);
	DrawFormatString(0, debugstrY, GetColor(255, 255, 255), str.c_str());
	debugstrY += 16;

	str = "Songs/シャイニングスター/シャイニングスター.tja"; // 自分で、"/"だけを使って、ハードコードする
	strW = GetDrawFormatStringWidth(str.c_str());
	DrawBox(0, debugstrY, strW, debugstrY + 16, GetColor(0, 0, 0), true);
	DrawFormatString(0, debugstrY, GetColor(255, 255, 255), "%s", str.c_str());
	debugstrY += 16;
	// tempTjaPathなど、これまでの複雑な経路を、一旦すべて無視する
	DrawFormatString(0, 100, GetColor(255, 255, 255), "シャイニングスター"); // 直接、ハードコードで試す
	////
	str = "path: " + CD.tjaPath;
	strW = GetDrawFormatStringWidth(str.c_str());
	DrawBox(0, debugstrY, strW, debugstrY + 16, GetColor(0, 0, 0), true);
	DrawFormatString(0, debugstrY, GetColor(255, 255, 255), str.c_str());
	debugstrY += 16;
	////
	str = "TITLE: " + CD.Title;
	strW = GetDrawFormatStringWidth(str.c_str());
	DrawBox(0, debugstrY, strW, debugstrY + 16, GetColor(0, 0, 0), true);
	DrawFormatString(0, debugstrY, GetColor(255, 255, 255), str.c_str());
	debugstrY += 16;

	str = "SUBTITLE: " + CD.subTitle;
	strW = GetDrawFormatStringWidth(str.c_str());
	DrawBox(0, debugstrY, strW, debugstrY + 16, GetColor(0, 0, 0), true);
	DrawFormatString(0, debugstrY, GetColor(255, 255, 255), str.c_str());
	debugstrY += 16;

	str = "BPM: " + std::to_string(CD.bpm);
	strW = GetDrawFormatStringWidth(str.c_str());
	DrawBox(0, debugstrY, strW, debugstrY + 16, GetColor(0, 0, 0), true);
	DrawFormatString(0, debugstrY, GetColor(255, 255, 255), str.c_str());
	debugstrY += 16;

	str = "LEVEL: " + std::to_string(CD.level);
	strW = GetDrawFormatStringWidth(str.c_str());
	DrawBox(0, debugstrY, strW, debugstrY + 16, GetColor(0, 0, 0), true);
	DrawFormatString(0, debugstrY, GetColor(255, 255, 255), str.c_str());
	debugstrY += 16;

	extern std::vector<ChartScanner::ChartFile> ChartList;
	for(const auto& chartfile : ChartList){
		std::string path = chartfile.ChartPath.string();
		strW = GetDrawFormatStringWidth(path.c_str());
		DrawBox(0, debugstrY, strW, debugstrY + 16, GetColor(0, 0, 0), true);
		DrawFormatString(0, debugstrY, GetColor(255, 255, 255), path.c_str());
		debugstrY += 16;
	}

}

void PlayScene::Input() {

	CD.Input();
}