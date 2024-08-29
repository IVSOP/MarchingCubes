#ifndef ALERRORS_H
#define ALERRORS_H

#include "Logs.hpp"
#include <AL/al.h>
#include <AL/alc.h>

#define ALCall(f) ALClearError();\
	f;\

void ALClearError();

#endif
