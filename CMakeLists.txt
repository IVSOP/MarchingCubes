cmake_minimum_required(VERSION 3.22.1)
project(MarchingCubes)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) #???????

set(CMAKE_CXX_STANDARD_REQUIRED true)

# TODO see link options like -Wl,--as-needed -Wl,-Bdynamic

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
		src/Assets.cpp
		src/Files.cpp
		src/PhysRenderer.cpp
		src/Settings.cpp
		src/Archive.cpp

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

set(ASSIMP_BUILD_TESTS off)
set(ASSIMP_NO_EXPORT on)
set(ASSIMP_INSTALL off)
set(ASSIMP_INJECT_DEBUG_POSTFIX off)
set(ASSIMP_INSTALL_PDB off)
set(ASSIMP_BUILD_ASSIMP_VIEW off)
set(GLFW_BUILD_DOCS off)
set(ENTT_NOEXCEPTION) # set on??
# 	set(ENTT_DISABLE_ASSERT on)

if (Distribution)
	set(TRACY_ENABLE off)
	set(TRACY_ONLY_LOCALHOST off)
	set(TRACY_NO_CRASH_HANDLER off)
else (Distribution)
	set(TRACY_ENABLE on)
	set(TRACY_ONLY_LOCALHOST on)
	set(TRACY_NO_CRASH_HANDLER on)
endif (Distribution)

if (WIN32)

	target_compile_options(MarchingCubes PRIVATE "$<$<CONFIG:RELEASE>:-O2>")

	cmake_policy(SET CMP0072 NEW) # ???????????????????????????????????????

	message(WARNING "IPO cannot be used with mingw, not using it")
	set(IPO off)

else (WIN32)
	target_compile_options(MarchingCubes PRIVATE "$<$<CONFIG:DEBUG>:-O0;-g3>") # ${CMAKE_CXX_FLAGS_DEBUG} ${GCC_COVERAGE_COMPILE_FLAGS}
	target_compile_options(MarchingCubes PRIVATE "$<$<CONFIG:RELEASE>:-O2;-Wall;-Wextra;-pedantic;-Wno-unused-parameter>") # ${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}
	target_compile_options(MarchingCubes PRIVATE "$<$<CONFIG:RELWITHDEBINFO>:-O2;-Wall;-Wextra;-pedantic;-Wno-unused-parameter>") # ${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}
	# MinSizeRel

	set(IPO on)

endif(WIN32)

if (IPO)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT result)
	if (result)
		message(STATUS "Using IPO")
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
		# set_property(TARGET foo PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
	else(result)
		message(FATAL_ERROR "IPO not supported :)")
	endif(result)
endif (IPO)

set_property(GLOBAL PROPERTY USE_FOLDERS ON) # ???????

find_package(OpenGL REQUIRED)

if (WIN32)
	include_directories(external/glew/include)

	if (NOT EXISTS ${CMAKE_BINARY_DIR}/glew32.dll)
		file(COPY external/glew/bin/Release/x64/glew32.dll DESTINATION ${CMAKE_BINARY_DIR})
	endif()

	if (NOT EXISTS ${CMAKE_BINARY_DIR}/steam_api64.dll)
		file(COPY external/steamworks/sdk/redistributable_bin/win64/steam_api64.dll DESTINATION ${CMAKE_BINARY_DIR})
	endif()

	if (NOT EXISTS ${CMAKE_BINARY_DIR}/phonon.dll)
		file(COPY external/steamaudio/lib/windows-x64/phonon.dll DESTINATION ${CMAKE_BINARY_DIR})
	endif()

	# if (NOT EXISTS ${CMAKE_BINARY_DIR}/assimp.dll)
	# 	file(COPY external/assimp_windows/bin/assimp.dll DESTINATION ${CMAKE_BINARY_DIR})
	# endif()

	include_directories(external/zstd-v1.5.6-win64/include/)

	# lib files are used at compile time only, so I don't copy them and link them directly
	target_link_directories(MarchingCubes PRIVATE
		external/glew/lib/Release/x64/
		# external/glew/bin/Release/x64/
		external/steamworks/sdk/redistributable_bin/win64/
		external/steamaudio/lib/windows-x64/
		# external/assimp_windows/lib/
		external/zstd-v1.5.6-win64/static/
		${CMAKE_BINARY_DIR}/
	)

else (WIN32) # linux
	find_package(GLEW REQUIRED)
	find_package(zstd REQUIRED)

	if (NOT EXISTS ${CMAKE_BINARY_DIR}/libsteam_api.so)
		file(COPY external/steamworks/sdk/redistributable_bin/linux64/libsteam_api.so DESTINATION ${CMAKE_BINARY_DIR})
	endif()

	if (NOT EXISTS ${CMAKE_BINARY_DIR}/libphonon.so)
		file(COPY external/steamaudio/lib/linux-x64/libphonon.so DESTINATION ${CMAKE_BINARY_DIR})
	endif()

	# if (NOT EXISTS ${CMAKE_BINARY_DIR}/libassimp.so)
	# 	file(COPY external/assimp/build/bin/libassimp.so DESTINATION ${CMAKE_BINARY_DIR} FOLLOW_SYMLINK_CHAIN)
	# endif()

	target_link_directories(MarchingCubes PRIVATE
		${CMAKE_BINARY_DIR}/
	)

endif (WIN32)

set(JPH_DEBUG_RENDERER ON)

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

include_directories(external/steamaudio/include/)

# set(BUILD_SHARED_LIBS ON)
# TODO if doing this as shared lib, copy the .so/.dll to build/buildWin
add_subdirectory(external/assimp)
# set(BUILD_SHARED_LIBS OFF)

#include_directories(external/assimp/code)
include_directories(external/assimp/include/assimp)

set(JSON_BuildTests OFF CACHE INTERNAL "")
# find_package(nlohmann_json 3.2.0 REQUIRED)
add_subdirectory(external/json)

# cmake_policy(SET CMP0167 NEW) # why the fuck did cmake include something specific for the boost lib, what the actual fuck
# find_package(Boost REQUIRED COMPONENTS serialization)
# include_directories(${Boost_INCLUDE_DIRS})

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
		nlohmann_json::nlohmann_json
		phonon
		libzstd_static
		# ${Boost_LIBRARIES}
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
		nlohmann_json::nlohmann_json
		phonon
		zstd
		# ${Boost_LIBRARIES}
	)

endif (WIN32)
# ?????? o glm funciona por magia sem linking nenhum????? e usar so link_libraries nao funcionou, isto ja me ultrapassa
