#include "SceneManager.h"

#include "Scene.h"
#include "SceneContexts.h"
// 匿名空間によって、SceneManagerクラスを外部からアクセスできないようにする。(ファサードパターン)

namespace {
	class SceneManager
	{
	private:

		Scene* currentScene = nullptr; // 現在のシーンを保持するポインタ
		GameContext* ctx_; // ゲームの状態を保持するコンテキスト
		
	public:
		void Init() {
			// 初期化処理

		}

		void Update() {
			// 更新処理

		}

		void Draw() {
			// 描画処理

		}

		void Finalize() {
			// 終了処理

		}

		void ChangeScene( Scene* newScene ) {
			// シーンの切り替え処理

		}
	};

	SceneManager g_sceneManager;
}


namespace SceneManager {


}