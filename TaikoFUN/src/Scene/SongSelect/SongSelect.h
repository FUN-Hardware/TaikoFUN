#pragma once


#include "Scene/Scene.h"
#include "SongItemData.h"
#include "Scene/SceneContexts.h"
#include "SelectedCourse.h"
#include "Audio/SongData.h"

#include <memory>
#include <vector>
#include <string>
class SongSelect : public Scene
{ 
public:

    SongSelect( GameContext* ctx );

    void Init() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;
    
    void Input() override;


private:

    std::vector<SongItemData> songList;
    size_t nowSelectedSongIdx = 0;
    size_t nowSelectedCourseIdx = 0;
    GameContext* ctx_;
    SelectedCourse sc_;
    bool choosingCourse = false;

    std::unique_ptr<SongData> demo;

    void playDemo();

    const long long demoInterval = 500000;
    long long intervalLeft = 0;



};

