#pragma once
#ifndef	IMAGE_IO_H
#define	IMAGE_IO_H

#include <glad/glad.h>
#include "glm/glm.hpp"
#include <string>
#include <vector>


class Engine;
struct FIBITMAP;

struct Image_Data {
	std::vector<GLubyte> pixelData;
	glm::ivec2 dimensions = glm::ivec2(0);
	int pitch = 0;
	unsigned int bpp = 0;
};

enum class Fill_Policy {
	CHECKERED,
	SOLID,
};

enum class Resize_Policy {
	NEAREST,
	LINEAR,
};

/** A static helper class used for reading/writing images.
Uses the FreeImage texture importer: http://freeimage.sourceforge.net/ */
class Image_IO {
public:
	/** Initialize the FreeImage library. */
	static void Initialize() noexcept;
	/** Shutdown the FreeImage library. */
	static void Deinitialize() noexcept;
	/** Import an image from disk.
	@param	engine			reference to the engine to use. 
	@param	relativePath	the path to the file.
	@param	importedData	the container to place the imported data within.
	@param	resizePolicy	the resize policy to use, such as nearest neighbor or linear interpolation.
	@return					true on successful import, false otherwise (error reported to engine). */
	static bool Import_Image(Engine& engine, const std::string& relativePath, Image_Data& importedData, const Resize_Policy& resizePolicy = Resize_Policy::LINEAR) noexcept;
	/** Load pixel data from a bitmap object.
	@param	bitmap			the FreeImage bitmap to read from.
	@param	importedData	the container to place the imported data within. */
	static void Load_Pixel_Data(FIBITMAP* bitmap, Image_Data& importedData) noexcept;
	/** Resize and update an image.
	@param	newSize			the desired image size.
	@param	importedData	the container holding the image data (gets updated with new data).
	@param	resizePolicy	the resize policy to use, such as nearest neighbor or linear interpolation. */
	static void Resize_Image(const glm::ivec2 newSize, Image_Data& importedData, const Resize_Policy& resizePolicy = Resize_Policy::LINEAR) noexcept;
	/** Retrieve the plugin version.
	@return					the plugin version. */
	static std::string Get_Version() noexcept;


private:
	/** Import a FreeImage bitmap from disk.
	@param	engine			reference to the engine to use. 
	@param	relativePath	the path to the file.
	@return					the free image bitmap object. */
	static FIBITMAP* Import_Bitmap(Engine& engine, const std::string& relativePath) noexcept;
};

#endif // IMAGE_IO_H