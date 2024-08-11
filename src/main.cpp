#include "Client.hpp"

#include <steam/steam_api.h>

#include "Phys.hpp"

#include <tracy/Tracy.hpp>

#define APP_ID 480

int main(int argc, char **argv) {
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
	Client client = Client(Phys::getPhysRenderer());
	// client.loadWorldFrom("saves/first.world");
	client.mainloop();
	// client.saveWorldTo("saves/first.world");

	// SteamAPI_Shutdown();
	return 0;
}

