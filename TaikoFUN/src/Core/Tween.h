#pragma once

#include <functional>

struct Tween
{


	double startVal, endVal;
	double duration;
	double elapsed = 0.f;

	bool isFinished;

	enum easeMode {
		in,
		out,
		inOut,
		outIn,
	};

	enum easeType {
		Linear,
		Quad,
		Cubic,
		Quart,
		Quint,
		Sine,
		Expo,
		Circ,
		Back,
		Elastic,
		Bounce,
	};

	void Update();

	double GetValue(double dt, easeType type, easeMode mode);


	double easeLinear(double t, easeMode mode);
	double easeQuad(double t, easeMode mode);
	double easeCubic(double t, easeMode mode);
	double easeQuart(double t, easeMode mode);
	double easeQuint(double t, easeMode mode);
	double easeSine(double t, easeMode mode);
	double easeExpo(double t, easeMode mode);
	double easeCirc(double t, easeMode mode);
	double easeBack(double t, easeMode mode);
	double easeElastic(double t, easeMode mode);
	double easeBounce(double t, easeMode mode);


};