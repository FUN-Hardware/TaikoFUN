#pragma once

#include "Chart/ChartData.h"
#include "Chart/Note.h"

class NoteRenderer
{ 
public:

	NoteRenderer(ChartData& cd) : cd_( cd ) {

	}

	void Draw();
	void Update();



private:
	ChartData& cd_;

};

