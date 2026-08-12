#include "DxLib.h"
#include "main.h"







GameState gGameState = GameState::Debug;

// プログラムは WinMain から始まります
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}


	while (CheckKeyInput(KEY_INPUT_ESCAPE))
	{

	Update();

	Draw();
	
	}
	

	DxLib_End();				// ＤＸライブラリ使用の終了処理

	return 0;				// ソフトの終了 
}

void Update() {

	switch (gGameState)
	{
	case GameState::Title:

		break;
	case GameState::Playing:

		break;	
	case GameState::Result:

		break;
	case GameState::Debug:
		
		break;
	default:
		break;
	}

}

void Draw() {

	switch (gGameState)
	{
	case GameState::Title:

		break;
	case GameState::Playing:

		break;
	case GameState::Result:

		break;
	case GameState::Debug:

		break;
	default:
		break;
	}



}
