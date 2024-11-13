#ifndef RENDERER_H
#define RENDERER_H

#include "common.hpp"
#include "Chunk.hpp"
#include "Shader.hpp"
#include "TextureArray.hpp"
#include <memory>
#include "Components.hpp"
#include <vector>
#include "Assets.hpp"
#include "Cubemap.hpp"

#include "PhysRenderer.hpp"

#include "Phys.hpp" // cursed, to call Phys::buildDebugVerts(); and get InsertInfo

using DrawObjects = std::vector<std::pair<GameObject *, std::vector<glm::mat4>>>;

using callbackfunc = void (*)(void *, const void *);

template <typename T>
struct MenuCallbackData {
	// when a setting has changed, callback is called with user_data and data
	// to tell if it has changed, last value is saved here too (but no longer as a pointer)
	// data is a pointer since it's going to be changed by imgui
	T *data;
	T old_data;

	const std::string name;
	void *user_data;
	callbackfunc callback;
	MenuCallbackData(T *data, const std::string &name, void *user_data, callbackfunc callback)
		: data(data), old_data(*data), name(name), user_data(user_data), callback(callback) {}
	~MenuCallbackData() = default;

	void callbackIfNeeded() {
		if (old_data != *data) {
			old_data = *data;
			callback(user_data, data);
		}
	}
};
// ugly hack, specific for floats, idc, entire menu will have to get reworked eventually
template <>
struct MenuCallbackData<float> {
	float *data;
	float old_data;

	const std::string name;
	void *user_data;
	callbackfunc callback;
	// specific for floats
	float min, max;
	// specific for sliders
	const std::string format; // fov = %.3f

	MenuCallbackData(float *data, const std::string &name, void *user_data, callbackfunc callback, float min, float max, const std::string &format)
		: data(data), old_data(*data), name(name), user_data(user_data), callback(callback), min(min), max(max), format(format) {}
	~MenuCallbackData() = default;

	void callbackIfNeeded() {
		if (old_data != *data) {
			old_data = *data;
			callback(user_data, data);
		}
	}
};

// TODO get this out of here
struct InsertInfo {
	const GameObject *obj;
	glm::quat rot;
	glm::vec3 pos;

	InsertInfo(const GameObject *obj, const glm::quat &rot, const glm::vec3 &pos)
		: obj(obj), rot(rot), pos(pos) {}
	~InsertInfo() = default;
};

// TODO many things are not deleted (ex: all related to models)
class Renderer {
public:
	Renderer() = delete;
	Renderer(GLsizei viewport_width, GLsizei viewport_height, PhysRenderer *phys_renderer);
	~Renderer();

	GLsizei viewport_width, viewport_height;

	GLuint materialBuffer, materialTBO;
	GLuint chunkInfoBuffer, chunkInfoTBO;
	GLuint pointLightBuffer, pointLightTBO;
	GLuint dirLightBuffer, dirLightTBO;
	GLuint spotLightBuffer, spotLightTBO;


	GLuint VAO, VAO_axis;
	GLuint indirectBuffer;
	GLuint VBO_base, VBO, vertexBuffer_axis;

	Shader mainShader, axisShader, normalShader;
	GLuint lightingFBO = 0, lightingFBODepthBuffer = 0;
	GLuint lightingTexture = 0; // color atttachment 0, scene renders into this
	GLuint brightTexture = 0; // color atttachment 1, extraction of brightly lit areas

	// ping pong FBOs, to perform two-pass Gaussian blur on the extracted bright colors
	GLuint VAO_viewport;
	GLuint vertexBuffer_viewport;
	GLuint pingpongFBO[2];
	GLuint pingpongTextures[2];
	Shader blurShader;

	// finally, in the normal FBO we merge the resulting blur with the original scene and apply gamma and exposure to everything at once
	// GLuint VAO_viewport;
	// GLuint vertexBuffer_viewport;
	Shader hdrBbloomMergeShader;
	// !! could have reused fbo and textures, but this is simpler and more flexible and less painful to manage

	Shader pointshader;

	// for models
	Shader modelShader, selectedModelShader, modelNormalShader, outlineShader, insertShader;
	GLuint VAO_models;
	GLuint VBO_models;
	GLuint TBO_models_buffer, TBO_models;
	GLuint IBO_models;

	// for skybox
	GLuint VAO_skybox;
	GLuint VBO_skybox;
	Shader skybox_shader;

	std::unique_ptr<TextureArray> textureArray = nullptr; // pointer since it starts as null and gets initialized later. unique_ptr so it always gets deleted

	PhysRenderer *phys_renderer;

	void draw(const glm::mat4 &view, const CustomVec<Vertex> &verts, const CustomVec<Point> &points, const std::vector<IndirectData> &indirect, const std::vector<ChunkInfo> &chunkInfo, const DrawObjects &objs,
		const DrawObjects &selected_objs, const glm::mat4 &projection, GLFWwindow * window, GLfloat deltaTime, Position &pos, Direction &dir, const SelectedBlockInfo &selectedInfo, const Cubemap &skybox); // const
	void postProcess(int bloomBlurPasses);
	void endFrame(GLFWwindow * window);
	void drawSkybox(const Cubemap &skybox, const glm::mat4 &view, const glm::mat4 &projection);
	void drawObjects(const glm::mat4 &view, const glm::mat4 &projection, const DrawObjects &objs);
	void drawSelectedObjects(const glm::mat4 &view, const glm::mat4 &projection, const DrawObjects &objs);
	void drawObjectNormals(const glm::mat4 &view, const glm::mat4 &projection, const DrawObjects &objs);
	void draw_phys(const glm::mat4 &view, const glm::mat4 &projection);
	void drawInsert(const glm::mat4 &view, const glm::mat4 &projection, const InsertInfo &insertInfo, bool valid);

	void loadTextures();
	void resizeViewport(GLsizei viewport_width, GLsizei viewport_height);
	void generate_FBO_depth_buffer(GLuint *depthBuffer) const;
	void generate_FBO_texture(GLuint *textureID, GLenum attachmentID); // makes the texture, needs to be called whenever viewport is resized (for now)
	void checkFrameBuffer();

	// a mess since the args are different
	void addMenuCallbackBool(bool *data, const std::string &name, void *user_data, callbackfunc callback);
	void addMenuCallbackFloat(float *data, const std::string &name, void *user_data, callbackfunc callback,float min, float max, const std::string &format);

private:
	void prepareFrame(GLuint num_verts, Position &pos, Direction &dir, GLfloat deltatime, const SelectedBlockInfo &selectedInfo);
	void drawLighting(const CustomVec<Vertex> &verts, const CustomVec<Point> &points, const std::vector<IndirectData> &indirect, const std::vector<ChunkInfo> &chunkInfo, const glm::mat4 &projection, const glm::mat4 &view); // camera is for debugging
	void drawAxis(const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &projection);
	void drawNormals(const CustomVec<Vertex> &verts, const std::vector<IndirectData> &indirect, const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &projection);
	void bloomBlur(int passes);
	void merge();

	std::vector<MenuCallbackData<bool>> boolMenu;
	std::vector<MenuCallbackData<float>> floatSliderMenu;
};

#endif
