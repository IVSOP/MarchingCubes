#ifndef CLIENT_H
#define CLIENT_H

#include <memory>
#include "Player.hpp"
#include "World.hpp"
#include "InputHandler.hpp"
class WindowManager;
#include "WindowManager.hpp"
#include "Renderer.hpp"
#include "Audio.hpp"

#ifdef PROFILING
	#include <valgrind/callgrind.h>
#endif

#include "PhysRenderer.hpp"

class Client {
public:
	std::unique_ptr<WindowManager> windowManager = nullptr;
	std::unique_ptr<World> world = nullptr;
	std::unique_ptr<Player> player = nullptr;
	std::unique_ptr<Renderer> renderer = nullptr;
	InputHandler inputHandler;

	// get this out of here please
	bool resize = false;

	Client(PhysRenderer *phys_renderer);
	~Client() = default;

	// these are here so that windowManager can call them, since it only has the client
	void resizeViewport(int windowWidth, int windowHeight);
	void pressKey(GLFWwindow *window, int key, int scancode, int action, int mods);
	void moveMouseTo(double xpos, double ypos);
	void centerMouseTo(double xpos, double ypos);
	void scroll(double xoffset, double yoffset);
	void pressMouseKey(GLFWwindow* window, int button, int action, int mods);

	void mainloop();
	void saveWorldTo(const std::string &filepath) const;
	void loadWorldFrom(const std::string &filepath);
};

#endif
