#include "WindowManager.hpp"
#include "Crash.hpp"
#include "Settings.hpp"

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void glfw_resizeViewport_callback(GLFWwindow* window, int windowWidth, int windowHeight) {
	Client *client = reinterpret_cast<Client *>(glfwGetWindowUserPointer(window));
	client->resizeViewport(windowWidth, windowHeight);
}

void WindowManager::fovCallback(void *_windowmanager, const void *_data) {
	WindowManager *wm = reinterpret_cast<WindowManager *>(_windowmanager);
	GLfloat fov = *reinterpret_cast<const GLfloat *>(_data);
	wm->updateProjection(fov);
}

void WindowManager::updateProjection(GLfloat fov) {
    this->projection = glm::perspective(glm::radians(fov), static_cast<GLfloat>(this->aspectRatio), static_cast<GLfloat>(Settings::znear), static_cast<GLfloat>(Settings::zfar));
}

void WindowManager::resizeViewport(int windowWidth, int windowHeight) {
    // Compute window's ration
    this->aspectRatio = static_cast<GLfloat>(windowWidth) / static_cast<GLfloat>(windowHeight);

    // Set perspective
	updateProjection(Settings::fov);

    // Set viewport to be the entire window
    glViewport(0, 0, windowWidth, windowHeight);

	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;

    // printf("window set to %d %d. half is %d %d\n", windowWidth, windowHeight, windowWidth / 2, windowHeight / 2);
}

void glfw_handleKey_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	Client *client = reinterpret_cast<Client *>(glfwGetWindowUserPointer(window));

	client->pressKey(window, key, scancode, action, mods);
}

void glfw_handleMouseMov_callback(GLFWwindow *window, double xpos, double ypos) {
	Client *client = reinterpret_cast<Client *>(glfwGetWindowUserPointer(window));

	if (!client->resize) {
		client->moveMouseTo(xpos, ypos);
	} else {
		glfwGetCursorPos(window, &xpos, &ypos);
		client->resize = false;
		client->centerMouseTo(xpos, ypos);
	}
}

void glfw_handleMouseKey_callback(GLFWwindow* window, int button, int action, int mods) {
	Client *client = reinterpret_cast<Client *>(glfwGetWindowUserPointer(window));

	client->pressMouseKey(window, button, action, mods);
}

void glfw_handleFileDrop_callback(GLFWwindow* window, int count, const char** paths) {
	int i;
	for (i = 0; i < count; i++) {
		printf("got filename: %s\n", paths[i]);
	}
}

void glfw_handleScroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	Client *client = reinterpret_cast<Client *>(glfwGetWindowUserPointer(window));

	client->scroll(xoffset, yoffset);
}

WindowManager::WindowManager(int windowWidth, int windowHeight, Client *client)
: windowWidth(windowWidth), windowHeight(windowHeight)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        perror("GLFW window failed to initiate");
    }

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 460";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only isto faz o que??????????????????????????????????????????????????????????????????????????????????????????????????????

    this->window = glfwCreateWindow(windowWidth, windowHeight, "MarchingCubes", NULL, NULL);

	CRASH_IF(window == nullptr, "GLFW window failed to create"); // call glfwTerminate?????
    glfwMakeContextCurrent(window);

	CRASH_IF(glewInit() != GLEW_OK, "GLEW failed");

	glfwSetWindowUserPointer(window, client);

	// vsync
	glfwSwapInterval(0);

    ///////////////////////// CALLBAKCS
    glfwSetFramebufferSizeCallback(window, glfw_resizeViewport_callback);
    glfwSetKeyCallback(window, glfw_handleKey_callback);
    glfwSetCursorPosCallback(window, glfw_handleMouseMov_callback);
	glfwSetMouseButtonCallback(window, glfw_handleMouseKey_callback);
	glfwSetDropCallback(window, glfw_handleFileDrop_callback);
	glfwSetScrollCallback(window, glfw_handleScroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    /* OpenGL settings */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	std::cout << glGetString(GL_VENDOR) << std::endl;
	std::cout << glGetString(GL_RENDERER) << std::endl;
	std::cout << glGetString(GL_VERSION) << std::endl;

	// During init, enable debug output

#if not defined(DISTRIBUTION)
	#if not (defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__))
		glEnable( GL_DEBUG_OUTPUT );
		glDebugMessageCallback( openglCallbackFunction, NULL );
	#endif
#endif

    // IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // During init, enable debug output
    // glEnable( GL_DEBUG_OUTPUT );
    // glDebugMessageCallback( openglCallbackFunction, NULL );

    // MSAA
    // glEnable(GL_MULTISAMPLE);
}

WindowManager::~WindowManager() {
	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}
