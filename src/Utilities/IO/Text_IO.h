#pragma once
#ifndef	TEXT_IO_H
#define	TEXT_IO_H

#include "glm/glm.hpp"
#include <ios>
#include <string>


class Engine;

/** A static helper class used for reading/writing text. */
class Text_IO {
public:
	/** Import a text document from disk.
	@param	engine			the engine to import to
	@param	relativePath	the path to the file
	@param	importedData	the container to place the imported data within
	@param	mode			file importing mode
	@return					true on successful import, false otherwise (error reported to engine) */
	static bool Import_Text(Engine& engine, const std::string& relativePath, std::string& importedData, const std::ios_base::openmode& mode = std::ios_base::in) noexcept;
	static void Export_Text(const std::string& relativePath, const std::string& exportedData) noexcept;
};

#endif // TEXT_IO_H
