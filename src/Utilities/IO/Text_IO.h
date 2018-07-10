#pragma once
#ifndef	TEXT_IO_H
#define	TEXT_IO_H
#include "glm\common.hpp"
#include <string>

using namespace std;
class Engine;

/** 
 * A static helper class used for reading/writing text.
 **/
class Text_IO
{
public:
	/** Import a text document from disk.
	 * @param	engine			the engine to import to
	 * @param	fulldirectory	the path to the file
	 * @param	importedData	the container to place the imported data within
	 * @return					true on successfull import, false otherwise (error reported to engine) */
	static bool Import_Text(Engine * engine, const string & fulldirectory, string & importedData);
	static void Export_Text(const string & fulldirectory, const string & exportedData);
};

#endif // TEXT_IO_H