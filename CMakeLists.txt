cmake_minimum_required(VERSION 3.22.1)
project(MarchingCubes)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD_REQUIRED true)


set(TRACY_DEFINES "-DTRACY_ENABLE -D TRACY_ONLY_LOCALHOST -DTRACY_NO_CRASH_HANDLER") #-DTRACY_NO_BROADCAST

if (WIN32)

	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} ${TRACY_DEFINES} -O2")

	cmake_policy(SET CMP0072 NEW)

else (WIN32)
	set(CMAKE_CXX_FLAGS_DEBUG "${GCC_COVERAGE_COMPILE_FLAGS} ${TRACY_DEFINES} -Wno-deprecated ${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
	# I have no idea if _RELEASE is even doing anything here or if the vars after also should have it
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} ${TRACY_DEFINES} -Wno-deprecated -O2 -Wall -Wextra -pedantic -Wno-unused-parameter")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "{CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} ${TRACY_DEFINES} -Wno-deprecated -O2 -Wall -Wextra -pedantic -Wno-unused-parameter")
endif(WIN32)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(OpenGL REQUIRED)

if (WIN32)
	include_directories(external/glew/include)

	if (NOT EXISTS ${CMAKE_BINARY_DIR}/glew32.dll)
		file(COPY external/glew/bin/Release/x64/glew32.dll DESTINATION ${CMAKE_BINARY_DIR})
	endif()

	if (NOT EXISTS ${CMAKE_BINARY_DIR}/steam_api64.dll)
		file(COPY external/steamworks/sdk/redistributable_bin/win64/steam_api64.dll DESTINATION ${CMAKE_BINARY_DIR})
	endif()

	if (NOT EXISTS ${CMAKE_BINARY_DIR}/assimp.dll)
		file(COPY external/assimp_windows/bin/assimp.dll DESTINATION ${CMAKE_BINARY_DIR})
	endif()

	# lib files are used at compile time only, so I don't copy them and link them directly
	link_directories(
		external/glew/lib/Release/x64/
		# external/glew/bin/Release/x64/
		external/steamworks/sdk/redistributable_bin/win64/
		external/assimp_windows/lib/
		${CMAKE_BINARY_DIR}/
	)

else (WIN32) # linux
	find_package(GLEW REQUIRED)

	if (NOT EXISTS ${CMAKE_BINARY_DIR}/libsteam_api.so)
		file(COPY external/steamworks/sdk/redistributable_bin/linux64/libsteam_api.so DESTINATION ${CMAKE_BINARY_DIR})
	endif()

	if (NOT EXISTS ${CMAKE_BINARY_DIR}/libassimp.so)
		file(COPY external/assimp/build/bin/libassimp.so DESTINATION ${CMAKE_BINARY_DIR} FOLLOW_SYMLINK_CHAIN)
	endif()

	link_directories(
		${CMAKE_BINARY_DIR}/
	)

endif (WIN32)

# find_package(ZLIB)

add_subdirectory(external/glm)
include_directories(external/glm)

add_subdirectory(external/glfw)
include_directories(external/glfw/include)

include_directories(external/imgui)
include_directories(external/imgui/backends)

include_directories(external/stb)

add_subdirectory(external/entt)
include_directories(external/entt/single_include/entt)

add_subdirectory(external/JoltPhysics/Build)
include_directories(external/JoltPhysics/Jolt)
include(Jolt_Cmake.cmake)

add_subdirectory(external/tracy)
include_directories(external/tracy/public/tracy)

include_directories(external/steamworks/sdk/public/)

# assimp is built manually only for some reason (but on mingw it works, wtf)
# compile it with 'cmake -DASSIMP_BUILD_TESTS=off -B build; cmake --build build --parallel' or, for mingw, do 'cp ../../TC-mingw.cmake .; cmake -DASSIMP_BUILD_TESTS=off -B buildWin -DCMAKE_TOOLCHAIN_FILE=TC-mingw.cmake; cmake --build buildWin --parallel'
# !!!!! does not build on mingw, gave up trying to figure out why that is, so binaries are in assim_windows
# add_subdirectory(external/assimp)
include_directories(external/assimp/code)
include_directories(external/assimp/include)

add_executable(MarchingCubes
		src/main.cpp
		src/common.cpp
		src/GLErrors.cpp
		src/Shader.cpp
		src/TextureArray.cpp
		src/Client.cpp
		src/Player.cpp
		src/World.cpp
		src/Chunk.cpp
		src/Renderer.cpp
		src/WindowManager.cpp
		src/InputHandler.cpp
		src/Material.cpp
		src/LookupTable.cpp
		src/Logs.cpp
		src/Phys.cpp
		src/Crash.cpp

		external/imgui/imgui.cpp
		external/imgui/imgui_demo.cpp
		external/imgui/imgui_draw.cpp
		external/imgui/imgui_tables.cpp
		external/imgui/imgui_widgets.cpp
		external/imgui/backends/imgui_impl_glfw.cpp
		external/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(MarchingCubes
		PUBLIC include/
)

if (WIN32)
	target_link_libraries(MarchingCubes
		glfw
		${OPENGL_LIBRARIES}
		glew32
		OpenGL::GL
		EnTT::EnTT
		Jolt
		steam_api64
		Tracy::TracyClient
		assimp
	)

else (WIN32) # linux
	target_link_libraries(MarchingCubes
			glfw
			GLEW::GLEW
			OpenGL::GL
			EnTT::EnTT
			Jolt
			steam_api
			Tracy::TracyClient
			assimp
	)

endif (WIN32)
# ?????? o glm funciona por magia sem linking nenhum????? e usar so link_libraries nao funcionou, isto ja me ultrapassa
