#pragma once
#ifndef	IMAGEIMPORTER
#define	IMAGEIMPORTER
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "GL\glew.h"
#include "GLM\common.hpp"
#include <string>

using namespace std;
using namespace glm;
class FIBITMAP;


/**
 * A static helper class used for importing images.
 * Uses the FreeImage texture importer: http://freeimage.sourceforge.net/
 **/
class DT_ENGINE_API Image_Importer 
{
public:
	/** Retrieve an image from disk. 
	 * Reports its errors into the messaging system. Safely fails.
	 * @param	fileName	the string absolute directory to the image file to import
	 * @return				a 32bit FIBITMAP* pointer containing the image if successfull, nullptr otherwise.  
	 * @note				requires manually deleting the FIBITMAP pointer when no longer needed! */
	static FIBITMAP * import_Image(const std::string& fileName);
	/** Parses the supplied image into a pixel array using the supplied dimensions, interpreted as a mono-chromatic image (all red channel).
	 * @param	bitmap		the bitmap pointer containing the image to parse
	 * @param	dimensions	the dimensions of the image as an (integer) vec2
	 * @return				a pointer to a GLubyte array containing our pixels */
	static GLubyte * parse_Image_1_channel(FIBITMAP * bitmap, const ivec2 & dimensions);
	/** Parses the supplied image into a pixel array using the supplied dimensions, interpreted as a di-chromatic image (red/green).
	 * @param	bitmap		the bitmap pointer containing the image to parse
	 * @param	dimensions	the dimensions of the image as an (integer) vec2
	 * @return				a pointer to a GLubyte array containing our pixels */
	static GLubyte * parse_Image_2_channel(FIBITMAP * bitmap, const ivec2 & dimensions);
	/** Parses the supplied image into a pixel array using the supplied dimensions, interpreted as a tri-chromatic image (red/green/blue). 
	 * @param	bitmap		the bitmap pointer containing the image to parse
	 * @param	dimensions	the dimensions of the image as an (integer) vec2
	 * @return				a pointer to a GLubyte array containing our pixels */
	static GLubyte * parse_Image_3_channel(FIBITMAP * bitmap, const ivec2 & dimensions);
	/** Parses the supplied image into a pixel array using the supplied dimensions, interpreted as a quad-chromatic image (red/green/blue/alpha). 
	 * @param	bitmap		the bitmap pointer containing the image to parse
	 * @param	dimensions	the dimensions of the image as an (integer) vec2
	 * @return				a pointer to a GLubyte array containing our pixels */
	static GLubyte * parse_Image_4_channel(FIBITMAP * bitmap, const ivec2 & dimensions);
};

#endif // IMAGEIMPORTER