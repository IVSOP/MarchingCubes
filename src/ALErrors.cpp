#include "ALErrors.hpp"

// true when error
bool ALGetError() {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
		Log::log(LOG_TYPE::ERR, "OpenAL error", "code is " + std::to_string(error));
		return true;
	}
	return false;
}

void ALClearError() {
	while (ALGetError());
}
