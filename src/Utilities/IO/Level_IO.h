#pragma once
#ifndef	LEVEL_IO_H
#define	LEVEL_IO_H

#include "Modules/ECS/ecsWorld.h"
#include <string>


/** A static helper class used for reading/writing levels. */
class Level_IO {
public:
	/** Check if a given level file-path exists.
	@param	relativePath		the relative path to a level file.
	@return						true if the level file exists, false otherwise. */
	static bool Level_Exists(const std::string& relativePath);
	/** Read a binary level map into the ecsWorld specified.
	@param	relativePath		the relative path to a level file.
	@param	world				the ecsWorld to import into.
	@return						true if the level is successfully imported, false otherwise. */
	static bool Import_BMap(const std::string& relativePath, ecsWorld& world);
	/** Write a binary level map using the ecsWorld specified.
	@param	relativePath		the relative path to a level file.
	@param	world				the ecsWorld to read from.
	@return						true if the level is successfully exported, false otherwise. */
	static bool Export_BMap(const std::string& relativePath, const ecsWorld& world);
};

#endif // LEVEL_IO_H