#include "LoadingChart.h"


#include "File/ChartLoader.h"
#include "Chart/CourseType.h"
#include "Scene/SceneContexts.h"

LoadingChart::LoadingChart(GameContext* ctx) {
	
//	ctx->chartData = ChartLoad::load(ctx->songItemdata.tjaPath.c_str(), ctx->songItemdata.);
	
	RequestScene(SceneID::Play);
}

void LoadingChart::Init() {

}

void LoadingChart::Update() {

}

void LoadingChart::Draw() {

}

void LoadingChart::Finalize() {

}

void LoadingChart::Input() {

}