/*
	Image Importer
	
	- Uses FreeImage : http://freeimage.sourceforge.net/
	- Provides handy functions for retrieving images from disk
*/

#pragma once
#ifndef	IMAGEIMPORTER
#define	IMAGEIMPORTER
#ifdef	ENGINE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif
#define GLEW_STATIC

#include "GL\glew.h"
#include "GLM\common.hpp"
#include <string>

using namespace std;
using namespace glm;

class FIBITMAP;
class ImageImporter {
public:
	// Attempts to load an image from disk given a @fileName, returning an FIBITMAP pointer.
	// Pointer needs to be deleted manually.
	// Updates @dimensions, @dataSize, and @success.
	DELTA_CORE_API static FIBITMAP * FetchImageFromDisk(const std::string &fileName, vec2 & dimensions, int & dataSize, bool & success);
	// Returns image data pointer formatted as GLubyte, arranged as a monoc-hromatic image (all red). 
	// Pointer needs to be deleted manually.
	// Updates @dimensions, @dataSize, and @success.
	DELTA_CORE_API static GLubyte * ReadImage_1channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success);
	// Returns image data pointer formatted as GLubyte, arranged as a di-chromatic image (red/green). 
	// Pointer needs to be deleted manually.
	// Updates @dimensions, @dataSize, and @success.
	DELTA_CORE_API static GLubyte * ReadImage_2channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success);
	// Returns image data pointer formatted as GLubyte, arranged as a tri-chromatic image (red/green/blue). 
	// Pointer needs to be deleted manually.
	// Updates @dimensions, @dataSize, and @success.
	DELTA_CORE_API static GLubyte * ReadImage_3channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success);
	// Returns image data pointer formatted as GLubyte, arranged as a quad-chromatic image (red/green/blue/alpha). 
	// Pointer needs to be deleted manually.
	// Updates @dimensions, @dataSize, and @success.
	DELTA_CORE_API static GLubyte * ReadImage_4channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success);
};
#endif // IMAGEIMPORTER