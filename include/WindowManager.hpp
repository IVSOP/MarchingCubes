#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

class Client;
#include "common.hpp"
#include "Client.hpp"

void glfw_handleKey_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void glfw_handleMouseMov_callback(GLFWwindow *window, double xpos, double ypos);
void glfw_handleMouseKey_callback(GLFWwindow* window, int button, int action, int mods);

class WindowManager {
public:
	int windowWidth;
	int windowHeight;
	GLfloat aspectRatio = 1.0f;

	// std::mutex mtx;
	GLFWwindow *window = nullptr;

	glm::mat4 projection = glm::mat4(1.0f);

	WindowManager() = delete; // in the future make this
	WindowManager(int windowWidth, int windowHeight, Client *client);
	~WindowManager();

	static void fovCallback(void *_windowmanager, const void *_data);
	void updateProjection(GLfloat fov);
	void resizeViewport(int windowWidth, int windowHeight);
};

#endif
