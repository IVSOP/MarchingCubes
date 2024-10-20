#include "InputHandler.hpp"

InputHandler::InputHandler(GLFWcursorposfun mouse_mov_callback, GLFWmousebuttonfun mouse_keypress_callback)
: keyInfo(std::make_unique<KeyInfo []>(MAX_KEYS_ID + 1)), curX(0.0f), curY(0.0f), lastX(0.0f), lastY(0.0f), inMenu(false), scroll_x(0.0f), scroll_y(0.0f), handleMouseMov(mouse_mov_callback), handleMouseKey(mouse_keypress_callback)
{

}

void handleMouseMovInMenu(GLFWwindow *window, double xpos, double ypos) {
	// maneira manhosa de meter o imgui clicavel
	ImGui::GetIO().AddMousePosEvent(xpos, ypos);
}

void handleMouseClickInMenu(GLFWwindow* window, int button, int action, int mods) {
	// maneira manhosa de meter o imgui clicavel
	ImGui::GetIO().AddMouseButtonEvent(button, action == GLFW_PRESS ? true : false);
}

void InputHandler::pressKey(GLFWwindow *window, int key, int scancode, int action, int mods) {
	KeyInfo *keys = this->keyInfo.get();

	keys[key].newAction(action, mods);

	// TODO bandaid fix temporario isto estar aqui, presskey nao devia fazer isto
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		if (inMenu) { // then go into engine
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetMouseButtonCallback(window, handleMouseKey);
			// ImGui::SetMouseCursor(ImGuiMouseCursor_None); // acho que isto nao e preciso
			inMenu = false;
			glfwSetCursorPosCallback(window, handleMouseMov);
			glfwSetCursorPos(window, curX, curY);
		} else { // then go into menu
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			// ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow); // acho que isto nao e preciso
			inMenu = true;
			glfwSetMouseButtonCallback(window, handleMouseClickInMenu);
			glfwSetCursorPosCallback(window, handleMouseMovInMenu);
		}
	}
}

void InputHandler::pressMouseKey(GLFWwindow* window, int button, int action, int mods) {
	KeyInfo *keys = this->keyInfo.get();

	keys[button].newAction(action, mods);
}

void InputHandler::scroll(double xoffset, double yoffset) {
	scroll_x += xoffset;
	scroll_y += yoffset;
}

void InputHandler::centerMouseTo(GLdouble center_x, GLdouble center_y) {
	curX = center_x;
	curY = center_y;
	lastX = center_x;
	lastY = center_y;
}

void InputHandler::moveMouseTo(GLdouble x, GLdouble y) {
	if (!inMenu) {
		this->curX = x;
		this->curY = y;
	}
}

std::vector<KeyInfo> InputHandler::getKeysPressedOrHeld() const {
	std::vector<KeyInfo> res;
	const KeyInfo *keys = keyInfo.get();
	for (unsigned int i = 0; i < MAX_KEYS_ID + 1; i++) {
		if (keys[i].action != GLFW_RELEASE) {
			res.push_back(keys[i]);
		}
	}
	return res;
}

// glm::vec2 InputHandler::getMouseMovDelta() const {
// 	return glm::vec2(curX - lastX, curY - lastY);
// }

void InputHandler::move(World *world, Player *player, int windowWidth, int windowHeight, GLfloat deltatime) {
	// muito mal feito, tbm nao tive paciencia mas funcemina

	const KeyInfo *keys = keyInfo.get();

	if ((&keys[GLFW_KEY_W])->action != GLFW_RELEASE) {
		player->move(FRONT, deltatime);
	}
	if ((&keys[GLFW_KEY_S])->action != GLFW_RELEASE) {
		player->move(BACK, deltatime);
	}
	if ((&keys[GLFW_KEY_A])->action != GLFW_RELEASE) {
		player->move(LEFT, deltatime);
	}
	if ((&keys[GLFW_KEY_D])->action != GLFW_RELEASE) {
		player->move(RIGHT, deltatime);
	}
	if ((&keys[GLFW_KEY_SPACE])->action != GLFW_RELEASE) {
		player->move(UP, deltatime);
	}
	if ((&keys[GLFW_KEY_LEFT_ALT])->action != GLFW_RELEASE) {
		player->move(DOWN, deltatime);
	}
	if ((&keys[GLFW_KEY_LEFT_SHIFT])->action != GLFW_RELEASE) {
		player->speedUp(true);
	} else {
		player->speedUp(false);
	}

	if (!inMenu) {
		// const int center_x = windowWidth / 2;
		// const int center_y = windowHeight / 2;

		const float xoffset = static_cast<GLfloat>(curX) - static_cast<GLfloat>(lastX);
		const float yoffset = static_cast<GLfloat>(lastY) - static_cast<GLfloat>(curY); // reversed since y-coordinates go from bottom to top

		lastX = curX;
		lastY = curY;

		// printf("Camera moving mouse from %f %f to %f %f\n", lastX, lastY, curX, curY);
		player->look(xoffset, yoffset);
	}
}


bool InputHandler::single_click(uint32_t keyid) {
	KeyInfo *keys = keyInfo.get();
	KeyInfo *key = &keys[keyid];

	return key->single_click();

	// int action = key->action;

	// if (action == GLFW_RELEASE) {
	// 	key->single_clicked = false;
	// 	return false;
	// }

	// // if (action == GLFW_PRESS) {
	// 	if (key->single_clicked) { // flag is set, so a single click occurred and the player still has not released the key
	// 		return false;
	// 	} else { // single_clicked not set, so this is a single click. set the flag to tell it was a single click
	// 		key->single_clicked = true;
	// 		return true;
	// 	}
	// // }

	// // return (key->action == GLFW_PRESS && key->prev_action != GLFW_PRESS);
}

void InputHandler::poll(GLFWwindow *window) {
	glfwPollEvents();

	// // manually poll certain keys (cursed)
	// KeyInfo *keys = keyInfo.get();

	// (&keys[GLFW_MOUSE_BUTTON_LEFT])->newAction(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT));
}

bool InputHandler::clicked(uint32_t keyid) {
	const KeyInfo *keys = keyInfo.get();
	const KeyInfo *key = &keys[keyid];
	return (key->action == GLFW_PRESS);
}

const KeyInfo *InputHandler::get(uint32_t keyid) {
	const KeyInfo *keys = keyInfo.get();
	return &keys[keyid];
}

GLfloat InputHandler::getXScroll() {
	return static_cast<GLfloat>(this->scroll_x);
}

GLfloat InputHandler::getYScroll() {
	return static_cast<GLfloat>(this->scroll_y);
}
