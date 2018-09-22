#pragma once
#ifndef LEVEL_IO_H
#define LEVEL_IO_H

#include "Assets\Asset_Level.h"
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>


class Engine;
/** A static helper class used for reading/writing level files.
Supports only our own level format so far. */
class Level_IO {
public:
	/***/
	static bool Import_Level(Engine * engine, const std::string & relativePath, std::vector<LevelStruct_Entity> & entities);


private:
	// Private Methods
	/** Parse the next series of lines into a level containing entities and components.
	@param	file_stream		the active file stream
	@return					a level */
	static std::vector<LevelStruct_Entity> parse_level(std::ifstream & file_stream);
	/** Parse the next series of lines into an entity containing components.
	@param	file_stream		the active file stream
	@return					an entity */
	static LevelStruct_Entity parse_entity(std::ifstream & file_stream);
	/** Parse the next series of lines into a series of component parameters/attributes.
	@param	file_stream		the active file stream
	@return					a component with parameters/attributes */
	static LevelStruct_Component parse_component(std::ifstream & file_stream);
	/** Attempts to retrieve a std::string between quotation marks "<std::string>"
	@return	the std::string between quotation marks */
	static std::string const get_between_quotes(std::string & s);
	/** Search a given string and return whether or not it contains the desired string.
	@param		s1	the string to search within
	@param		s2	the target string to find
	@return		true if the second string is found in the first, else otherwise. */
	static bool const find(const std::string & s1, const std::string & s2);
	/** Parse a given line between parantheses and convert it to a string.
	@param	in	the string to convert
	@return		a string */
	static std::string const getType_String(std::string & in);
	/** Parse a given line between parantheses and convert it to an unsigned int.
	@param	in	the string to convert
	@return		an unsigned int */
	static unsigned int const getType_UInt(std::string & in);
	/** Parse a given line between parantheses and convert it to an int.
	@param	in	the string to convert
	@return		an int */
	static int const getType_Int(std::string & in);
	/** Parse a given line between parantheses and convert it to a double.
	@param	in	the string to convert
	@return		a double */
	static double const getType_Double(std::string & in);
	/** Parse a given line between parantheses and convert it to a float.
	@param	in	the string to convert
	@return		a float */
	static float const getType_Float(std::string & in);
	/** Parse a given line between parantheses and convert it to a vec2.
	@param	in	the string to convert
	@return		a vec2 */
	static glm::vec2 const getType_Vec2(std::string & in);
	/** Parse a given line between parantheses and convert it to a vec3.
	@param	in	the string to convert
	@return		a vec3 */
	static glm::vec3 const getType_Vec3(std::string & in);
	/** Parse a given line between parantheses and convert it to a vec4.
	@param	in	the string to convert
	@return		a vec4 */
	static glm::vec4 const getType_Vec4(std::string & in);
	/** Parse a given line between parantheses and convert it to a quat.
	@param	in	the string to convert
	@return		a quat */
	static glm::quat const getType_Quat(std::string & in);
};

#endif // LEVEL_IO_H