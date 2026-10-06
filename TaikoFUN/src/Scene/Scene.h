#pragma once

/// <summary>
/// シーンの基底クラス
/// </summary>

class Scene
{ 
	
	virtual ~Scene() = default;	// 

public:
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Finalize() = 0;

};

