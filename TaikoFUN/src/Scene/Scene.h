#pragma once

/// <summary>
/// シーンの基底クラス
/// </summary>
#include <optional>
#include "SceneID.h"




class Scene
{ 
public:  

	Scene() = default;
	virtual ~Scene() = default;	// 
	
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Finalize() = 0;

	virtual void Input() = 0;
	const std::optional<SceneID> Request() const {
		return request_;
	}


protected:
	void RequestScene( SceneID req ) {
		if ( !request_ ) request_ = req;
	}

private:


	std::optional<SceneID> request_;

};

