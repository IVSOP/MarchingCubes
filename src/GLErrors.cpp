#include "GLErrors.hpp"

#include <signal.h>
#include <iostream>

#include "Logs.hpp"

void GLClearError() {
	while (glGetError());
}

// bool GLLogCall(const char *function, const char *file, int line) {
// 	GLenum error;
// 	while ((error = glGetError())) {
// 		std::cout << "[OpenGL Error] (" << error << "): " << function <<
// 			" " << file << ":" << line << std::endl;
// 		return false;
// 	}
// 	return true;
// }

void checkErrorInShader(GLuint shader) {
	int res;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
	if (!res) {
		int len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		//dynamic allocation on the stack
		//ele nao pôs o +1
		char *message = (char *)alloca((len + 1) * sizeof(char));
		glGetShaderInfoLog(shader, len, &len, message);
		// std::cout << "Shader compilation failed for " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
		print_error("Shader compilation failed"); // can I use %s here???????
		std::cout << message << std::endl;
		glDeleteShader(shader);
		raise(SIGINT);
	}
}

void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	const std::string title = "opengl callback";
    std::string log_message = "message: " + std::string(message) + "\ntype:";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        log_message += "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        log_message += "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        log_message += "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        log_message += "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        log_message += "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        log_message += "OTHER";
        break;
    }

	log_message += "\nid: " + std::to_string(id) + "\nseverity: ";
	LOG_TYPE logtype = LOG_TYPE::INFO;

    switch (severity){
    case GL_DEBUG_SEVERITY_LOW:
        log_message += "LOW";
		logtype = LOG_TYPE::WARN;
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        log_message += "MEDIUM";
		logtype = LOG_TYPE::WARN;
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        log_message += "HIGH";
		logtype = LOG_TYPE::ERR;
        break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		log_message += "NOTIFICATION";
		logtype = LOG_TYPE::INFO;
		break;
    }

	Log::log(logtype, title, log_message);
}

// thse two are basically the same, will remake this
void validateProgram(const GLuint program) {
	GLCall(glValidateProgram(program));
	GLint params = 0;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &params);
	if (params == GL_FALSE) {
		print_error("Program not valid");
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		// GLchar infoLog[maxLength];
		GLchar *infoLog = static_cast<GLchar *>(alloca(maxLength * sizeof(GLchar)));
		infoLog[0] = '\0';
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

		// The program is useless now. So delete it.
		glDeleteProgram(program);

		// got lazy
		print_error(infoLog);

		raise(SIGINT);
	}
}

void checkProgramLinking(const GLuint program) {
	GLCall(glValidateProgram(program));
	GLint params = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &params);
	if (params == GL_FALSE) {
		print_error("Program linking failed");
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		if (maxLength > 0) {

			// The maxLength includes the NULL character???
			// GLchar infoLog[maxLength];
			GLchar *infoLog = static_cast<GLchar *>(alloca((maxLength + 1) * sizeof(GLchar)));
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// got lazy
			print_error(infoLog);
		} else {
			fprintf(stderr, "No error log available\n");
		}
		// The program is useless now. So delete it.
		glDeleteProgram(program);


		raise(SIGINT);
	}
}
