#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <memory>
#include <vector>
#include "common.hpp"
#include "World.hpp"
#include "Player.hpp"

#define MAX_KEYS_ID GLFW_KEY_MENU

// TODO
// const int scancode = glfwGetKeyScancode(GLFW_KEY_X);
// set_key_mapping(scancode, swap_weapons);

// TODO raw mouse

// problem: callbacks are only called when keys are pressed or released
// so the previous state is not the state at the previous time it was read, but at the previous callback
// in the future I might manually poll everything
// prev state is kind of useless because of this

struct KeyInfo {
	KeyInfo() {
		action = GLFW_RELEASE;
		mods = 0;

		prev_action = GLFW_RELEASE;
		prev_mods = 0;
	}
	~KeyInfo() = default;

	constexpr void press(int mods = 0) {
		// action = GLFW_PRESS;
		newAction(action, mods);
	}

	constexpr void newAction(int _action, int _mods) {
		prev_action = this->action;
		prev_mods = this->mods;

		action = _action;
		mods = _mods;
	}

	constexpr void newAction(int _action) {
		prev_action = this->action;
		prev_mods = this->mods;

		action = _action;
		mods = this->mods;
	}

	constexpr void set_prev(int _prev_action, int _prev_mods = 0) {
		prev_action = _prev_action;
		prev_mods = _prev_mods;
	}

	constexpr void release() {
		// action = GLFW_RELEASE;
		newAction(GLFW_RELEASE, 0);
	}

	constexpr bool single_click() {
		// if action was a release, cannot have clicked, and set the flag to false
		if (action == GLFW_RELEASE) {
			single_clicked = false;
			return false;
		}

		if (single_clicked) {
			// if previously single clicked, now it has to be false, but keep the flag for the next timr
			return false;
		} else {
			single_clicked = true;
			return true;
		}
	}

	int action;
	int mods;

	int prev_action;
	int prev_mods;

	bool single_clicked = false;
};

class InputHandler {
public:
	// Estado sobre teclas pressionadas, rato, etc
	// WTF why not use a vector or something
	std::unique_ptr<KeyInfo []> keyInfo; // [MAX_KEYS_ID + 1]

	// mouse
	GLdouble curX;
	GLdouble curY;
	GLdouble lastX;
	GLdouble lastY;

	bool inMenu; // faz o rato nao virar a camera

	GLdouble scroll_x;
	GLdouble scroll_y;

	// Get this out of here asap
	GLFWcursorposfun handleMouseMov = nullptr;
	GLFWmousebuttonfun handleMouseKey = nullptr;

	InputHandler() = delete;
	InputHandler(GLFWcursorposfun mouse_mov_callback, GLFWmousebuttonfun mouse_keypress_callback);
	~InputHandler() = default;

	void pressKey(GLFWwindow *window, int key, int scancode, int action, int mods);
	void pressMouseKey(GLFWwindow* window, int button, int action, int mods);
	void scroll(double xoffset, double yoffset);
	void centerMouseTo(GLdouble center_x, GLdouble center_y); // same as below but also changes the last position
	void moveMouseTo(GLdouble x, GLdouble y);

	// devolve teclas com estado diferente de RELEASE
	std::vector<KeyInfo> getKeysPressedOrHeld() const;
	// delta em que o rato se moveu
	// glm::vec2 getMouseMovDelta() const;

	void poll(GLFWwindow *window);

	// changes camera
	// pointers because its easier, idk if using references would be copying the object (in the caller)
	void move(World *world, Player *player, int windowWidth, int windowHeight, GLfloat deltatime);
	// returns true if the key was clicked, but only returns true again after it has been released once
	bool single_click(uint32_t key);
	bool clicked(uint32_t key);
	const KeyInfo *get(uint32_t keyid);

	GLfloat getXScroll();
	GLfloat getYScroll();
};

#endif
