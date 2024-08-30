#include "Client.hpp"
#include <steam/steam_api.h>
#include "Phys.hpp"

#include "Audio.hpp"

#define APP_ID 480

int main(int argc, char **argv) {

	Audio::ALContext::setupContext();
	// if (SteamAPI_RestartAppIfNecessary(APP_ID)) {
	// 	return 1;
	// }

	// if (!SteamAPI_Init()) {
	// 	// TODO log error
	// 	exit(EXIT_FAILURE);
	// }

	// this is VERY BAD, REMOVE THIS, TEMPORARY
	Phys::setup_phys();

	// this is ALSO VERY BAD that is receives the debug renderer, TEMPORARY
	// cursed but I wanted to be able to manually destroy the client, fuck it
	// otherwise destroying audio context would happen before client is destroyed
	// TODO fix it somehow, make audio not be a static class??
	std::unique_ptr<Client> client = std::make_unique<Client>(Phys::getPhysRenderer());
	client->mainloop();

	client = nullptr;
	// SteamAPI_Shutdown();
	Audio::ALContext::destroyContext();
	return 0;
}

