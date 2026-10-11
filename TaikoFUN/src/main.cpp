#include <cstdlib>
#include <memory>
#include <vector>


#include "DxLib.h"
#include "main.h"
#include "Scenes.h"
#include "Input/Input.h"
#include "Debug/Fps.h"
#include "Core/Tween.h"
#include "Core/Time.h"
#include "Skin/SkinData.h"
#include "Core/ChartScanner.h"
#include "Scene/SceneContexts.h"
#include "Scene/SceneManager.h"
#include "File/FindAllTJA.h"
#include "File/ChartLoader.h"
#include "Overray/AdjustJudgeOffset.h"

GameState gGameState = GameState::Playing;
GameState preGameState = GameState::Null;



std::unique_ptr<Debug> db;
std::unique_ptr<PlayScene> ps; // プレイシーン
ChartScanner CS;
std::vector<ChartScanner::ChartFile> ChartList;
std::unique_ptr<AdjustJudgeOffset> ov;

// プログラムは WinMain から始まります

bool isWindowScreen = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {



	SetGraphMode(1280, 720, 32);
	ChangeWindowMode(TRUE);
	SetWindowSizeExtendRate(1.0);
	SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);

	if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}

	SetOutApplicationLogValidFlag(FALSE);
	SetWaitVSyncFlag( FALSE );
	SetDrawMode(DX_DRAWMODE_BILINEAR);


	Skin::loadSkin();

	ChartList = CS.scanCharts("Songs");

	// シーン関係の初期化を行う


	GameContext ctx;

	SceneManager sm;
	
	ov = std::make_unique<AdjustJudgeOffset>(&ctx);

	// 仮
	
	std::vector<std::string> tempTjaPath = FindAllTjaFiles( "Songs" );

	if ( tempTjaPath.empty() ) {
		printfDx( "Songs に tja が見つかりません\n" );   // 仮の表示でよい
		// ここで止めるか return する
	}
	ctx.chartData = ChartLoad::load( tempTjaPath[0].c_str(), CourseType::Hard );

	sm.Init( SceneID::SongSelect, &ctx );


	while (CheckHitKey(KEY_INPUT_ESCAPE) == 0 && ProcessMessage() == 0)
	{





	Update(sm);

	Draw(sm);
	
	}
	
	sm.Finalize();
	ctx.resultData.reset();
	ctx.chartData.reset();
	db.reset();
	ps.reset();
	Skin::unloadSkin();
	DxLib_End();				// ＤＸライブラリ使用の終了処理

	return 0;				// ソフトの終了
}

void Update(SceneManager& sm) {


	
	Time::Update();


	Input::Update();
	FPS::Update();

	if ( Input::isKeyTriggered( KEY_INPUT_F11 ) ) {
		isWindowScreen = !isWindowScreen;
		ChangeWindowMode(isWindowScreen);
	}


	sm.Update();
	ov->Update();

}

void Draw(SceneManager& sm) {

	DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);


	sm.Draw();

	ov->Draw();

	ScreenFlip();



}
