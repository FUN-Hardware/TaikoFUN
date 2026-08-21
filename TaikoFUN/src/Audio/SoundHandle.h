#pragma once



#include "DxLib.h"
#include <string>

struct SoundHandle
{

	int handle = -1;

	SoundHandle() = default;
	SoundHandle(std::string path) {
		handle = LoadSoundMem(path.c_str());
	}
	~SoundHandle() {
		StopSoundMem(handle);		// サウンドを削除する前に明示的に止めておかないとフリーズするため、自動で停止してから削除するクラスを作ったよ
		DeleteSoundMem(handle);
	}

	SoundHandle(const SoundHandle&) = delete;             // 「新規に作りながらコピーする」動作そのものを封印
	SoundHandle& operator=(const SoundHandle&) = delete;  // 「既存の変数に上書きコピーする」動作そのものを封印  ありがとうClaude

	void load(const char* path) {
		handle = LoadSoundMem(path);
	}


	void play(bool restart = false) {
		PlaySoundMem(handle, DX_PLAYTYPE_BACK, restart);
	}

	void stop() {
		StopSoundMem(handle);
	}

};
