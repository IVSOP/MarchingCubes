#include "Client.hpp"

// #include <steam/steam_api.h>
#include <steam/steam_api.h>

int main(int argc, char **argv) {
	if (!SteamAPI_Init()) {
		exit(EXIT_FAILURE);
	}

	Client client = Client();
	// client.loadWorldFrom("saves/first.world");
	client.mainloop();
	// client.saveWorldTo("saves/first.world");

	SteamAPI_Shutdown();
	return 0;
}

