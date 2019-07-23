#pragma once
#ifndef LEVEL_IO_H
#define LEVEL_IO_H

#include "Assets/Level.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>


class Engine;

/** A static helper class used for reading/writing level files.
Supports only our own level format so far. */
class Level_IO {
public:
	/** Import a level from disk.
	@param	engine			the engine to import to
	@param	relativePath	the path to the file
	@param	entities		the container to place the imported level entity descriptions
	@return					true on successfull import, false otherwise (error reported to engine) */
	static bool Import_Level(Engine * engine, const std::string & relativePath, std::vector<LevelStruct_Entity> & entities);


private:
	// Private Methods
	/***/
	static std::vector<LevelStruct_Entity> parse_level(const std::vector<char> & ecsData);	
};

#endif // LEVEL_IO_H