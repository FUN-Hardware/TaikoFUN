#include "Tween.h"
#include "Time.h"
#include "dxlib.h"


void Tween::Update() {

	elapsed += Time::deltaSec();

	isFinished = elapsed >= duration;
}

double Tween::GetValue(double dt, easeType type, easeMode mode) {
	if (isFinished) {
		return 1.0;
	}
	else {
		double t = elapsed / duration;

		switch (type) {
		case Linear:
			return easeLinear(t, mode);
		case Quad:
			return easeQuad(t, mode);
		// Add cases for other ease types
		}


	}
}

double Tween::easeLinear(double t, easeMode mode) {
	return (mode == easeMode::in) ? t : 1.0 - (1.0 - t);
}

double Tween::easeQuad(double t, easeMode mode) {
	return (mode == easeMode::in) ? t * t : 1.0 - (1.0 - t) * (1.0 - t);
}