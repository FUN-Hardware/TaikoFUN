#pragma once
#include "Scene/Scene.h"
#include "Scene/SceneContexts.h"
#include "ResultLogic.h"
#include <memory>
#include <string>

class ResultScene : public Scene {
public:
    explicit ResultScene(GameContext* ctx) : ctx_(ctx) {}
    ~ResultScene() override { Finalize(); }
    void Init() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;
    void Input() override;
private:
    void Text(int x, int y, const std::string& text, int font, unsigned int color);
    void CenterText(int x, int y, const std::string& text, int font, unsigned int color);
    GameContext* ctx_;
    std::shared_ptr<const ResultCtx> data_;
    ResultLogic::Summary summary_;
    ResultLogic::Animation animation_;
    int labelFont_ = -1, titleFont_ = -1, numberFont_ = -1, rankFont_ = -1;
};
