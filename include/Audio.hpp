#ifndef AUDIO_H
#define AUDIO_H

// #include <phonon.h>
#include <sndfile.h>
#include "stdlib.hpp"
#include "ALErrors.hpp"
#include "Crash.hpp"
#include <unordered_map>

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

class ALContext {
public:
	Buffer &createBufferFromWav(const std::string &filename) {
		// Load WAV file using libsndfile
		SF_INFO sfInfo;
		SNDFILE* sndFile = sf_open(filename.c_str(), SFM_READ, &sfInfo);
		CRASH_IF(!sndFile, "Failed to open WAV file: " + filename);

		// Read WAV data
		std::vector<short> samples(sfInfo.frames * sfInfo.channels);
		sf_read_short(sndFile, samples.data(), samples.size());
		sf_close(sndFile);

		// ????????????????????
		Buffer &buffer = name_to_buff->emplace(std::piecewise_construct, std::forward_as_tuple(filename), std::forward_as_tuple()).first->second;
		// printf("channels %u size %lu rate %u\n", sfInfo.channels, samples.size(), sfInfo.samplerate);
		ALCall(alBufferData(buffer.id, sfInfo.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, 
					samples.data(), samples.size() * sizeof(short), sfInfo.samplerate));

		return buffer;
	}

	ALContext()
	: name_to_buff(std::make_unique<std::unordered_map<std::string, Buffer>>())
	{
		// Initialization
		device = alcOpenDevice(nullptr);
		CRASH_IF(!device, "Failed to open OpenAL device");

		// context
		ALCcontext* context = alcCreateContext(device, nullptr);
		CRASH_IF(!context || !alcMakeContextCurrent(context), "Failed to create or make OpenAL context current");


		// buffer
		Buffer &buffer = createBufferFromWav("song.wav");

		// source
		ALuint source;
		ALCall(alGenSources(1, &source));
		ALCall(alSourcei(source, AL_BUFFER, buffer.id));

		ALCall(alSourcePlay(source));

		// ALCall(alDeleteSources(1, &source));

		// Wait for the sound to finish playing
		// ALint sourceState;
		// do {
		// 	ALCall(alGetSourcei(source, AL_SOURCE_STATE, &sourceState));
		// } while (sourceState == AL_PLAYING);
	}
	~ALContext() {
		name_to_buff = nullptr; // destroy buffers
		context = alcGetCurrentContext();
		device = alcGetContextsDevice(context);
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(context);
		alcCloseDevice(device);
	}

private:
	ALCdevice *device = nullptr;
	ALCcontext* context = nullptr;

	// each filename has a dedicated buffer
	// unique ptr to easily manually dfestruct it
	std::unique_ptr<std::unordered_map<std::string, Buffer>> name_to_buff;
};

#endif
