#pragma once
#ifndef	IMAGE_IO_H
#define	IMAGE_IO_H
#include "glm\common.hpp"
#include "GL/glew.h"
#include <string>


class Engine;
class FIBITMAP;

struct Image_Data {
	GLubyte * pixelData = nullptr;
	glm::ivec2 dimensions = glm::ivec2(0);
	int pitch = 0;
	unsigned int bpp = 0;
};

/** 
 * A static helper class used for reading/writing images.
 * Uses the FreeImage texture importer: http://freeimage.sourceforge.net/
 **/
class Image_IO
{
public:
	/** Import an image from disk.
	 * @param	engine			the engine to import to
	 * @param	fulldirectory	the path to the file
	 * @param	importedData	the container to place the imported data within
	 * @return					true on successfull import, false otherwise (error reported to engine) */
	static bool Import_Image(Engine * engine, const std::string & fulldirectory, Image_Data & importedData);
	/** Load pixel data from a bitmap object.
	 * @param	bitmap			the FreeImage bitmap to read from
	 * @param	importedData	the container to place the imported data within */
	static void Load_Pixel_Data(FIBITMAP * bitmap, Image_Data & importedData);
	/** Resize and update an image.
 	 * @param	newSize			the desired image size
	 * @param	importedData	the container holding the image data (gets updated with new data) */
	static void Resize_Image(const glm::ivec2 newSize, Image_Data & importedData);
	/** Get the plugin version.
	 * @return the plugin version */
	static const std::string Get_Version();


private:
	/** Import a FreeImage bitmap from disk.
	 * @param	engine			the engine to import to
	 * @param	fulldirectory	the path to the file
	 * @return					the free image bitmap object */
	static FIBITMAP * Import_Bitmap(Engine * engine, const std::string & fulldirectory);
};

#endif // IMAGE_IO_H