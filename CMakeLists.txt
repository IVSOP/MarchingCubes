cmake_minimum_required(VERSION 3.22.1)
project(MarchingCubes)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) #???????

set(CMAKE_CXX_STANDARD_REQUIRED true)

# TODO see link options like -Wl,--as-needed -Wl,-Bdynamic

set(LIB_OUTPUT_DIR "${CMAKE_BINARY_DIR}/lib")
file(MAKE_DIRECTORY ${LIB_OUTPUT_DIR})

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
		src/Image.cpp
		src/ALErrors.cpp
		src/Audio.cpp
		src/embree.cpp
		src/Cubemap.cpp

		external/imgui/imgui.cpp
		external/imgui/imgui_demo.cpp
		external/imgui/imgui_draw.cpp
		external/imgui/imgui_tables.cpp
		external/imgui/imgui_widgets.cpp
		external/imgui/backends/imgui_impl_glfw.cpp
		external/imgui/backends/imgui_impl_opengl3.cpp
)

# set_target_properties(MarchingCubes PROPERTIES
#     BUILD_WITH_INSTALL_RPATH TRUE    # Use build RPATH
#     INSTALL_RPATH "$ORIGIN/../lib"   # Set RPATH for installed executable
#     BUILD_RPATH "$ORIGIN/../lib"      # Set RPATH for built executable
#     SKIP_BUILD_RPATH FALSE            # Do not skip building RPATH
# )

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
	target_compile_definitions(MarchingCubes
		PRIVATE DISTRIBUTION=true
	)
else (Distribution)
	set(TRACY_ENABLE on)
	set(TRACY_ONLY_LOCALHOST on)
	set(TRACY_NO_CRASH_HANDLER on)
endif (Distribution)

# disable rtti and exceptions globally (you can ignore the warnings that come from this)
# add_compile_options(-fno-rtti) # I REALLY wanted to disable this shit, but assimp needs it, and using it only in assimp will cause linking issues with jolt
# so ill have to enable it
add_compile_options(-frtti)
add_compile_options(-fno-exceptions)

if (WIN32)

	target_compile_options(MarchingCubes PRIVATE "$<$<CONFIG:RELEASE>:-O3>")

	cmake_policy(SET CMP0072 NEW) # ???????????????????????????????????????

	if (Distribution)
		set(IPO off)
		message(WARNING "IPO cannot be used with mingw, not using it")
	endif (Distribution)
else (WIN32)
	target_compile_options(MarchingCubes PRIVATE "$<$<CONFIG:DEBUG>:-O0;-g3>") # ${CMAKE_CXX_FLAGS_DEBUG} ${GCC_COVERAGE_COMPILE_FLAGS}
	target_compile_options(MarchingCubes PRIVATE "$<$<CONFIG:RELEASE>:-O3;-Wall;-Wextra;-pedantic;-Wno-unused-parameter>") # ${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}
	target_compile_options(MarchingCubes PRIVATE "$<$<CONFIG:RELWITHDEBINFO>:-O3;-Wall;-Wextra;-pedantic;-Wno-unused-parameter>") # ${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}
	# MinSizeRel

	if (Distribution)
		set(IPO on)
	else (Distribution)
		set(IPO off)
	endif (Distribution)

endif(WIN32)

if (IPO)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT result)
	if (result)
		message(STATUS "Using IPO")
		# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON) # this sets it for ALL libraries and inlines them. this is illegal for some of them
		set_property(TARGET MarchingCubes PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
		# TODO add more targets???
		# set_property(TARGET foo PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
	else(result)
		message(FATAL_ERROR "IPO not supported :)")
	endif(result)
endif (IPO)

set_property(GLOBAL PROPERTY USE_FOLDERS ON) # ???????

find_package(OpenGL REQUIRED)

if (WIN32)
	include_directories(external/glew/include)

	if (NOT EXISTS ${LIB_OUTPUT_DIR}/glew32.dll)
		file(COPY external/glew/bin/Release/x64/glew32.dll DESTINATION ${LIB_OUTPUT_DIR})
	endif()

	if (NOT EXISTS ${LIB_OUTPUT_DIR}/steam_api64.dll)
		file(COPY external/steamworks/sdk/redistributable_bin/win64/steam_api64.dll DESTINATION ${LIB_OUTPUT_DIR})
	endif()

	# if (NOT EXISTS ${LIB_OUTPUT_DIR}/phonon.dll)
	# 	file(COPY external/steamaudio/lib/windows-x64/phonon.dll DESTINATION ${LIB_OUTPUT_DIR})
	# endif()

	# if (NOT EXISTS ${LIB_OUTPUT_DIR}/assimp.dll)
	# 	file(COPY external/assimp_windows/bin/assimp.dll DESTINATION ${LIB_OUTPUT_DIR})
	# endif()

	include_directories(external/zstd-v1.5.6-win64/include/)

	if (NOT EXISTS ${LIB_OUTPUT_DIR}/OpenAL32.dll)
		file(COPY external/openal-soft/win64/OpenAL32.dll DESTINATION ${LIB_OUTPUT_DIR})
	endif()
	include_directories(external/openal-soft/win64/include)

	if (NOT EXISTS ${LIB_OUTPUT_DIR}/sndfile.dll)
		file(COPY external/libsndfile/win64/sndfile.dll DESTINATION ${LIB_OUTPUT_DIR})
	endif()
	include_directories(external/libsndfile/win64/include)

	if (NOT EXISTS ${LIB_OUTPUT_DIR}/embree4.dll)
		file(COPY external/embree/win64/bin/embree4.dll DESTINATION ${LIB_OUTPUT_DIR})
	endif()
	include_directories(external/embree/win64/include)
	if (NOT EXISTS ${LIB_OUTPUT_DIR}/tbb12.dll)
		file(COPY external/embree/win64/bin/tbb12.dll DESTINATION ${LIB_OUTPUT_DIR})
	endif()
	include_directories(external/embree/win64/include)


	# lib files are used at compile time only, so I don't copy them and link them directly
	target_link_directories(MarchingCubes PRIVATE
		external/glew/lib/Release/x64/
		external/steamworks/sdk/redistributable_bin/win64/
		# external/steamaudio/lib/windows-x64/
		external/zstd-v1.5.6-win64/static/
		external/openal-soft/win64/
		external/libsndfile/win64/
		external/embree/win64/lib/
		${LIB_OUTPUT_DIR}/
	)

else (WIN32) # linux
	# I got some issues where people didn't have a high enough version of glew so I'll just link it statically for now
	# on arch linux that won't be available, so link manually
	# I make it link to a certain file manually, and also have to change the name that gets linked
	set(GLEW_USE_STATIC_LIBS ON)
	find_package(GLEW)
	if(NOT GLEW_FOUND)
		message(STATUS "static glew not found, using local copy")
		set(GLEW_LOCAL_LIB external/glew/lib/linux/x64/)
		set(GLEW_LIB libGLEW.a)
	else()
		set(GLEW_LIB GLEW::GLEW)
	endif()

	find_package(zstd REQUIRED)

	if (NOT EXISTS ${LIB_OUTPUT_DIR}/libsteam_api.so)
		file(COPY external/steamworks/sdk/redistributable_bin/linux64/libsteam_api.so DESTINATION ${LIB_OUTPUT_DIR})
	endif()

	# if (NOT EXISTS ${LIB_OUTPUT_DIR}/libphonon.so)
	# 	file(COPY external/steamaudio/lib/linux-x64/libphonon.so DESTINATION ${LIB_OUTPUT_DIR})
	# endif()

	# if (NOT EXISTS ${LIB_OUTPUT_DIR}/libassimp.so)
	# 	file(COPY external/assimp/build/bin/libassimp.so DESTINATION ${LIB_OUTPUT_DIR} FOLLOW_SYMLINK_CHAIN)
	# endif()

	if (NOT EXISTS ${LIB_OUTPUT_DIR}/libopenal.so)
		file(COPY external/openal-soft/linux64/libopenal.so DESTINATION ${LIB_OUTPUT_DIR})
	endif()
	include_directories(external/openal-soft/linux64/include)

	if (NOT EXISTS ${LIB_OUTPUT_DIR}/libsndfile.so)
		file(COPY external/libsndfile/linux64/libsndfile.so DESTINATION ${LIB_OUTPUT_DIR})
	endif()
	include_directories(external/libsndfile/linux64/include)

	if (NOT EXISTS ${LIB_OUTPUT_DIR}/libembree4.so)
		file(COPY external/embree/linux64/lib/libembree4.so DESTINATION ${LIB_OUTPUT_DIR} FOLLOW_SYMLINK_CHAIN)
	endif()
	include_directories(external/embree/linux64/include)

	target_link_directories(MarchingCubes PRIVATE
		${LIB_OUTPUT_DIR}/
		${GLEW_LOCAL_LIB}
	)

endif (WIN32)

add_subdirectory(external/glm)
include_directories(external/glm)

add_subdirectory(external/glfw)
include_directories(external/glfw/include)

include_directories(external/imgui)
include_directories(external/imgui/backends)

include_directories(external/stb)

add_subdirectory(external/entt)
include_directories(external/entt/single_include/entt)

set(JPH_DEBUG_RENDERER ON)
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
# assimp uses exceptions for some reason, so use them here specifically
target_compile_options(assimp PRIVATE -fexceptions -Wno-error)
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
		# phonon
		libzstd_static
		OpenAL32
		sndfile
		embree4
		winmm
		# ${Boost_LIBRARIES}
	)

else (WIN32) # linux
	target_link_libraries(MarchingCubes
		glfw
		# GLEW::GLEW
		${GLEW_LIB}
		OpenGL::GL
		EnTT::EnTT
		Jolt
		steam_api
		Tracy::TracyClient
		assimp
		nlohmann_json::nlohmann_json
		# phonon
		zstd
		openal
		sndfile
		embree4
		# ${Boost_LIBRARIES}
	)

endif (WIN32)
# ?????? o glm funciona por magia sem linking nenhum????? e usar so link_libraries nao funcionou, isto ja me ultrapassa
