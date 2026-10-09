#pragma once
// ChartDrawer.h
#include "Chart/ChartData.h"

#include "BackgroundRenderer.h" // 背景描画担当クラス
#include "NoteRenderer.h"
#include <memory>

// 譜面の描画を担当するクラス
class ChartDrawer
{ 
public:

	ChartDrawer( ChartData& cd ): cd_( cd ) { 


		nr_ = std::make_unique<NoteRenderer>(cd);

	} // PlaySceneがシーンを生成するときに受け取る

	void Init();
	void Update( const ChartPlayer& cp_ );
	void Draw( const ChartPlayer& cp_ );
	

private:
	ChartData& cd_;	// PlaySceneが保有するChartDataのポインタを保持し、これを参照して描画する
	std::unique_ptr<BackgroundRenderer> bg_;
	std::unique_ptr<NoteRenderer> nr_;



};

