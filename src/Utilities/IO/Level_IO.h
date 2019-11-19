#pragma once
#ifndef	LEVEL_IO_H
#define	LEVEL_IO_H

#include "Modules/ECS/ecsWorld.h"
#include "glm/glm.hpp"
#include <string>


class Engine;

/** A static helper class used for reading/writing levels. */
class Level_IO {
public:
	/***/
	static bool Level_Exists(const std::string& relativePath) noexcept;
	/***/
	static bool Import_BMap(const std::string& relativePath, ecsWorld& world) noexcept;
	/***/
	static bool Export_BMap(const std::string& relativePath, const ecsWorld& world) noexcept;
};

#endif // LEVEL_IO_H
