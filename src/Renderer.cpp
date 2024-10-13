#include "Renderer.hpp"

#include <iostream>
#include <vector>
#include <cstdio>
#include "Vertex.hpp"
#include "Material.hpp"
#include "Camera.hpp"
#include "Crash.hpp"
#include "Logs.hpp"
#include "Profiling.hpp"
#include "Phys.hpp"
#include "Settings.hpp"

// TODO get this out of here too
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

// TODO manage texture slots somewhere
// I hate cpp enums so much, I can't I use enum class ... : int and have it convert to a fucking int
// TODO when should I use GL_TEXTURE0???????????????????????????????????????
struct TEXSLOTS {
	// TODO abstract this. GL_TEXTURE0 + slot is needed only when using glActiveTexture, but not when calling glBindTexture
	static constexpr GLint BASESLOT = GL_TEXTURE0;

	static constexpr GLint TEX_ARRAY_SLOT = 0;
	static constexpr GLint BRIGHT_TEXTURE_SLOT = 1;
	static constexpr GLint SCENE_TEXTURE_SLOT = 2;
	static constexpr GLint MATERIAL_TEXTURE_BUFFER_SLOT = 3;
	static constexpr GLint POINTLIGHT_TEXTURE_BUFFER_SLOT = 4;
	static constexpr GLint DIRLIGHT_TEXTURE_BUFFER_SLOT = 5;
	static constexpr GLint SPOTLIGHT_TEXTURE_BUFFER_SLOT = 6;
	static constexpr GLint CHUNKINFO_TEXTURE_BUFFER_SLOT = 7;
	static constexpr GLint MODELS_TRANSFORM_TEXTURE_BUFFER_SLOT = 8;
	static constexpr GLint MODELS_NORMALMAT_TEXTURE_BUFFER_SLOT = 9;
	static constexpr GLint MAXSLOT = 10;
};

#define MAX_MATERIALS 8
#define MAX_LIGHTS 8

struct PointLight {
    glm::vec3 position;

    GLfloat constant;
    GLfloat linear;
    GLfloat quadratic; 

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

	// need one extra float for 16. read Material.h
	GLfloat padding_1;
};
static_assert(sizeof(PointLight) == 4 * sizeof(glm::vec4), "Error: PointLight has unexpected size");

struct DirLight {
    glm::vec3 direction;
  
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};
static_assert(sizeof(DirLight) == 3 * sizeof(glm::vec4), "Error: DirLight has unexpected size");

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    GLfloat cutOff;
    GLfloat outerCutOff;
  
    GLfloat constant;
    GLfloat linear;
    GLfloat quadratic;
  
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};
static_assert(sizeof(SpotLight) == 5 * sizeof(glm::vec4), "Error: SpotLight has unexpected size");


// quad filling entire screen
const ViewportVertex viewportVertices[] = {
	{-1.0f, -1.0f, 0.0f, 0.0f, 0.0f},
	{1.0f, -1.0f, 0.0f, 1.0f, 0.0f},
	{1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
	{1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
	{-1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
	{-1.0f, -1.0f, 0.0f, 0.0f, 0.0f}
};

void Renderer::drawAxis(const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &projection) {
	const AxisVertex vertices[] = {
		// x
		{-1000.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
		{1000.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},

		// y
		{0.0f, -1000.0f, 0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 1000.0f, 0.0f, 0.0f, 1.0f, 0.0f},

		// z
		{0.0f, 0.0f, -1000.0f, 0.0f, 0.0f, 1.0f},
		{0.0f, 0.0f, 1000.0f, 0.0f, 0.0f, 1.0f}
	};

	// bind VAO, VBO
	GLCall(glBindVertexArray(this->VAO_axis));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer_axis));

	// load vertices
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

	axisShader.use();
	axisShader.setMat4("u_MVP", projection * view * model);

	GLCall(glDrawArrays(GL_LINES, 0, 6)); // 6 pontos, 3 linhas
}

Renderer::Renderer(GLsizei viewport_width, GLsizei viewport_height, PhysRenderer *phys_renderer)
: viewport_width(viewport_width), viewport_height(viewport_height), VAO(0), VBO(0),
  mainShader("shaders/lighting.vert", "shaders/lighting.frag"),
  axisShader("shaders/axis.vert", "shaders/axis.frag"),
  normalShader("shaders/normals.vert", "shaders/normals.frag", "shaders/normals.gs"),
  blurShader("shaders/blur.vert", "shaders/blur.frag"),
  hdrBbloomMergeShader("shaders/hdrBloomMerge.vert", "shaders/hdrBloomMerge.frag"),
  pointshader("shaders/points.vert", "shaders/points.frag"),
  modelShader("shaders/lighting_models.vert", "shaders/lighting_models.frag"),
  selectedModelShader("shaders/selected_models.vert", "shaders/selected_models.frag"),
  modelNormalShader("shaders/model_normals.vert", "shaders/model_normals.frag", "shaders/model_normals.gs"),
  outlineShader("shaders/outline.vert", "shaders/outline.frag"),
  insertShader("shaders/insert.vert", "shaders/insert.frag"),
  phys_renderer(phys_renderer)
{
	// TODO make a workaround for this
	CRASH_IF(getMaxTextureUnits() < TEXSLOTS::MAXSLOT, "Not enough texture slots");
	GLint maxTextureUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);

	//////////////////////////// LOADING VAO ////////////////////////////
	GLCall(glGenVertexArrays(1, &this->VAO));
	GLCall(glBindVertexArray(this->VAO));

	//////////////////////////// LOADING VBOS ////////////////////////////////

	// ?????????????? this VBO is never even used TODO wtf is this
	GLCall(glGenBuffers(1, &this->VBO_base));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->VBO_base));
	{
		GLuint vertex_id_layout = 0;
		GLCall(glEnableVertexAttribArray(vertex_id_layout));
		GLCall(glVertexAttribIPointer(vertex_id_layout, 1, GL_INT, sizeof(GLint), (const void *)0));
		GLCall(glVertexAttribDivisor(vertex_id_layout, 0)); // repeat every instance

		GLint ids[] = {
			0, 1, 2
		};

		GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(ids), ids, GL_STATIC_DRAW));
	}

	GLCall(glGenBuffers(1, &this->VBO));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->VBO));
	{
		GLuint data_layout = 1;
		GLCall(glEnableVertexAttribArray(data_layout));
		GLCall(glVertexAttribIPointer(data_layout, 1, GL_INT, sizeof(Vertex), (const void *)offsetof(Vertex, data)));
		GLCall(glVertexAttribDivisor(data_layout, 1)); // advance every instance
	}

	//////////////////////////// LOADING VAO FOR AXIS ////////////////////////////
	GLCall(glGenVertexArrays(1, &this->VAO_axis));
	GLCall(glBindVertexArray(this->VAO_axis));

	//////////////////////////// LOADING VBO FOR AXIS ////////////////////////////////
	GLCall(glGenBuffers(1, &this->vertexBuffer_axis));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer_axis));
	// GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
	{
		GLuint vertex_position_layout = 0;
		GLCall(glEnableVertexAttribArray(vertex_position_layout));					// size appart				// offset
		GLCall(glVertexAttribPointer(vertex_position_layout, 3, GL_FLOAT, GL_FALSE, sizeof(AxisVertex), (const void *)offsetof(AxisVertex, coords)));
		// GLCall(glVertexAttribDivisor(vertex_position_layout, 0)); // values are per vertex

		GLuint vertex_color_layout = 1;
		GLCall(glEnableVertexAttribArray(vertex_color_layout));					// size appart				// offset
		GLCall(glVertexAttribPointer(vertex_color_layout, 4, GL_FLOAT, GL_FALSE, sizeof(AxisVertex), (const void *)offsetof(AxisVertex, color)));
		// GLCall(glVertexAttribDivisor(vertex_color_layout, 0)); // values are per vertex
	}

	//////////////////////////// LOADING VAO FOR HDR ////////////////////////////
	GLCall(glGenVertexArrays(1, &this->VAO_viewport));
	GLCall(glBindVertexArray(this->VAO_viewport));

	//////////////////////////// LOADING VBO FOR HDR ////////////////////////////
	GLCall(glGenBuffers(1, &this->vertexBuffer_viewport));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer_viewport));
	// GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
	{
		GLuint vertex_position_layout = 0;
		GLCall(glEnableVertexAttribArray(vertex_position_layout));					// size appart				// offset
		GLCall(glVertexAttribPointer(vertex_position_layout, 4, GL_FLOAT, GL_FALSE, sizeof(ViewportVertex), (const void *)offsetof(ViewportVertex, coords)));
		// GLCall(glVertexAttribDivisor(vertex_position_layout, 0)); // values are per vertex

		GLuint vertex_texcoord_layout = 1;
		GLCall(glEnableVertexAttribArray(vertex_texcoord_layout));					// size appart				// offset
		GLCall(glVertexAttribPointer(vertex_texcoord_layout, 2, GL_FLOAT, GL_FALSE, sizeof(ViewportVertex), (const void *)offsetof(ViewportVertex, tex_coord)));
		// GLCall(glVertexAttribDivisor(vertex_normal_layout, 0)); // values are per vertex
	}

	//////////////////////////// LOADING VAO FOR MODELS ////////////////////////////
	GLCall(glGenVertexArrays(1, &this->VAO_models));
	GLCall(glBindVertexArray(this->VAO_models));

	glGenBuffers(1, &IBO_models);

	//////////////////////////// LOADING VBO FOR MODELS ////////////////////////////
	GLCall(glGenBuffers(1, &this->VBO_models));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->VBO_models));
	// GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
	{
		GLuint vertex_position_layout = 0;
		GLCall(glEnableVertexAttribArray(vertex_position_layout));					// size appart				// offset
		GLCall(glVertexAttribPointer(vertex_position_layout, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (const void *)offsetof(ModelVertex, coords)));
		// GLCall(glVertexAttribDivisor(vertex_position_layout, 0)); // values are per vertex

		GLuint vertex_normal_layout = 1;
		GLCall(glEnableVertexAttribArray(vertex_normal_layout));					// size appart				// offset
		GLCall(glVertexAttribPointer(vertex_normal_layout, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (const void *)offsetof(ModelVertex, normal)));

		GLuint vertex_uv_layout = 2;
		GLCall(glEnableVertexAttribArray(vertex_uv_layout));					// size appart				// offset
		GLCall(glVertexAttribPointer(vertex_uv_layout, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (const void *)offsetof(ModelVertex, uv)));
	}

	GLCall(glGenBuffers(1, &TBO_models_buffer));
	GLCall(glBindBuffer(GL_TEXTURE_BUFFER, TBO_models_buffer));
	GLCall(glGenTextures(1, &TBO_models));
	GLCall(glBindTexture(GL_TEXTURE_BUFFER, TBO_models));
	GLCall(glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, TBO_models_buffer)); // bind the buffer to the texture

	//////////////////////////// INDIRECT BUFFER ////////////////////////////
	GLCall(glGenBuffers(1, &this->indirectBuffer));
	GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, this->indirectBuffer));

	//////////////////////////// LOADING SHADER UNIFORMS ///////////////////////////
	mainShader.use();
	mainShader.setInt("u_TextureArraySlot", TEXSLOTS::TEX_ARRAY_SLOT);
	mainShader.setMat4("u_Model", glm::mat4(1.0f)); // load identity just for safety
	mainShader.setMat4("u_View", glm::mat4(1.0f)); // load identity just for safety
	mainShader.setMat4("u_Projection", glm::mat4(1.0f)); // load identity just for safety

	modelShader.use();
	modelShader.setInt("u_TextureArraySlot", TEXSLOTS::TEX_ARRAY_SLOT);


	//////////////////////////// load textures with info on materials and lights and chunk info
	GLCall(glGenBuffers(1, &materialBuffer));
	GLCall(glBindBuffer(GL_TEXTURE_BUFFER, materialBuffer));
	GLCall(glGenTextures(1, &materialTBO));
	GLCall(glBindTexture(GL_TEXTURE_BUFFER, materialTBO));
	GLCall(glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, materialBuffer)); // bind the buffer to the texture

	GLCall(glGenBuffers(1, &chunkInfoBuffer));
	GLCall(glBindBuffer(GL_TEXTURE_BUFFER, chunkInfoBuffer));
	GLCall(glGenTextures(1, &chunkInfoTBO));
	GLCall(glBindTexture(GL_TEXTURE_BUFFER, chunkInfoTBO));
	GLCall(glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, chunkInfoBuffer)); // bind the buffer to the texture

	GLCall(glGenBuffers(1, &pointLightBuffer));
	GLCall(glBindBuffer(GL_TEXTURE_BUFFER, pointLightBuffer));
	GLCall(glGenTextures(1, &pointLightTBO));
	GLCall(glBindTexture(GL_TEXTURE_BUFFER, pointLightTBO));
	GLCall(glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, pointLightBuffer)); // bind the buffer to the texture

	GLCall(glGenBuffers(1, &dirLightBuffer));
	GLCall(glBindBuffer(GL_TEXTURE_BUFFER, dirLightBuffer));
	GLCall(glGenTextures(1, &dirLightTBO));
	GLCall(glBindTexture(GL_TEXTURE_BUFFER, dirLightTBO));
	GLCall(glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, dirLightBuffer)); // bind the buffer to the texture

	GLCall(glGenBuffers(1, &spotLightBuffer));
	GLCall(glBindBuffer(GL_TEXTURE_BUFFER, spotLightBuffer));
	GLCall(glGenTextures(1, &spotLightTBO));
	GLCall(glBindTexture(GL_TEXTURE_BUFFER, spotLightTBO));
	GLCall(glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, spotLightBuffer)); // bind the buffer to the texture

	//////////////////////////// load texture with info about chunks
	// ................


	// for axis shader
	axisShader.use();
	axisShader.setMat4("u_MVP", glm::mat4(1.0f));  // load identity just for safety

	//////////////////////////// LOADING TEXTURES ///////////////////////////
	loadTextures();


	//////////////////////////// LOADING FRAMEBUFFERS AND TEXTURE ATTACHMENTS ////////////////////////////
	GLCall(glGenFramebuffers(1, &lightingFBO));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, lightingFBO));
		generate_FBO_depth_buffer(&lightingFBODepthBuffer);
		generate_FBO_texture(&lightingTexture, GL_COLOR_ATTACHMENT0);
		generate_FBO_texture(&brightTexture, GL_COLOR_ATTACHMENT1);
		GLCall(checkFrameBuffer());
	GLCall(glGenFramebuffers(2, pingpongFBO));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]));
		generate_FBO_texture(&pingpongTextures[0], GL_COLOR_ATTACHMENT0);
		GLCall(checkFrameBuffer());
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[1]));
		generate_FBO_texture(&pingpongTextures[1], GL_COLOR_ATTACHMENT0);
		GLCall(checkFrameBuffer());
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));



	//////////////////////////// CLEANUP ///////////////////////////
	GLCall(glUseProgram(0));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	GLCall(glBindVertexArray(0));
}

Renderer::~Renderer() {
	GLCall(glDeleteBuffers(1, &VBO));
	GLCall(glDeleteBuffers(1, &VBO_base));
	GLCall(glDeleteBuffers(1, &materialBuffer));
	GLCall(glDeleteBuffers(1, &vertexBuffer_axis));
	GLCall(glDeleteBuffers(1, &vertexBuffer_viewport));
	GLCall(glDeleteBuffers(1, &indirectBuffer));

	GLCall(glDeleteVertexArrays(1, &VAO));
	GLCall(glDeleteVertexArrays(1, &VAO_axis));
	GLCall(glDeleteVertexArrays(1, &VAO_viewport));

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
	GLCall(glDeleteTextures(1, &lightingTexture));
	GLCall(glDeleteTextures(2, pingpongTextures));
	// delete the TBOs????????????????????????????????????????????

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, lightingFBO));
	GLCall(glDeleteRenderbuffers(1, &lightingFBODepthBuffer));

	GLCall(glDeleteFramebuffers(1, &lightingFBO));
	GLCall(glDeleteFramebuffers(2, pingpongFBO));
}

void Renderer::loadTextures() {
	// load texture array instance
	this->textureArray = std::make_unique<TextureArray>(TEXTURE_WIDTH, TEXTURE_HEIGHT, TEXTURE_LAYERS);

	TextureArray *tex = textureArray.get();
	tex->addTexture("textures/missing_texture.png"); // 0
	tex->addTexture("textures/black.png"); // 1
	tex->addTexture("textures/white.png"); // 2
	tex->addTexture("textures/birch_planks.png"); // 3
	tex->addTexture("textures/redstone_block.png"); // 4
	tex->setTextureArrayToSlot(TEXSLOTS::TEX_ARRAY_SLOT);
}

void Renderer::prepareFrame(GLuint num_triangles, Position &pos, Direction &dir, Movement &mov, GLfloat deltaTime, const SelectedBlockInfo &selectedInfo) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Debug");
	// ImGui::ShowDemoWindow();
	ImGui::Text("FPS: %lf", 1.0f / deltaTime);
	ImGui::Text("Facing x:%f y:%f z:%f", dir.front.x, dir.front.y, dir.front.z);
	ImGui::Text("Selected: material %d, chunk %u, normal %u, empty %d, world pos %d %d %d", selectedInfo.materialID, selectedInfo.chunkID, selectedInfo.normal, selectedInfo.isEmpty(), selectedInfo.world_pos.x, selectedInfo.world_pos.y, selectedInfo.world_pos.z);

	ImGui::Text("%u triangles", num_triangles);
	ImGui::InputFloat3("Position", glm::value_ptr(pos.pos));
	ImGui::SliderFloat("##Camera_speed", &mov.speed, 0.0f, 1000.0f, "Camera speed = %.3f");
	ImGui::SameLine();
	ImGui::InputFloat("Camera speed", &mov.speed, 1.0f, 10.0f);
	float fov = static_cast<GLfloat>(Settings::fov);
	ImGui::SliderFloat("FOV", &fov, 0.0f, 140.0f, "fov = %.3f");
	Settings::setFov(static_cast<GLdouble>(fov));
	ImGui::SliderFloat("gamma", &Settings::gamma, 0.0f, 10.0f, "gamma = %.3f");
	ImGui::SliderFloat("exposure", &Settings::exposure, 0.0f, 10.0f, "exposure = %.3f");
	ImGui::InputInt("bloomPasses", &Settings::bloomBlurPasses, 1, 1); if (Settings::bloomBlurPasses < 0) Settings::bloomBlurPasses = 0;
	// ImGui::InputInt("bloomPasses", &bloomBlurPasses, 1, 1, "bloomPasses = %d");
	ImGui::SliderFloat("bloomThreshold", &Settings::bloomThreshold, 0.0f, 5.0f);
	// texOffsetCoeff = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX) * 10.0f;
	ImGui::SliderFloat("bloomOffset", &Settings::bloomOffset, 0.0f, 10.0f, "bloomOffset = %.3f");
	ImGui::Checkbox("Show axis", &Settings::showAxis);
	ImGui::Checkbox("Show normals", &Settings::showNormals);
	ImGui::Checkbox("Wireframe", &Settings::wireframe);
	ImGui::Checkbox("Wireframe models", &Settings::wireframe_models);
	ImGui::SliderFloat("break_radius", &Settings::break_radius, 1.0f, 100.0f, "break_radius = %.3f");
	ImGui::SliderFloat("break_range", &Settings::break_range, 1.0f, 500.0f, "break_range = %.3f");
	ImGui::Checkbox("Render physics", &Settings::render_physics);
	ImGui::Checkbox("Render", &Settings::render);
	ImGui::Checkbox("Render models", &Settings::render_models);
	ImGui::Checkbox("Select", &Settings::select);
	ImGui::Checkbox("Show model normals", &Settings::showModelNormals);
	ImGui::Checkbox("Limit fps", &Settings::limitFPS);
	ImGui::SliderFloat("FPS limit", &Settings::fps, 0.0f, 240.0f, "FPS limit = %.3f");
	ImGui::Checkbox("Insert", &Settings::insert);
	ImGui::Checkbox("Brake and place voxels", &Settings::edit_terrain);
}

void Renderer::drawLighting(const CustomVec<Vertex> &verts, const CustomVec<Point> &points, const std::vector<IndirectData> &indirect, const std::vector<ChunkInfo> &chunkInfo, const glm::mat4 &projection, const glm::mat4 &view) {
	constexpr glm::mat4 model = glm::mat4(1.0f);
	// const glm::mat4 MVP = projection * view * model;

	// for (GLuint i = 0; i < quads.size(); i++) {
	// 	printf("[%u] quad position: %u %u %u len: %u %u\n", i, quads[i].getPosition().x, quads[i].getPosition().y, quads[i].getPosition().z, quads[i].getLen().x, quads[i].getLen().y);
	// }

	//////////////////////////////////////////////// he normal scene is drawn into the lighting framebuffer, where the bright colors are then separated
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, lightingFBO));
		glStencilMask(0xFF); // allow writing to all bits, otherwise clear does not even work
    	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		if (Settings::wireframe) {
			GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
		}
		// glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

		GLCall(glBindVertexArray(this->VAO));

		GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->VBO));
		GLCall(glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW));



		// bind program, load uniforms
		mainShader.use();

		// load MVP, texture array and view
		// this->textureArray.get()->setTextureArrayToSlot(TEXSLOTS::TEX_ARRAY_SLOT);
		// mainShader.setInt("u_TextureArraySlot", TEXSLOTS::TEX_ARRAY_SLOT);
		mainShader.setMat4("u_Model", model);
		mainShader.setMat4("u_View", view);
		mainShader.setMat4("u_Projection", projection);
		mainShader.setMat3("u_NormalMatrix", glm::mat3(glm::transpose(glm::inverse(view * model))));

		mainShader.setFloat("u_BloomThreshold", Settings::bloomThreshold);

		// load UBO
		Material materials[8];
		materials[0] = {
			// glm::vec3(1.0f, 1.0f, 1.0f),
			// glm::vec3(1.0f, 1.0f, 1.0f),
			// glm::vec3(1.0f, 1.0f, 1.0f),
			// // glm::vec3(2.99f, 0.72f, 0.0745f),
			// glm::vec3(0.0f),
			glm::vec3(0.9f, 0.9f, 0.85f),
			glm::vec3(0.95f, 0.95f, 0.9f),
			glm::vec3(0.9f, 0.9f, 0.85f),
			glm::vec3(0.0f),
			32.0f,
			4
		};

		materials[1] = {
			// glm::vec3(1.0f, 1.0f, 1.0f),
			// glm::vec3(1.0f, 1.0f, 1.0f),
			// glm::vec3(1.0f, 1.0f, 1.0f),
			// // glm::vec3(2.99f, 0.72f, 0.0745f),
			// glm::vec3(0.0f),
			glm::vec3(0.9f, 0.9f, 0.85f),
			glm::vec3(0.95f, 0.95f, 0.9f),
			glm::vec3(0.9f, 0.9f, 0.85f),
			glm::vec3(0.0f),
			32.0f,
			3
		};

		GLCall(glBindBuffer(GL_TEXTURE_BUFFER, materialBuffer));
		GLCall(glBufferData(GL_TEXTURE_BUFFER, MAX_MATERIALS * sizeof(Material), materials, GL_STATIC_DRAW));
		GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::MATERIAL_TEXTURE_BUFFER_SLOT));
		GLCall(glBindTexture(GL_TEXTURE_BUFFER, materialTBO));
		// GLCall(glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, materialBuffer)); // bind the buffer to the texture (has been done while setting up)
		mainShader.setInt("u_MaterialTBO", TEXSLOTS::MATERIAL_TEXTURE_BUFFER_SLOT);

		PointLight pointLights[MAX_LIGHTS];
		pointLights[0] = {
			.position = glm::vec3(30.0f, 15.0f, 30.0f),
			.constant = 1.0f,
			.linear = 0.09f,
			.quadratic = 0.032f,
			.ambient = glm::vec3(0.2f, 0.2f, 0.0f),
			.diffuse = glm::vec3(0.78f, 0.78f, 0.0f),
			.specular = glm::vec3(1.0f, 1.0f, 1.0f),
			.padding_1 = 0.0f
		};

		GLCall(glBindBuffer(GL_TEXTURE_BUFFER, pointLightBuffer));
		GLCall(glBufferData(GL_TEXTURE_BUFFER, MAX_LIGHTS * sizeof(PointLight), pointLights, GL_STATIC_DRAW));
		GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::POINTLIGHT_TEXTURE_BUFFER_SLOT));
		GLCall(glBindTexture(GL_TEXTURE_BUFFER, pointLightTBO));
		// GLCall(glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, pointLightBuffer)); // bind the buffer to the texture (has been done while setting up)
		mainShader.setInt("u_PointLightTBO", TEXSLOTS::POINTLIGHT_TEXTURE_BUFFER_SLOT);
		mainShader.setInt("u_NumPointLights", 0);

		DirLight dirLights[MAX_LIGHTS];
		dirLights[0] = {
			// .direction = glm::normalize(glm::vec3(0.5f, -0.45f, 0.5f)),
			// .direction = glm::normalize(glm::vec3(1.0f, 0.1f, 0.0f)),
			.direction = glm::normalize(glm::vec3(0.0f, 0.1f, 1.0f)),
			// .ambient = glm::vec3(0.2f, 0.2f, 0.2f),
			// .diffuse = glm::vec3(0.78f, 0.78f, 0.78f),
			// .specular = glm::vec3(1.0f, 1.0f, 1.0f)
			.ambient = glm::vec3(0.8f, 0.8f, 0.7f),
			.diffuse = glm::vec3(1.0f, 0.96f, 0.86f),
			.specular = glm::vec3(0.9f, 0.9f, 0.8f)
		};

		GLCall(glBindBuffer(GL_TEXTURE_BUFFER, dirLightBuffer));
		GLCall(glBufferData(GL_TEXTURE_BUFFER, MAX_LIGHTS * sizeof(DirLight), dirLights, GL_STATIC_DRAW));
		GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::DIRLIGHT_TEXTURE_BUFFER_SLOT));
		GLCall(glBindTexture(GL_TEXTURE_BUFFER, dirLightTBO));
		// GLCall(glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, pointLightBuffer)); // bind the buffer to the texture (has been done while setting up)
		mainShader.setInt("u_DirLightTBO", TEXSLOTS::DIRLIGHT_TEXTURE_BUFFER_SLOT);
		mainShader.setInt("u_NumDirLights", 1);

		SpotLight spotLights[MAX_LIGHTS];
		spotLights[0] = {
			// .position = camera.Position,
			// .position = glm::vec3(0.0f, 1.0f, 3.0f),
			// .direction = camera.Front,
			// .direction = glm::vec3(0.0f, -0.25f, -0.97f),
			.cutOff = glm::cos(glm::radians(12.5f)),
			.outerCutOff = glm::cos(glm::radians(17.5f)),
			.constant = 1.0f,
			.linear = 0.09f,
			.quadratic = 0.032f,
			.ambient = glm::vec3(0.1f, 0.1f, 0.1f),
			.diffuse = glm::vec3(0.8f, 0.8f, 0.8f),
			.specular = glm::vec3(1.0f, 1.0f, 1.0f)
		};

		GLCall(glBindBuffer(GL_TEXTURE_BUFFER, spotLightBuffer));
		GLCall(glBufferData(GL_TEXTURE_BUFFER, MAX_LIGHTS * sizeof(SpotLight), spotLights, GL_STATIC_DRAW));
		GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::SPOTLIGHT_TEXTURE_BUFFER_SLOT));
		GLCall(glBindTexture(GL_TEXTURE_BUFFER, spotLightTBO));
		// GLCall(glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, spotLightBuffer)); // bind the buffer to the texture (has been done while setting up)
		mainShader.setInt("u_SpotLightTBO", TEXSLOTS::SPOTLIGHT_TEXTURE_BUFFER_SLOT);
		mainShader.setInt("u_NumSpotLights", 0);

		// specify 2 attachments
		constexpr GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		GLCall(glDrawBuffers(2, attachments));

		// mainShader.validate();

		// info for indirect call
		GLCall(glBindBuffer(GL_TEXTURE_BUFFER, chunkInfoBuffer));
		GLCall(glBufferData(GL_TEXTURE_BUFFER, chunkInfo.size() * sizeof(ChunkInfo), chunkInfo.data(), GL_STATIC_DRAW));
		GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::CHUNKINFO_TEXTURE_BUFFER_SLOT));
		GLCall(glBindTexture(GL_TEXTURE_BUFFER, chunkInfoTBO));
		mainShader.setInt("u_ChunkInfoTBO", TEXSLOTS::CHUNKINFO_TEXTURE_BUFFER_SLOT);

		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer));
		GLCall(glBufferData(GL_DRAW_INDIRECT_BUFFER, indirect.size() * sizeof(IndirectData), indirect.data(), GL_DYNAMIC_DRAW));

		if (Settings::render) {
			// GLCall(glDrawArraysInstanced(GL_TRIANGLES, 0, 3, verts.size()));
			GLCall(glMultiDrawArraysIndirect(GL_TRIANGLES, (void *)0, indirect.size(), 0));
		}

		// draw points
		// NEED TO USE OTHER SHADER
		// pointshader.use();
		// pointshader.setMat4("u_Model", model);
		// pointshader.setMat4("u_View", view);
		// pointshader.setMat4("u_Projection", projection);
		// glEnable( GL_PROGRAM_POINT_SIZE );
		// GLCall(glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vertex), points.data(), GL_STATIC_DRAW));
		// GLCall(glDrawArrays(GL_POINTS, 0, points.size()));

		if (Settings::showAxis) {
			drawAxis(model, view, projection);
		}

		if (Settings::showNormals) {
			drawNormals(verts, indirect, model, view, projection);
		}

		if (Settings::wireframe) {
			GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
		}
}

void Renderer::drawNormals(const CustomVec<Vertex> &verts, const std::vector<IndirectData> &indirect, const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &projection) {
	GLCall(glBindVertexArray(this->VAO));

	// GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->VBO));
	// GLCall(glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW));

	normalShader.use();
	normalShader.setMat4("u_Model", model);
	normalShader.setMat4("u_View", view);
	normalShader.setMat4("u_Projection", projection);
	normalShader.setFloat("u_BloomThreshold", Settings::bloomThreshold);
	normalShader.setMat3("u_NormalMatrix", glm::mat3(glm::transpose(glm::inverse(view * model))));
	normalShader.setInt("u_ChunkInfoTBO", TEXSLOTS::CHUNKINFO_TEXTURE_BUFFER_SLOT);

	// normalShader.validate();

	GLCall(glMultiDrawArraysIndirect(GL_TRIANGLES, (void *)0, indirect.size(), 0));
	// GLCall(glDrawArraysInstanced(GL_TRIANGLES, 0, 3, verts.size()));
}

void Renderer::bloomBlur(int passes) {
	//////////////////////////////////////////////// run the ping pong gaussian blur several times
	blurShader.use();
	GLCall(glBindVertexArray(this->VAO_viewport));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer_viewport));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(viewportVertices), viewportVertices, GL_STATIC_DRAW));

	if (passes == 0) {
		// if this happens, instead of switching verything just clear pingpongTextures[1], which will be used in merging the textures
		// bind FBO, attach texture, clear color buffer
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[1]));
		GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::BRIGHT_TEXTURE_SLOT));
		GLCall(glBindTexture(GL_TEXTURE_2D, pingpongTextures[1])); // use texture from pong
		blurShader.setInt("u_BlurBuffer", TEXSLOTS::BRIGHT_TEXTURE_SLOT);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return;
	}


	// manually doing the first passes, since I need to get the texture from the scene
	// horizontal pass
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // isto devia ser chamado sequer???????????????????????????????????? acho que a imagem e 100% overwritten
		blurShader.setInt("u_Horizontal", 0);
		GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::BRIGHT_TEXTURE_SLOT));
		GLCall(glBindTexture(GL_TEXTURE_2D, this->brightTexture)); // use texture from scene
		blurShader.setInt("u_BlurBuffer", TEXSLOTS::BRIGHT_TEXTURE_SLOT);
		GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));

	// vertical pass
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[1]));
    	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		blurShader.setInt("u_Horizontal", 1);
		GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::BRIGHT_TEXTURE_SLOT));
		GLCall(glBindTexture(GL_TEXTURE_2D, pingpongTextures[0])); // use texture from ping
		blurShader.setInt("u_BlurBuffer", TEXSLOTS::BRIGHT_TEXTURE_SLOT);
		GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));

	for (GLint i = 0; i < passes - 1; i++) {
		blurShader.setFloat("u_TexOffsetCoeff", Settings::bloomOffset);

		// horizontal pass
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]));
			blurShader.setInt("u_Horizontal", 0);
			GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::BRIGHT_TEXTURE_SLOT));
			GLCall(glBindTexture(GL_TEXTURE_2D, pingpongTextures[1])); // use texture from pong
			blurShader.setInt("u_BlurBuffer", TEXSLOTS::BRIGHT_TEXTURE_SLOT);
			GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));

		// vertical pass
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[1]));
			blurShader.setInt("u_Horizontal", 1);
			GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::BRIGHT_TEXTURE_SLOT));
			GLCall(glBindTexture(GL_TEXTURE_2D, pingpongTextures[0])); // use texture from ping
			blurShader.setInt("u_BlurBuffer", TEXSLOTS::BRIGHT_TEXTURE_SLOT);
			GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));
	}
}

void Renderer::merge() {
	//////////////////////////////////////////////// join the blur to the scene and apply gamma and exposure
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// GLCall(glBindVertexArray(this->VAO_viewport));
		// GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer_viewport));
		// GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(viewportVertices), viewportVertices, GL_STATIC_DRAW));

		hdrBbloomMergeShader.use();
		GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::SCENE_TEXTURE_SLOT));
		GLCall(glBindTexture(GL_TEXTURE_2D, this->lightingTexture));
		GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::BRIGHT_TEXTURE_SLOT));
		GLCall(glBindTexture(GL_TEXTURE_2D, pingpongTextures[1]));

		hdrBbloomMergeShader.setInt("u_SceneBuffer", TEXSLOTS::SCENE_TEXTURE_SLOT);
		hdrBbloomMergeShader.setInt("u_BrightBuffer", TEXSLOTS::BRIGHT_TEXTURE_SLOT);
		hdrBbloomMergeShader.setFloat("u_Gamma", Settings::gamma);
		hdrBbloomMergeShader.setFloat("u_Exposure", Settings::exposure);

		// hdrBbloomMergeShader.validate();
		GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));
}

void Renderer::endFrame(GLFWwindow * window) {
	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

void Renderer::postProcess(int bloomBlurPasses) {
	bloomBlur(bloomBlurPasses);
	merge();
}

void Renderer::draw(const glm::mat4 &view, const CustomVec<Vertex> &verts, const CustomVec<Point> &points, const std::vector<IndirectData> &indirect, const std::vector<ChunkInfo> &chunkInfo, const DrawObjects &objs, const DrawObjects &selected_objs, const glm::mat4 &projection, GLFWwindow * window, GLfloat deltaTime, Position &pos, Direction &dir, Movement &mov, const SelectedBlockInfo &selectedInfo) {
	ZoneScoped;
	
	prepareFrame(verts.size(), pos, dir, mov, deltaTime, selectedInfo);
	drawLighting(verts, points, indirect, chunkInfo, projection, view);
	drawObjects(view, projection, objs);
	drawSelectedObjects(view, projection, selected_objs);

	if (Settings::render_physics) {
		draw_phys(view, projection);
	}
}

// make sure fbo is bound before calling this
// !!!! I changed from GL_DEPTH_COMPONENT to GL_DEPTH24_STENCIL8 and GL_DEPTH_ATTACHMENT to GL_DEPTH_STENCIL_ATTACHMENT
void Renderer::generate_FBO_depth_buffer(GLuint *depthBuffer) const {

	if (*depthBuffer > 0) {
		glDeleteRenderbuffers(1, depthBuffer);
	}

	glGenRenderbuffers(1, depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, *depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->viewport_width, this->viewport_height);
	// bind the render buffer to this FBO (maybe this is missing actualy binding it, idk, but it gets regenerated automatically when screen is resized)
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *depthBuffer);
}

// this does NOT take into acount currently used textures slots etc, only here for organisation
// make sure fbo is bound before calling this
void Renderer::generate_FBO_texture(GLuint *textureID, GLenum attachmentID) {
	// delete existing texture, if needed
	if (*textureID != 0) { // for safety, delete the texture entirely. maybe does not need to be done
		GLCall(glBindTexture(GL_TEXTURE_2D, 0));
		GLCall(glDeleteTextures(1, textureID));
	}

	GLCall(glGenTextures(1, textureID));
	GLCall(glBindTexture(GL_TEXTURE_2D, *textureID));
	// you can get the default behaviour by either not using the framebuffer or setting this to GL_RGBA
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, this->viewport_width, this->viewport_height, 0, GL_RGBA, GL_FLOAT, NULL));  // change to 32float if needed

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	// attach to fbo
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentID, GL_TEXTURE_2D, *textureID, 0));
}

// regenerate the textures for all the FBOs
void Renderer::resizeViewport(GLsizei viewport_width, GLsizei viewport_height) {
	this->viewport_width = viewport_width;
	this->viewport_height = viewport_height;

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, lightingFBO));
	generate_FBO_depth_buffer(&lightingFBODepthBuffer);
	generate_FBO_texture(&lightingTexture, GL_COLOR_ATTACHMENT0);
	generate_FBO_texture(&brightTexture, GL_COLOR_ATTACHMENT1);
	GLCall(checkFrameBuffer());

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]));
	generate_FBO_texture(&pingpongTextures[0], GL_COLOR_ATTACHMENT0);
	GLCall(checkFrameBuffer());
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[1]));
	generate_FBO_texture(&pingpongTextures[1], GL_COLOR_ATTACHMENT0);
	GLCall(checkFrameBuffer());

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

// needs to be improved
void Renderer::checkFrameBuffer() {
	GLCall(GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

	CRASH_IF(status != GL_FRAMEBUFFER_COMPLETE, "Error in fbo");
}

// TODO optimize, for now 1 draw per object
void Renderer::drawObjects(const glm::mat4 &view, const glm::mat4 &projection, const DrawObjects &objs) {
	if (Settings::wireframe_models) {
		GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
	}
	
	modelShader.use();

	// bind VAO, VBO, TBO
	GLCall(glBindVertexArray(this->VAO_models));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->VBO_models));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->IBO_models));
	modelShader.setInt("u_TransformTBO", TEXSLOTS::MODELS_TRANSFORM_TEXTURE_BUFFER_SLOT);
	// modelShader.setInt("u_NormalMatTBO", TEXSLOTS::MODELS_NORMALMAT_TEXTURE_BUFFER_SLOT);


	// TODO clean this up, same initialization as main shader
	modelShader.setFloat("u_BloomThreshold", Settings::bloomThreshold);

	// // load UBO
	// Material materials[8];
	// materials[0] = {
	// 	// glm::vec3(1.0f, 1.0f, 1.0f),
	// 	// glm::vec3(1.0f, 1.0f, 1.0f),
	// 	// glm::vec3(1.0f, 1.0f, 1.0f),
	// 	// // glm::vec3(2.99f, 0.72f, 0.0745f),
	// 	// glm::vec3(0.0f),
	// 	glm::vec3(0.9f, 0.9f, 0.85f),
	// 	glm::vec3(0.95f, 0.95f, 0.9f),
	// 	glm::vec3(0.9f, 0.9f, 0.85f),
	// 	glm::vec3(0.0f),
	// 	32.0f,
	// 	4
	// };

	// materials[1] = {
	// 	// glm::vec3(1.0f, 1.0f, 1.0f),
	// 	// glm::vec3(1.0f, 1.0f, 1.0f),
	// 	// glm::vec3(1.0f, 1.0f, 1.0f),
	// 	// // glm::vec3(2.99f, 0.72f, 0.0745f),
	// 	// glm::vec3(0.0f),
	// 	glm::vec3(0.9f, 0.9f, 0.85f),
	// 	glm::vec3(0.95f, 0.95f, 0.9f),
	// 	glm::vec3(0.9f, 0.9f, 0.85f),
	// 	glm::vec3(0.0f),
	// 	32.0f,
	// 	3
	// };

	// GLCall(glBindBuffer(GL_TEXTURE_BUFFER, materialBuffer));
	// GLCall(glBufferData(GL_TEXTURE_BUFFER, MAX_MATERIALS * sizeof(Material), materials, GL_STATIC_DRAW));
	// GLCall(glActiveTexture(TEXSLOTS::MATERIAL_TEXTURE_BUFFER_SLOT));
	// GLCall(glBindTexture(GL_TEXTURE_BUFFER, materialTBO));
	// GLCall(glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, materialBuffer)); // bind the buffer to the texture (has been done while setting up)
	modelShader.setInt("u_MaterialTBO", TEXSLOTS::MATERIAL_TEXTURE_BUFFER_SLOT);

	// PointLight pointLights[MAX_LIGHTS];
	// pointLights[0] = {
	// 	.position = glm::vec3(30.0f, 15.0f, 30.0f),
	// 	.constant = 1.0f,
	// 	.linear = 0.09f,
	// 	.quadratic = 0.032f,
	// 	.ambient = glm::vec3(0.2f, 0.2f, 0.0f),
	// 	.diffuse = glm::vec3(0.78f, 0.78f, 0.0f),
	// 	.specular = glm::vec3(1.0f, 1.0f, 1.0f),
	// 	.padding_1 = 0.0f
	// };

	// GLCall(glBindBuffer(GL_TEXTURE_BUFFER, pointLightBuffer));
	// GLCall(glBufferData(GL_TEXTURE_BUFFER, MAX_LIGHTS * sizeof(PointLight), pointLights, GL_STATIC_DRAW));
	// GLCall(glActiveTexture(TEXSLOTS::POINTLIGHT_TEXTURE_BUFFER_SLOT));
	// GLCall(glBindTexture(GL_TEXTURE_BUFFER, pointLightTBO));
	// GLCall(glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, pointLightBuffer)); // bind the buffer to the texture (has been done while setting up)
	modelShader.setInt("u_PointLightTBO", TEXSLOTS::POINTLIGHT_TEXTURE_BUFFER_SLOT);
	modelShader.setInt("u_NumPointLights", 0);

	// DirLight dirLights[MAX_LIGHTS];
	// dirLights[0] = {
	// 	// .direction = glm::normalize(glm::vec3(0.5f, -0.45f, 0.5f)),
	// 	// .direction = glm::normalize(glm::vec3(1.0f, 0.1f, 0.0f)),
	// 	.direction = glm::normalize(glm::vec3(0.0f, 0.1f, 1.0f)),
	// 	// .ambient = glm::vec3(0.2f, 0.2f, 0.2f),
	// 	// .diffuse = glm::vec3(0.78f, 0.78f, 0.78f),
	// 	// .specular = glm::vec3(1.0f, 1.0f, 1.0f)
	// 	.ambient = glm::vec3(0.8f, 0.8f, 0.7f),
	// 	.diffuse = glm::vec3(1.0f, 0.96f, 0.86f),
	// 	.specular = glm::vec3(0.9f, 0.9f, 0.8f)
	// };

	// GLCall(glBindBuffer(GL_TEXTURE_BUFFER, dirLightBuffer));
	// GLCall(glBufferData(GL_TEXTURE_BUFFER, MAX_LIGHTS * sizeof(DirLight), dirLights, GL_STATIC_DRAW));
	// GLCall(glActiveTexture(TEXSLOTS::DIRLIGHT_TEXTURE_BUFFER_SLOT));
	// GLCall(glBindTexture(GL_TEXTURE_BUFFER, dirLightTBO));
	// // GLCall(glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, pointLightBuffer)); // bind the buffer to the texture (has been done while setting up)
	modelShader.setInt("u_DirLightTBO", TEXSLOTS::DIRLIGHT_TEXTURE_BUFFER_SLOT);
	modelShader.setInt("u_NumDirLights", 1);

	// SpotLight spotLights[MAX_LIGHTS];
	// spotLights[0] = {
	// 	// .position = camera.Position,
	// 	// .position = glm::vec3(0.0f, 1.0f, 3.0f),
	// 	// .direction = camera.Front,
	// 	// .direction = glm::vec3(0.0f, -0.25f, -0.97f),
	// 	.cutOff = glm::cos(glm::radians(12.5f)),
	// 	.outerCutOff = glm::cos(glm::radians(17.5f)),
	// 	.constant = 1.0f,
	// 	.linear = 0.09f,
	// 	.quadratic = 0.032f,
	// 	.ambient = glm::vec3(0.1f, 0.1f, 0.1f),
	// 	.diffuse = glm::vec3(0.8f, 0.8f, 0.8f),
	// 	.specular = glm::vec3(1.0f, 1.0f, 1.0f)
	// };

	// GLCall(glBindBuffer(GL_TEXTURE_BUFFER, spotLightBuffer));
	// GLCall(glBufferData(GL_TEXTURE_BUFFER, MAX_LIGHTS * sizeof(SpotLight), spotLights, GL_STATIC_DRAW));
	// GLCall(glActiveTexture(TEXSLOTS::SPOTLIGHT_TEXTURE_BUFFER_SLOT));
	// GLCall(glBindTexture(GL_TEXTURE_BUFFER, spotLightTBO));
	// // GLCall(glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, spotLightBuffer)); // bind the buffer to the texture (has been done while setting up)
	modelShader.setInt("u_SpotLightTBO", TEXSLOTS::SPOTLIGHT_TEXTURE_BUFFER_SLOT);
	modelShader.setInt("u_NumSpotLights", 0);

	// specify 2 attachments
	constexpr GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	GLCall(glDrawBuffers(2, attachments));

	// modelShader.validate();
	modelShader.setMat4("u_View", view);
	modelShader.setMat4("u_Projection", projection);

	if (Settings::render_models) {
		for (const auto &pair : objs) {

			if (pair.second.size() == 0) continue; // no entities

			// verts loaded once
			const GameObject *obj = pair.first;
			GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(ModelVertex) * obj->verts.size(), obj->verts.data(), GL_STATIC_DRAW));
			GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * obj->indices.size(), obj->indices.data(), GL_STATIC_DRAW));

			// TODO use a VBO instead?
			GLCall(glBindBuffer(GL_TEXTURE_BUFFER, TBO_models_buffer));
			GLCall(glBufferData(GL_TEXTURE_BUFFER, pair.second.size() * sizeof(glm::mat4), pair.second.data(), GL_STATIC_DRAW));
			GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::MODELS_TRANSFORM_TEXTURE_BUFFER_SLOT)); // TODO call this only once?
			GLCall(glBindTexture(GL_TEXTURE_BUFFER, TBO_models));

			// TODO compute normal matrices here on the cpu

			GLCall(glDrawElementsInstanced(GL_TRIANGLES, obj->indices.size(), GL_UNSIGNED_INT, 0, pair.second.size()));


				// modelShader.setMat3("u_NormalMatrix", glm::mat3(glm::transpose(glm::inverse(view * transform))));
				// modelShader.setMat4("u_Model", transform);

				// GLCall(glDrawElements(GL_TRIANGLES, obj->indices.size(), GL_UNSIGNED_INT, 0));
		}
	}

	if (Settings::wireframe_models) {
		GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	}

	if (Settings::showModelNormals) {
		drawObjectNormals(view, projection, objs);
	}
}


// copied from drawObjects!!!!!!
void Renderer::drawSelectedObjects(const glm::mat4 &view, const glm::mat4 &projection, const DrawObjects &objs) {
	// setup stencil
	glEnable(GL_STENCIL_TEST);
	// glClear(GL_STENCIL_BUFFER_BIT);
	// Configure stencil to write operation
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 0xFF); // Always pass the stencil test
    glStencilMask(0xFF); // Enable writing to the stencil buffer


	if (Settings::wireframe_models) {
		GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
	}
	
	selectedModelShader.use();

	// bind VAO, VBO, TBO
	GLCall(glBindVertexArray(this->VAO_models));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->VBO_models));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->IBO_models));
	selectedModelShader.setInt("u_TransformTBO", TEXSLOTS::MODELS_TRANSFORM_TEXTURE_BUFFER_SLOT);

	// TODO clean this up, same initialization as main shader
	selectedModelShader.setFloat("u_BloomThreshold", Settings::bloomThreshold);

	selectedModelShader.setInt("u_MaterialTBO", TEXSLOTS::MATERIAL_TEXTURE_BUFFER_SLOT);
	selectedModelShader.setInt("u_PointLightTBO", TEXSLOTS::POINTLIGHT_TEXTURE_BUFFER_SLOT);
	selectedModelShader.setInt("u_NumPointLights", 0);
	selectedModelShader.setInt("u_DirLightTBO", TEXSLOTS::DIRLIGHT_TEXTURE_BUFFER_SLOT);
	selectedModelShader.setInt("u_NumDirLights", 1);

	selectedModelShader.setInt("u_SpotLightTBO", TEXSLOTS::SPOTLIGHT_TEXTURE_BUFFER_SLOT);
	selectedModelShader.setInt("u_NumSpotLights", 0);

	constexpr GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	GLCall(glDrawBuffers(2, attachments));

	selectedModelShader.setMat4("u_View", view);
	selectedModelShader.setMat4("u_Projection", projection);

	selectedModelShader.setVec4("u_Color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	// TODO optimize this mess
	if (Settings::render_models) {
		// first pass, render normally. will write to stencil buffer
		for (const auto &pair : objs) {

			if (pair.second.size() == 0) continue; // no entities (can this ever happen????? TODO)

			// verts loaded once
			const GameObject *obj = pair.first;
			GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(ModelVertex) * obj->verts.size(), obj->verts.data(), GL_STATIC_DRAW));
			GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * obj->indices.size(), obj->indices.data(), GL_STATIC_DRAW));

			// TODO use a VBO instead?
			GLCall(glBindBuffer(GL_TEXTURE_BUFFER, TBO_models_buffer));
			GLCall(glBufferData(GL_TEXTURE_BUFFER, pair.second.size() * sizeof(glm::mat4), pair.second.data(), GL_STATIC_DRAW));
			GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::MODELS_TRANSFORM_TEXTURE_BUFFER_SLOT)); // TODO call this only once?
			GLCall(glBindTexture(GL_TEXTURE_BUFFER, TBO_models));


			GLCall(glDrawElementsInstanced(GL_TRIANGLES, obj->indices.size(), GL_UNSIGNED_INT, 0, pair.second.size()));
		}

		// second pass, mask against stencil buffer, draw slightly larger versions
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);

		outlineShader.use();
		outlineShader.setInt("u_TransformTBO", TEXSLOTS::MODELS_TRANSFORM_TEXTURE_BUFFER_SLOT);
		outlineShader.setMat4("u_View", view);
		outlineShader.setMat4("u_Projection", projection);
		glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.1f, 1.1f, 1.1f));
		outlineShader.setMat4("u_Model", model);
		outlineShader.setMat4("u_Projection", projection);

		for (const auto &pair : objs) {
			if (pair.second.size() == 0) continue; // no entities

			// verts loaded once
			const GameObject *obj = pair.first;
			GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(ModelVertex) * obj->verts.size(), obj->verts.data(), GL_STATIC_DRAW));
			GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * obj->indices.size(), obj->indices.data(), GL_STATIC_DRAW));

			// TODO use a VBO instead?
			GLCall(glBindBuffer(GL_TEXTURE_BUFFER, TBO_models_buffer));
			GLCall(glBufferData(GL_TEXTURE_BUFFER, pair.second.size() * sizeof(glm::mat4), pair.second.data(), GL_STATIC_DRAW));
			GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::MODELS_TRANSFORM_TEXTURE_BUFFER_SLOT)); // TODO call this only once?
			GLCall(glBindTexture(GL_TEXTURE_BUFFER, TBO_models));


			GLCall(glDrawElementsInstanced(GL_TRIANGLES, obj->indices.size(), GL_UNSIGNED_INT, 0, pair.second.size()));
		}
	}

	if (Settings::wireframe_models) {
		GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	}

	// cleanup stencil, so that other draw calls don't use it
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::drawObjectNormals(const glm::mat4 &view, const glm::mat4 &projection, const DrawObjects &objs) {
	modelNormalShader.use();

	// bind VAO, VBO, TBO
	GLCall(glBindVertexArray(this->VAO_models));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->VBO_models));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->IBO_models));
	modelNormalShader.setInt("u_TransformTBO", TEXSLOTS::MODELS_TRANSFORM_TEXTURE_BUFFER_SLOT);

	// TODO clean this up, same initialization as main shader
	modelNormalShader.setFloat("u_BloomThreshold", Settings::bloomThreshold);

	constexpr GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	GLCall(glDrawBuffers(2, attachments));

	modelNormalShader.setMat4("u_View", view);
	modelNormalShader.setMat4("u_Projection", projection);

	for (const auto &pair : objs) {

		if (pair.second.size() == 0) continue; // no entities

		// verts loaded once
		const GameObject *obj = pair.first;
		GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(ModelVertex) * obj->verts.size(), obj->verts.data(), GL_STATIC_DRAW));
		GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * obj->indices.size(), obj->indices.data(), GL_STATIC_DRAW));

		// TODO use a VBO instead?
		GLCall(glBindBuffer(GL_TEXTURE_BUFFER, TBO_models_buffer));
		GLCall(glBufferData(GL_TEXTURE_BUFFER, pair.second.size() * sizeof(glm::mat4), pair.second.data(), GL_STATIC_DRAW));
		GLCall(glActiveTexture(TEXSLOTS::BASESLOT + TEXSLOTS::MODELS_TRANSFORM_TEXTURE_BUFFER_SLOT)); // TODO call this only once?
		GLCall(glBindTexture(GL_TEXTURE_BUFFER, TBO_models));


		GLCall(glDrawElementsInstanced(GL_TRIANGLES, obj->indices.size(), GL_UNSIGNED_INT, 0, pair.second.size()));
	}
}

void Renderer::draw_phys(const glm::mat4 &view, const glm::mat4 &projection) {
	phys_renderer->clearVerts();
	Phys::buildDebugVerts();
	const std::vector<PhysVertex> &verts = phys_renderer->getVerts();

	// cursed, but axis VAO and shader have exactly what I need so I'll just reuse it. currently PhysVertex == AxisVertex

	// bind VAO, VBO
	GLCall(glBindVertexArray(this->VAO_axis));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer_axis));

	// load vertices
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(PhysVertex) * verts.size(), verts.data(), GL_STATIC_DRAW));

	axisShader.use();
	axisShader.setMat4("u_MVP", projection * view);

	GLCall(glDrawArrays(GL_TRIANGLES, 0, verts.size()));
}

void Renderer::drawInsert(const glm::mat4 &view, const glm::mat4 &projection, const InsertInfo &insertInfo, bool valid) {
	if (Settings::wireframe_models) {
		GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
	}
	
	insertShader.use();

	// bind VAO, VBO, TBO
	GLCall(glBindVertexArray(this->VAO_models));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->VBO_models));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->IBO_models));

	insertShader.setFloat("u_BloomThreshold", Settings::bloomThreshold);

	insertShader.setInt("u_MaterialTBO", TEXSLOTS::MATERIAL_TEXTURE_BUFFER_SLOT);

	insertShader.setInt("u_PointLightTBO", TEXSLOTS::POINTLIGHT_TEXTURE_BUFFER_SLOT);
	insertShader.setInt("u_NumPointLights", 0);

	insertShader.setInt("u_DirLightTBO", TEXSLOTS::DIRLIGHT_TEXTURE_BUFFER_SLOT);
	insertShader.setInt("u_NumDirLights", 1);

	insertShader.setInt("u_SpotLightTBO", TEXSLOTS::SPOTLIGHT_TEXTURE_BUFFER_SLOT);
	insertShader.setInt("u_NumSpotLights", 0);

	// specify 2 attachments
	constexpr GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	GLCall(glDrawBuffers(2, attachments));

	// insertShader.validate();
	insertShader.setMat4("u_View", view);
	insertShader.setMat4("u_Projection", projection);

	glm::vec4 color;
	if (valid) {
		color = glm::vec4(0.0f, 1.0f, 0.0f, 0.5f);
	} else {
		color = glm::vec4(1.0f, 0.0f, 0.0f, 0.5f);
	}
	insertShader.setVec4("u_Color", color);


	// TODO get this out of here
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(insertInfo.pos));
	glm::mat4 rotationMatrix = glm::toMat4(insertInfo.rot); // TODO use glm::rotate instead???
	const glm::mat4 model = translationMatrix * rotationMatrix;
	insertShader.setMat4("u_Model", model);

	const GameObject *obj = insertInfo.obj;
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(ModelVertex) * obj->verts.size(), obj->verts.data(), GL_STATIC_DRAW));
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * obj->indices.size(), obj->indices.data(), GL_STATIC_DRAW));

	// TODO use a VBO instead?
	GLCall(glBindBuffer(GL_TEXTURE_BUFFER, TBO_models_buffer));


	// TODO compute normal matrix here on the cpu

	GLCall(glDrawElements(GL_TRIANGLES, obj->indices.size(), GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(0)));

	if (Settings::wireframe_models) {
		GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	}
}
