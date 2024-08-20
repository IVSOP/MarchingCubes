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

	static void setFov(GLdouble _fov) {
		Settings::fov = _fov;
	}
};


#endif
