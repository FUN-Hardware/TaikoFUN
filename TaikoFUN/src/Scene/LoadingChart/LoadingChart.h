#pragma once
#include "Scene/Scene.h"
#include "Scene/SceneContexts.h"

enum class CourseType;

class LoadingChart:
    public Scene
{ 





public:

    LoadingChart(GameContext* ctx, const char* path, CourseType ct);

    void Init() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

    void Input() override;


};

