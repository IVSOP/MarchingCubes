#include "Client.hpp"

#include "LookupTable.hpp"
#include "Looper.hpp"
#include "Audio.hpp"
#include "Assets.hpp"
#include "Settings.hpp"

#define PLAYER_POS Position(glm::vec3(64, 16, 64))
#define PLAYER_LOOKAT glm::vec3(0, 0, -1)

Client::Client(PhysRenderer *phys_renderer)
: windowManager(std::make_unique<WindowManager>(1920, 1080, this)),
  world(std::make_unique<World>()),
  player(std::make_unique<Player>()),
  renderer(std::make_unique<Renderer>(1920, 1080, phys_renderer)), // get these from window manager???
  inputHandler(glfw_handleMouseMov_callback, glfw_handleMouseKey_callback) // funcs from window manager
{

	resizeViewport(1920, 1080); // these too

	// Looper looper = Looper<4000>();
	// auto func = [this]() {
    //     printf("hello\n");
    // };

	// std::thread t([&]() { looper.loop(func); });
	// t.detach();

	// std::thread(looper.loop(func));


	// Chunk chunk;
	// for (GLuint y = 0; y < CHUNK_SIZE; y ++) {
	// 	for (GLuint z = 0; z < CHUNK_SIZE; z ++) {
	// 		for (GLuint x = 0; x < CHUNK_SIZE; x ++) {
	// 			Voxel voxel = Voxel(sphere_getValue(x, y, z), 0); // REMAKE THIS to add spheres to world and not just to the chunk
	// 			// if (x == 5) voxel.material_id = 1;
	// 			chunk.insertVoxelAt(glm::uvec3(x, y, z), voxel);
	// 		}
	// 	}
	// }


	// for (GLuint x = 0; x < WORLD_SIZE_X; x++) {
	// 	for (GLuint y = 0; y < WORLD_SIZE_Y; y++) {
	// 		for (GLuint z = 0; z < WORLD_SIZE_Z; z++) {
	// 			// world.get()->copyChunkTo(chunk, glm::uvec3(x, 0, z));
	// 			// world.get()->copyChunkTo(chunk, glm::uvec3(x, 15, z));
	// 			// world.get()->copyChunkTo(chunk1, glm::uvec3(x, y, z));
	// 			world.get()->copyChunkTo(chunk, glm::uvec3(x + 1, y, z + 1));
	// 			return;
	// 		}
	// 	}
	// }

	// constexpr glm::vec3 center1(0.0f);
	// constexpr GLfloat radius1 = 8.0f;
	// constexpr glm::vec3 center2(16.0f, 8.0f, 16.0f);
	// constexpr GLfloat radius2 = 10.5f;
	// world.get()->addSphere(center1, radius1);
	// world.get()->addSphere(center2, radius2);

	// world.get()->loadHeightMap("textures/iceland_heightmap.png");
	// world.get()->loadHeightMap("textures/terreno.jpg");
	// world.get()->loadHeightMap("textures/Ridge Through Terrain Height Map.png");
	// world.get()->loadHeightMap("textures/Height Map.png");


	// flat terrain for testing
	// JPH::TriangleList flat_terrain;

	// JPH::Float3 v1, v2, v3;

	// const glm::vec3 verts[] = {
	// 	glm::vec3(1.0f, 0.0f, 1.0f) * 1000.0f,
	// 	glm::vec3(1.0f, 0.0f, -1.0f) * 1000.0f,
	// 	glm::vec3(-1.0f, 0.0f, -1.0f) * 1000.0f,

	// 	glm::vec3(-1.0f,0.0f,  -1.0f) * 1000.0f,
	// 	glm::vec3(-1.0f,0.0f,  1.0f) * 1000.0f,
	// 	glm::vec3(1.0f, 0.0f, 1.0f) * 1000.0f,
	// };


	// v1.x = verts[0].x; v1.y = verts[0].y; v1.z = verts[0].z;
	// v2.x = verts[1].x; v2.y = verts[1].y; v2.z = verts[1].z;
	// v3.x = verts[2].x; v3.y = verts[2].y; v3.z = verts[2].z;
	// JPH::Triangle a(v1, v2, v3);
	// v1.x = verts[3].x; v1.y = verts[3].y; v1.z = verts[3].z;
	// v2.x = verts[4].x; v2.y = verts[4].y; v2.z = verts[4].z;
	// v3.x = verts[5].x; v3.y = verts[5].y; v3.z = verts[5].z;
	// JPH::Triangle b(v1, v2, v3);
	// flat_terrain.push_back(a);
	// flat_terrain.push_back(b);

	// Phys::loadTerrain(world->getPhysTerrain());
	
	
		world.get()->loadHeightMap("textures/Rolling Hills Height Map.png");


	// FileHandler savefile = FileHandler(Settings::saves_dir + "1.bin", FileModes::Read | FileModes::Bin);
	// world = std::make_unique<World>(savefile);


	player->setupPhys(PLAYER_POS, PLAYER_LOOKAT);
	Frustum frustum = Frustum(
		player->getPos().pos, player->getRotation(), player->getUpVector(),
		static_cast<GLfloat>(Settings::fov), windowManager->aspectRatio, static_cast<GLfloat>(Settings::znear), static_cast<GLfloat>(Settings::zfar)
	);
	world->buildData(frustum);
}

void Client::resizeViewport(int windowWidth, int windowHeight) {
	if (windowWidth == 0 || windowHeight == 0) {
        fprintf(stderr, "Detected window size 0, ignoring resize operation\n");
    } else {
		windowManager.get()->resizeViewport(windowWidth, windowHeight);
		renderer.get()->resizeViewport(windowWidth, windowHeight);

		this->resize = true;
	}
}

void Client::pressKey(GLFWwindow *window, int key, int scancode, int action, int mods) {
    inputHandler.pressKey(window, key, scancode, action, mods);
}

void Client::moveMouseTo(double xpos, double ypos) {
	inputHandler.moveMouseTo(xpos, ypos);
}

void Client::centerMouseTo(double xpos, double ypos) {
	inputHandler.centerMouseTo(xpos, ypos);
}

void Client::pressMouseKey(GLFWwindow* window, int button, int action, int mods) {
	inputHandler.pressMouseKey(window, button, action, mods);
}

// TODO order of input processing, rendering and phys updating probaby makes things feel off by 1 frame, consider making phys update right after processing inputs
void Client::mainloop() {

	// // uint32_t idmagujo = world->loadModel("magujo/magujo.glb", "magujo/magujo-hitbox.json");
	// uint32_t idmagujo = 0;
	// // world->spawn(idmagujo, JPH::Vec3::sZero(), JPH::Quat::sIdentity());
	// entt::entity magujoentity = world->spawn(idmagujo, JPH::Vec3(0.0f, 50.0f, 0.0f), JPH::Quat::sIdentity());

	// uint32_t idmagujonpc = 1;
	// (void)world->spawnCharacter(idmagujonpc, JPH::Vec3::sZero(), JPH::Quat::sIdentity());

	// AudioComponent &audio = world->entt_registry.emplace<AudioComponent>(magujoentity, "crazy_frog_mono.wav");
	// audio.setGain(10.0f);
	// audio.play();

	for (int i = 0; i < 100; i++) {
		entt::entity ball = world->spawn(2, JPH::Vec3(0.0f, 0.0f + (i * 5.0f), 0.0f), JPH::Quat::sIdentity());
		// AudioComponent &audio = world->entt_registry.emplace<AudioComponent>(ball, "crazy_frog_mono.wav");
		// audio.setGain(1.0f);
		// audio.play();
	}

	// uint32_t idsphere_mc = 0;
	// world->spawnMarchingCubes(idsphere_mc, glm::ivec3(0, -40, 0));

	// // uint32_t idlivingroom = world->loadModel("livingroom/InteriorTest.fbx");
	// // world->spawn(idlivingroom, JPH::Vec3(0.0f, 50.0f, 0.0f), JPH::Quat::sIdentity());

	// // uint32_t idsenna = world->loadModel("senna/senna.fbx");
	// // world->spawn(idsenna, JPH::Vec3(0.0f, 100.0f, 0.0f), JPH::Quat::sIdentity());

	// // uint32_t idcottage = world->loadModel("cottage/cottage_fbx.fbx");
	// // world->spawn(idcottage, JPH::Vec3(0.0f, 100.0f, 0.0f), JPH::Quat::sIdentity());

	// // uint32_t idearth = world->loadModel("earth/earth.fbx");
	// // world->spawn(idearth, JPH::Vec3(0.0f, 100.0f, 0.0f), JPH::Quat::sIdentity());

	// // uint32_t iddragon = world->loadModel("dragon/dragon.glb");
	// // world->spawn(iddragon, JPH::Vec3(0.0f, 100.0f, 0.0f), JPH::Quat::sIdentity());

	// // uint32_t idbuilding = world->loadModel("buildings/fbx/Residential Buildings 001.fbx");
	// // world->spawn(idbuilding, JPH::Vec3(0.0f, 100.0f, 0.0f), JPH::Quat::sIdentity());

	// {
	// 	FileHandler savefile(Settings::saves_dir + "1.bin", FileModes::Write | FileModes::Bin | FileModes::Trunc);
	// 	world->save(savefile);
	// }
	// exit(1);

    double lastFrameTime, currentFrameTime, deltaTime = PHYS_STEP; // to prevent errors when this is first ran, I initialize it to the physics substep
    while (!glfwWindowShouldClose(windowManager.get()->window)) {
        glfwPollEvents(); // at the start due to imgui (??) test moving it to after the unlock()

		lastFrameTime = glfwGetTime();

        // printf("delta is %f (%f fps)\n", deltaTime, 1.0f / deltaTime);
		// TODO sometimes I use this data, other times I call functions from Player which gets them again, what a mess
		Position  pos = player->getPos();
		Direction dir = player->getDir();
		Movement  mov = player->getMov();

		SelectedBlockInfo selectedBlock = world.get()->getSelectedBlock(pos.pos, dir.front, Settings::break_range);
        inputHandler.applyInputs(
			world.get(),
			selectedBlock,
			Settings::break_radius,
			player.get(), windowManager->windowWidth, windowManager->windowHeight, static_cast<GLfloat>(deltaTime));

		// ray cast to select entities
		glm::vec3 raydir = dir.front * Settings::raycast_len;
		// to prevent from colliding with the body of the player itself I did this, will change in the future
		glm::vec3 rayorigin = pos.pos + (dir.front * 3.5f);
		JPH::BodyID lookatbody = Phys::raycastBody(JPH::Vec3(rayorigin.x, rayorigin.y, rayorigin.z), JPH::Vec3(raydir.x, raydir.y, raydir.z));
		entt::entity selected_entity = Phys::getUserData(lookatbody).getEntity();
		world->entt_registry.emplace<Selected>(selected_entity);



		const int collisionSteps = 1;
		Phys::update(deltaTime, collisionSteps);

		player->postTick();


        // std::unique_lock<std::mutex> lock = std::unique_lock<std::mutex>(mtx);
        // renderer.get()->draw(draw_quads, projection, *camera.get(), window, deltaTime);
		Frustum frustum = Frustum(
			player->getPos().pos, player->getRotation(), player->getUpVector(),
			static_cast<GLfloat>(Settings::fov), windowManager->aspectRatio, static_cast<GLfloat>(Settings::znear), static_cast<GLfloat>(Settings::zfar)
		);
    	world->buildData(frustum);

		glm::mat4 view = player->getViewMatrix();
		renderer->draw(
			view,
			world->getVerts(),
			world->getPoints(),
			world->getIndirect(),
			world->getInfo(),
			world->getEntitiesToDraw(frustum),
			world->getSelectedEntities(),
			windowManager->projection,
			windowManager->window, deltaTime,
			pos,
			dir,
			mov,
			selectedBlock);
		
        // lock.unlock();


		// 	phys.applyToPosition(phys_pos_view.get<Position>(entity).pos, static_cast<GLfloat>(deltaTime));
		// }

        currentFrameTime = glfwGetTime();
        deltaTime = currentFrameTime - lastFrameTime;

		// if (renderer.get()->limitFPS) {
		// 	const double fps_time = 1.0f / renderer.get()->fps;
		// 	if (deltaTime < fps_time) {
		// 		const double sleepTime = (fps_time - deltaTime) * 10E5; // multiply to get from seconds to microseconds, this is prob platform dependent and very bad
		// 		usleep(sleepTime);
		// 		deltaTime = fps_time;
		// 	}
		// }

		// set listener props
		Audio::ALContext::setListenerPosition(pos.pos);
		Audio::ALContext::setListenerVelocity(player->getVelocity());
		Audio::ALContext::setListenerOrientation(dir.front, dir.up);

		// set audio for all entities
		{
			JPH::Vec3 pos_jph;
			glm::vec3 pos;
			auto group = world->entt_registry.group<>(entt::get<AudioComponent, Physics>);
			for (const auto entity : group) {
				const AudioComponent &audio = group.get<AudioComponent>(entity);
				const Physics &phys = group.get<Physics>(entity);

				pos_jph = phys.getPosition();
				pos = glm::vec3(pos_jph.GetX(), pos_jph.GetY(), pos_jph.GetZ());

				audio.setPosition(pos);
			}
		}

    }
}

// void Client::saveWorldTo(const std::string &filepath) const {
// 	std::ofstream file(filepath, std::ios::binary);

// 	player.get()->saveTo(file);
// 	world.get()->saveTo(file);

// 	file.flush();
// 	file.close();
// }

// void Client::loadWorldFrom(const std::string &filepath) {
// 	std::ifstream file(filepath, std::ios::binary);

// 	// got lazy, maybe it is faster to iterate and change pre-existing world???
// 	player = std::make_unique<Player>(file);
// 	world = std::make_unique<World>(file);
// 	// inputHandler = InputHandler(glfw_handleMouseMov_callback, glfw_handleMouseKey_callback);
// }
