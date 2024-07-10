#ifndef RENDERER_H
#define RENDERER_H

#include "common.hpp"
#include "Chunk.hpp"
#include "Shader.hpp"
#include "TextureArray.hpp"
#include <memory>
#include "Components.hpp"

#include <vector>

class Renderer {
public:

	Renderer() = delete;
	Renderer(GLsizei viewport_width, GLsizei viewport_height);
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
	

	GLfloat gamma = 2.2f, exposure = 1.0f, bloomThreshold = 1.0f, bloomOffset = 1.0f, explodeCoeff = 0.0f;
	int bloomBlurPasses = 0; //5;
	bool showAxis = true;
	bool showNormals = false;
	bool wireframe = false;
	bool limitFPS = false;
	double fps = 60.0f;
	GLfloat break_radius = 5.0f;
	GLfloat break_range = 10.0f;
	Shader pointshader;

	std::unique_ptr<TextureArray> textureArray = nullptr; // pointer since it starts as null and gets initialized later. unique_ptr so it always gets deleted

	void draw(const glm::mat4 &view, const VertContainer<Vertex> &verts, const VertContainer<Point> &points, const std::vector<IndirectData> &indirect, const std::vector<ChunkInfo> &chunkInfo, const glm::mat4 &projection, GLFWwindow * window, GLfloat deltaTime, Position &pos, Direction &dir, Movement &mov); // const

	void loadTextures();
	void resizeViewport(GLsizei viewport_width, GLsizei viewport_height);
	void generate_FBO_depth_buffer(GLuint *depthBuffer) const;
	void generate_FBO_texture(GLuint *textureID, GLenum attachmentID); // makes the texture, needs to be called whenever viewport is resized (for now)
	void checkFrameBuffer();

private:
	void prepareFrame(GLuint num_verts, Position &pos, Direction &dir, Movement &mov, GLfloat deltatime);
	void drawLighting(const VertContainer<Vertex> &verts, const VertContainer<Point> &points, const std::vector<IndirectData> &indirect, const std::vector<ChunkInfo> &chunkInfo, const glm::mat4 &projection, const glm::mat4 &view); // camera is for debugging
	void drawAxis(const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &projection);
	void drawNormals(const VertContainer<Vertex> &verts, const std::vector<IndirectData> &indirect, const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &projection);
	void bloomBlur(int passes);
	void merge();
	void endFrame(GLFWwindow * window);
};

#endif
