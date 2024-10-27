#ifndef ALERRORS_H
#define ALERRORS_H

#include "Logs.hpp"
#include <AL/al.h>
#include <AL/alc.h>

#if (DISTRIBUTION)
	#define ALCall(f) ALClearError();\
		f;
#else
	#define ALCall(f) f;
#endif

void ALClearError();

#endif
