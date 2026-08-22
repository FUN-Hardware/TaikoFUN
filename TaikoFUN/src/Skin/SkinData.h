#pragma once

#include <unordered_map>
#include <string>

#include "DxLib.h"
#include "Audio/SoundHandle.h"

struct imgData {

	int handle = -1;
	int w = 0, h = 0;
	std::string path;
	
	void load(const char* path) {
		handle = LoadGraph(path);
		GetGraphSize(handle, &w, &h);
		this->path = path;
	}

	imgData() = default;
	imgData(std::string path) {
		handle = LoadGraph(path.c_str());
		GetGraphSize(handle, &w, &h);
		this->path = path;
	}
	imgData(int handle, int w, int h, std::string path) {
		this->handle = handle;
		this->w = w;
		this->h = h;
		this->path = path;
	}
	~imgData() {
		DeleteGraph(handle);
	}

	imgData(const imgData&) = delete;
	imgData& operator=(const imgData&) = delete;
	//imgData(imgData&&) = default;             // ムーブは許可する
	//imgData& operator=(imgData&&) = default;  // ムーブ代入も許可する
};


namespace Skin {
	void loadSkin();
	imgData& GetTexture(const std::string& key);
	SoundHandle& GetSound(const std::string& key);
}
