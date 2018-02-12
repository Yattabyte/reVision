#pragma once
#ifndef	MODELIMPORTER
#define	MODELIMPORTER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "btBulletCollisionCommon.h"
#include "glm\common.hpp"
#include <string>
#include <vector>

using namespace std;
using namespace glm;


/** 
 * A static helper class used for importing models.
 * Uses the Assimp model importer: http://assimp.sourceforge.net/
 **/
class DT_ENGINE_API Model_Importer 
{
public:
	/** Reads the model from disk and retrieves its vertices.
	 * @param	fulldirectory	a string containing the absolute directory of the model to read from
	 * @param	pFlags	integer flags for controlling the ASSIMP importing process
	 * @param	points	a reference to a btScalar vector that will hold the vertices
	 * @return	1 if successfull, 0 if file doesn't exist, and -1 if the file is corrupt */
	static int import_Model(const string & fulldirectory, unsigned int pFlags, vector<btScalar> & points);

	/** Reads the model from disk and retrieves its vertices and UV coordinates.
	 * @param	fulldirectory	a string containing the absolute directory of the model to read from
	 * @param	pFlags	integer flags for controlling the ASSIMP importing process
	 * @param	vertices	a reference to a btScalar vector that will hold the vertices
	 * @param	uv_coords	a reference to a vec2 vector that will hold the UV coordinates
	 * @return	1 if successfull, 0 if file doesn't exist, and -1 if the file is corrupt */
	static int import_Model(const string & fulldirectory, unsigned int pFlags, vector<vec3> & vertices, vector<vec2> & uv_coords);
};

#endif // MODELIMPORTER