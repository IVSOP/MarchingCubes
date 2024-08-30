#include "Audio.hpp"
#include <vector>

using namespace Audio;

std::unique_ptr<std::unordered_map<std::string, Buffer>> ALContext::name_to_buff = std::make_unique<std::unordered_map<std::string, Buffer>>();
ALCdevice *ALContext::device = nullptr;
ALCcontext *ALContext::context = nullptr;

Buffer &ALContext::createBufferFromWavIfNotExists(const std::string &filename) {
	auto lookup = ALContext::name_to_buff->find(filename);
	if (lookup != name_to_buff->end()) {
		return lookup->second;
	}

	// else inser it...

	// Load WAV file using libsndfile
	SF_INFO sfInfo;
	SNDFILE* sndFile = sf_open(filename.c_str(), SFM_READ, &sfInfo);
	CRASH_IF(!sndFile, "Failed to open WAV file: " + filename);

	// Read WAV data
	std::vector<short> samples(sfInfo.frames * sfInfo.channels);
	sf_read_short(sndFile, samples.data(), samples.size());
	sf_close(sndFile);

	// ????????????????????
	Buffer &buffer = ALContext::name_to_buff->emplace(std::piecewise_construct, std::forward_as_tuple(filename), std::forward_as_tuple()).first->second;
	// printf("channels %u size %lu rate %u\n", sfInfo.channels, samples.size(), sfInfo.samplerate);
	ALCall(alBufferData(buffer.id, sfInfo.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, 
				samples.data(), samples.size() * sizeof(short), sfInfo.samplerate));

	return buffer;
}

Buffer &ALContext::createBufferFromWav(const std::string &filename) {
	// Load WAV file using libsndfile
	SF_INFO sfInfo;
	SNDFILE* sndFile = sf_open(filename.c_str(), SFM_READ, &sfInfo);
	CRASH_IF(!sndFile, "Failed to open WAV file: " + filename);

	// Read WAV data
	std::vector<short> samples(sfInfo.frames * sfInfo.channels);
	sf_read_short(sndFile, samples.data(), samples.size());
	sf_close(sndFile);

	// ????????????????????
	Buffer &buffer = ALContext::name_to_buff->emplace(std::piecewise_construct, std::forward_as_tuple(filename), std::forward_as_tuple()).first->second;
	// printf("channels %u size %lu rate %u\n", sfInfo.channels, samples.size(), sfInfo.samplerate);
	ALCall(alBufferData(buffer.id, sfInfo.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, 
				samples.data(), samples.size() * sizeof(short), sfInfo.samplerate));

	return buffer;
}

void ALContext::setupContext()
{
	// Initialization
	ALContext::device = alcOpenDevice(nullptr);
	CRASH_IF(!ALContext::device, "Failed to open OpenAL device");

	// context
	ALContext::context = alcCreateContext(device, nullptr);
	CRASH_IF(!ALContext::context || !alcMakeContextCurrent(ALContext::context), "Failed to create or make OpenAL context current");


	ALCall(alDistanceModel(AL_INVERSE_DISTANCE));

	// // buffer
	// Buffer &buffer = createBufferFromWav("song.wav");

	// // source
	// ALuint source;
	// ALCall(alGenSources(1, &source));
	// ALCall(alSourcei(source, AL_BUFFER, buffer.id));

	// ALCall(alSourcePlay(source));

	// ALCall(alDeleteSources(1, &source));

	// Wait for the sound to finish playing
	// ALint sourceState;
	// do {
	// 	ALCall(alGetSourcei(source, AL_SOURCE_STATE, &sourceState));
	// } while (sourceState == AL_PLAYING);
}

void ALContext::destroyContext() {
	name_to_buff = nullptr; // destroy buffers
	ALContext::context = alcGetCurrentContext();
	ALContext::device = alcGetContextsDevice(ALContext::context);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(ALContext::context);
	alcCloseDevice(ALContext::device);
}

void ALContext::setListenerPosition(const glm::vec3 &pos) {
	ALCall(alListenerfv(AL_POSITION, glm::value_ptr(pos)));
}

void ALContext::setListenerVelocity(const glm::vec3 &vel) {
	ALCall(alListenerfv(AL_VELOCITY, glm::value_ptr(vel)));
}

// TODO slightly improve this mess
void ALContext::setListenerOrientation(const glm::vec3 &front, const glm::vec3 &up) {
	ALfloat rot[6] = {front.x, front.y, front.z, up.x, up.y, up.z};

	ALCall(alListenerfv(AL_ORIENTATION, rot));
}
