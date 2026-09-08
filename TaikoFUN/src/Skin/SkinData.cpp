#include "SkinData.h"
#include "DxLib.h"

#include <memory>
#include <cassert>



namespace {
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
	
	emplaceImg("play/bg", "Resource/Image/SkinColor/bgstage.png");
	emplaceImg("play/minitaiko", "Resource/Image/SkinColor/minitaiko.png");
	emplaceImg("play/ScrollField/bg", "Resource/Image/Playing/scrollfield_bg.png");
	emplaceImg("play/ScrollField/don", "Resource/Image/Playing/scrollfield_don.png");
	emplaceImg("play/ScrollField/katsu", "Resource/Image/Playing/scrollfield_ka.png");
	emplaceImg("play/ScrollField/hit", "Resource/Image/Playing/scrollfield_hit.png");

}

void SkinData::LoadNotesImgs(const std::string& path) {
	int baseImgHandle = LoadGraph(path.c_str());
	int baseImgW = 0, baseImgH = 0;
	GetGraphSize(baseImgHandle, &baseImgW, &baseImgH);

	int handles[15];
	int DivNum = 15;
	int DivX = 15;
	int DivY = 1;
	int XSize = baseImgW / DivX;
	int YSize = baseImgH / DivY;
	LoadDivGraph(path.c_str(), DivNum, DivX, DivY, XSize, YSize, handles);

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
	emplaceImg("note/BalloonBody",	handles[12], XSize, YSize, path);			// 風船(体)
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
}