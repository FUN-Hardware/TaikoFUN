#include "LoadingChart.h"


#include "File/ChartLoader.h"
#include "Chart/CourseType.h"
#include "Scene/SceneContexts.h"

LoadingChart::LoadingChart(GameContext* ctx, const char* path, CourseType ct) {
	
	ctx->chartData = ChartLoad::load(path, ct);

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