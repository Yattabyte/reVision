#pragma once
#ifndef	IMAGE_IO_H
#define	IMAGE_IO_H

#include "Utilities/GL/glad/glad.h"
#include "glm/glm.hpp"
#include <string>


class Engine;
struct FIBITMAP;

struct Image_Data {
	GLubyte * pixelData = nullptr;
	glm::ivec2 dimensions = glm::ivec2(0);
	int pitch = 0;
	unsigned int bpp = 0;
};

/** A static helper class used for reading/writing images.
Uses the FreeImage texture importer: http://freeimage.sourceforge.net/ */
class Image_IO {
public:
	/** Initialzie the freeimage library. */
	static void Initialize();
	/** Shutdown the freeimage library*/
	static void Deinitialize();
	/** Import an image from disk.
	@param	engine			the engine to import to
	@param	relativePath	the path to the file
	@param	importedData	the container to place the imported data within
	@param	linear			set to true to use linear filtering, false to use nearest
	@return					true on successfull import, false otherwise (error reported to engine) */
	static bool Import_Image(Engine * engine, const std::string & relativePath, Image_Data & importedData, const bool & linear = true);
	/** Load pixel data from a bitmap object.
	@param	bitmap			the FreeImage bitmap to read from
	@param	importedData	the container to place the imported data within */
	static void Load_Pixel_Data(FIBITMAP * bitmap, Image_Data & importedData);
	/** Resize and update an image.
 	@param	newSize			the desired image size
	@param	importedData	the container holding the image data (gets updated with new data) 
	@param	linear			set to true to use linear filtering, false to use nearest */
	static void Resize_Image(const glm::ivec2 newSize, Image_Data & importedData, const bool & linear = true);
	/** Get the plugin version.
	@return the plugin version */
	static std::string Get_Version();


private:
	/** Import a FreeImage bitmap from disk.
	@param	engine			the engine to import to
	@param	relativePath	the path to the file
	@return					the free image bitmap object */
	static FIBITMAP * Import_Bitmap(Engine * engine, const std::string & relativePath);
};

#endif // IMAGE_IO_H