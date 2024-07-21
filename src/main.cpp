#include "Client.hpp"

#include <steam/steam_api.h>

#include "Phys.hpp"

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

	Client client = Client();
	// client.loadWorldFrom("saves/first.world");
	client.mainloop();
	// client.saveWorldTo("saves/first.world");

	// SteamAPI_Shutdown();
	return 0;
}

