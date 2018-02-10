#pragma once
#ifndef	IMAGEIMPORTER
#define	IMAGEIMPORTER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLEW_STATIC

#include "GL\glew.h"
#include "GLM\common.hpp"
#include <string>

using namespace std;
using namespace glm;
class FIBITMAP;

/**
 * A static helper class used for importing images.
 * Uses the FreeImage texture importer: http://freeimage.sourceforge.net/
 */
class DT_ENGINE_API ImageImporter 
{
public:
	/** Retrieve an image from disk. 
	 * Reports its errors into the messaging system. Safely fails.
	 * @param	fileName	the string absolute directory to the image file to import
	 * @return	a 32bit FIBITMAP* pointer containing the image if successfull, nullptr otherwise.  
	 * @note	requires manually deleting the FIBITMAP pointer when no longer needed! */
	static FIBITMAP * Import_Image(const std::string& fileName);

	/** Parses the supplied image into a pixel array using the supplied dimensions, interpreted as a mono-chromatic image (all red channel).
	 * @param	bitmap	the bitmap pointer containing the image to parse
	 * @param	dimensions	the dimensions of the image as an (integer) vec2
	 * @return	a pointer to a GLubyte array containing our pixels */
	static GLubyte * Parse_Image_1channel(FIBITMAP * bitmap, const ivec2 & dimensions);

	/** Parses the supplied image into a pixel array using the supplied dimensions, interpreted as a di-chromatic image (red/green).
	 * @param	bitmap	the bitmap pointer containing the image to parse
	 * @param	dimensions	the dimensions of the image as an (integer) vec2
	 * @return	a pointer to a GLubyte array containing our pixels */
	static GLubyte * Parse_Image_2channel(FIBITMAP * bitmap, const ivec2 & dimensions);

	/** Parses the supplied image into a pixel array using the supplied dimensions, interpreted as a tri-chromatic image (red/green/blue). 
	 * @param	bitmap	the bitmap pointer containing the image to parse
	 * @param	dimensions	the dimensions of the image as an (integer) vec2
	 * @return	a pointer to a GLubyte array containing our pixels */
	static GLubyte * Parse_Image_3channel(FIBITMAP * bitmap, const ivec2 & dimensions);

	/** Parses the supplied image into a pixel array using the supplied dimensions, interpreted as a quad-chromatic image (red/green/blue/alpha). 
	 * @param	bitmap	the bitmap pointer containing the image to parse
	 * @param	dimensions	the dimensions of the image as an (integer) vec2
	 * @return	a pointer to a GLubyte array containing our pixels */
	static GLubyte * Parse_Image_4channel(FIBITMAP * bitmap, const ivec2 & dimensions);
};

#endif // IMAGEIMPORTER