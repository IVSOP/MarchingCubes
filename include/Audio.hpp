#ifndef AUDIO_H
#define AUDIO_H

// #include <phonon.h>
#include <sndfile.h>
#include "stdlib.hpp"
#include "ALErrors.hpp"
#include "Crash.hpp"
#include <unordered_map>
#include "types.hpp"

namespace Audio {

struct Buffer {
	Buffer() {
		ALCall(alGenBuffers(1, &id));
	}
	Buffer(ALuint id) : id(id) {}
	~Buffer() {
		ALCall(alDeleteBuffers(1, &id));
	}
	ALuint id;
};

struct Source {
	Source() {
		ALCall(alGenSources(1, &id));
		ALCall(alSourcei(id, AL_SOURCE_RELATIVE, AL_FALSE));
		ALCall(alSourcef(id, AL_GAIN, 1.0f));
		ALCall(alSourcef(id, AL_REFERENCE_DISTANCE, 1.0f));
	};

	Source(const Buffer &buffer) {
		ALCall(alGenSources(1, &id));
		ALCall(alSourcei(id, AL_BUFFER, buffer.id));
		ALCall(alSourcei(id, AL_SOURCE_RELATIVE, AL_FALSE));
		ALCall(alSourcef(id, AL_GAIN, 1.0f));
		ALCall(alSourcef(id, AL_REFERENCE_DISTANCE, 1.0f));
	}

	void setBuffer(const Buffer &buffer) const {
		ALCall(alSourcei(id, AL_BUFFER, buffer.id));
	}

	~Source() {
		ALCall(alDeleteSources(1, &id));
	}

	void play() const {
		ALCall(alSourcePlay(id));
	}

	void pause() const {
		ALCall(alSourcePause(id));
	}

	void setPosition(const glm::vec3 &pos) const {
		ALCall(alSourcefv(id, AL_POSITION, glm::value_ptr(pos)));
	}

	void setGain(const ALfloat gain) const {
		alSourcef(id, AL_GAIN, gain);
	}

	ALuint id;
};

class ALContext {
public:
	static Buffer &createBufferFromWav(const std::string &filename);
	static Buffer &createBufferFromWavIfNotExists(const std::string &filename);

	static void setupContext();

	static void destroyContext();

	static void setListenerPosition(const glm::vec3 &pos);
	static void setListenerVelocity(const glm::vec3 &vel);
	static void setListenerOrientation(const glm::vec3 &front, const glm::vec3 &up);

private:
	static ALCdevice *device;
	static ALCcontext* context;

	// each filename has a dedicated buffer
	// unique ptr to easily manually dfestruct it
	static std::unique_ptr<std::unordered_map<std::string, Buffer>> name_to_buff;
};


} // end namespace

#endif
