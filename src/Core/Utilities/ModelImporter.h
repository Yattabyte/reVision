/*
	Model Importer
	
	- Uses ASSIMP :  http://assimp.sourceforge.net/
	- Provides handy functions for retrieving models from disk
*/

#pragma once
#ifndef	MODELIMPORTER
#define	MODELIMPORTER
#ifdef	ENGINE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "btBulletCollisionCommon.h"
#include "glm\common.hpp"
#include <string>
#include <vector>

using namespace std;
using namespace glm;

class ModelImporter {
public:
	// Reads the model from disk and returns its vertices into btScalar vector @points.
	// Returns 1 if successfull, 0 if file doesn't exist, and -1 if the file exists but is unreadable.
	DELTA_CORE_API static int Import_Model(const string &fulldirectory, unsigned int pFlags, vector<btScalar> &points);
	// Reads the model from disk and returns its vertices and uvcoords into @vertices and @uv_coords.
	// Returns 1 if successfull, 0 if file doesn't exist, and -1 if the file exists but is unreadable.
	DELTA_CORE_API static int Import_Model(const string &fulldirectory, unsigned int pFlags, vector<vec3> &vertices, vector<vec2> &uv_coords);
};
#endif // MODELIMPORTER