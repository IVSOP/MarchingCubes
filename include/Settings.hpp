#ifndef SETTINGS_H
#define SETTINGS_H

#include "types.hpp"
#include "stdlib.hpp"

// all player settings are set here like global vars
// it makes sense to make them be owned by many classes, so no one ows them
struct Settings {
	// camera
	static GLdouble fov;
	static GLdouble znear;
	static GLdouble zfar;

	static std::string saves_dir;

	// rendering parameters
	static GLfloat gamma;
	static GLfloat exposure;
	static int bloomBlurPasses;
	static GLfloat bloomThreshold;
	static GLfloat bloomOffset;

	// rendering options
	static bool showAxis;
	static bool showNormals;
	static bool wireframe;
	static bool wireframe_models;
	static bool render_physics;
	static bool render;
	static bool render_models;
	static bool showModelNormals;

	// breaking
	static GLfloat break_radius;
	static GLfloat break_range;

	// entity selection
	static float raycast_len;
	static bool select;

	// inserting entities
	// break_range is used by this too
	static bool insert;

	static bool edit_terrain;

	// fps
	static bool limitFPS;
	static float fps;

	static void setFov(GLdouble _fov) {
		Settings::fov = _fov;
	}
};


#endif
