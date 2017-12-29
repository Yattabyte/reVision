/*
	Image Importer
	
	- Uses FreeImage : http://freeimage.sourceforge.net/
	- Provides handy functions for retrieving images from disk
*/

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
class ImageImporter {
public:
	// Attempts to load an image from disk given a @fileName, returning an FIBITMAP pointer.
	// Pointer needs to be unloaded manually.
	DT_ENGINE_API static FIBITMAP * FetchImageFromDisk(const std::string &fileName);
	// Returns image data pointer formatted as GLubyte, arranged as a monoc-hromatic image (all red). 
	DT_ENGINE_API static GLubyte * ParseImage_1channel(FIBITMAP *bitmap, const ivec2 & dimensions);
	// Returns image data pointer formatted as GLubyte, arranged as a di-chromatic image (red/green). 
	DT_ENGINE_API static GLubyte * ParseImage_2channel(FIBITMAP *bitmap, const ivec2 & dimensions);
	// Returns image data pointer formatted as GLubyte, arranged as a tri-chromatic image (red/green/blue). 
	DT_ENGINE_API static GLubyte * ParseImage_3channel(FIBITMAP *bitmap, const ivec2 & dimensions);
	// Returns image data pointer formatted as GLubyte, arranged as a quad-chromatic image (red/green/blue/alpha). 
	DT_ENGINE_API static GLubyte * ParseImage_4channel(FIBITMAP *bitmap, const ivec2 & dimensions);
};
#endif // IMAGEIMPORTER