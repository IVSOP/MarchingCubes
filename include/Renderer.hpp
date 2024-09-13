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

#include "PhysRenderer.hpp"

#include "Phys.hpp" // cursed, to call Phys::buildDebugVerts();

using DrawObjects = std::vector<std::pair<GameObject *, std::vector<glm::mat4>>>;

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
	Shader modelShader, selectedModelShader;
	GLuint VAO_models;
	GLuint VBO_models;
	GLuint TBO_models_buffer, TBO_models;
	GLuint IBO_models;

	std::unique_ptr<TextureArray> textureArray = nullptr; // pointer since it starts as null and gets initialized later. unique_ptr so it always gets deleted

	PhysRenderer *phys_renderer;

	void draw(const glm::mat4 &view, const CustomVec<Vertex> &verts, const CustomVec<Point> &points, const std::vector<IndirectData> &indirect, const std::vector<ChunkInfo> &chunkInfo, const DrawObjects &objs,
		const DrawObjects &selected_objs, const glm::mat4 &projection, GLFWwindow * window, GLfloat deltaTime, Position &pos, Direction &dir, Movement &mov, const SelectedBlockInfo &selectedInfo); // const
	void drawObjects(const glm::mat4 &view, const glm::mat4 &projection, const DrawObjects &objs);
	void drawSelectedObjects(const glm::mat4 &view, const glm::mat4 &projection, const DrawObjects &objs);
	void draw_phys(const glm::mat4 &view, const glm::mat4 &projection);

	void loadTextures();
	void resizeViewport(GLsizei viewport_width, GLsizei viewport_height);
	void generate_FBO_depth_buffer(GLuint *depthBuffer) const;
	void generate_FBO_texture(GLuint *textureID, GLenum attachmentID); // makes the texture, needs to be called whenever viewport is resized (for now)
	void checkFrameBuffer();

private:
	void prepareFrame(GLuint num_verts, Position &pos, Direction &dir, Movement &mov, GLfloat deltatime, const SelectedBlockInfo &selectedInfo);
	void drawLighting(const CustomVec<Vertex> &verts, const CustomVec<Point> &points, const std::vector<IndirectData> &indirect, const std::vector<ChunkInfo> &chunkInfo, const glm::mat4 &projection, const glm::mat4 &view); // camera is for debugging
	void drawAxis(const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &projection);
	void drawNormals(const CustomVec<Vertex> &verts, const std::vector<IndirectData> &indirect, const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &projection);
	void bloomBlur(int passes);
	void merge();
	void endFrame(GLFWwindow * window);
};

#endif
