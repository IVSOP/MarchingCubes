#ifndef SETTINGS_H
#define SETTINGS_H

#include "types.hpp"
#include "stdlib.hpp"

// all player settings are set here like global vars
// it makes sense to make them be owned by many classes, so no one ows them
struct Settings {
	static GLdouble fov;
	static GLdouble znear;
	static GLdouble zfar;
	static std::string saves_dir;

	static GLfloat gamma;
	static GLfloat exposure;
	static int bloomBlurPasses;
	static GLfloat bloomThreshold;
	static GLfloat bloomOffset;
	static GLfloat break_radius;
	static GLfloat break_range;

	static bool showAxis;
	static bool showNormals;
	static bool wireframe;
	static bool wireframe_models;
	static bool render_physics;
	static bool render;
	static bool render_models;

	static float raycast_len;

	static void setFov(GLdouble _fov) {
		Settings::fov = _fov;
	}
};


#endif
