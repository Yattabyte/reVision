/*
	dt_FreeImage : http://freeimage.sourceforge.net/

	- A plugin which uses FreeImage for loading images from disk
*/

#pragma once
#ifndef	DT_FREEIMAGE
#define	DT_FREEIMAGE
#ifdef	DT_FREEIMAGE_EXPORT
#define DELTA_FREEIMAGE_API __declspec(dllexport)
#else
#define	DELTA_FREEIMAGE_API __declspec(dllimport)
#endif

#define GLEW_STATIC
#include "GL\glew.h"
#include "GLM\common.hpp"
#include "Assets\Asset_Material.h"
#include "Assets\Asset_Texture.h"
#include "Managers\Asset_Manager.h"

using namespace std;
using namespace glm;

class FIBITMAP;
namespace dt_FreeImage {
	// Initializes the freeimage plugin and any systems it needs
	// Returns true if it successfull, false otherwise. Reports its own errors to the console.
	DELTA_FREEIMAGE_API bool Initialize();
	// Attempts to load an image from disk given a @fileName, returning an FIBITMAP pointer.
	// Pointer needs to be deleted manually.
	// Updates @dimensions, @dataSize, and @success.
	DELTA_FREEIMAGE_API FIBITMAP * FetchImageFromDisk(const std::string &fileName, vec2 & dimensions, int & dataSize, bool & success);
	// Returns image data pointer formatted as GLubyte, arranged as a monoc-hromatic image (all red). 
	// Pointer needs to be deleted manually.
	// Updates @dimensions, @dataSize, and @success.
	DELTA_FREEIMAGE_API GLubyte * ReadImage_1channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success);
	// Returns image data pointer formatted as GLubyte, arranged as a di-chromatic image (red/green). 
	// Pointer needs to be deleted manually.
	// Updates @dimensions, @dataSize, and @success.
	DELTA_FREEIMAGE_API GLubyte * ReadImage_2channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success);
	// Returns image data pointer formatted as GLubyte, arranged as a tri-chromatic image (red/green/blue). 
	// Pointer needs to be deleted manually.
	// Updates @dimensions, @dataSize, and @success.
	DELTA_FREEIMAGE_API GLubyte * ReadImage_3channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success);
	// Returns image data pointer formatted as GLubyte, arranged as a quad-chromatic image (red/green/blue/alpha). 
	// Pointer needs to be deleted manually.
	// Updates @dimensions, @dataSize, and @success.
	DELTA_FREEIMAGE_API GLubyte * ReadImage_4channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success);
}
namespace Asset_Manager {
	DELTA_FREEIMAGE_API void load_asset(Shared_Asset_Material &user, const std::string(&textures)[6], const bool &threaded = true);
	DELTA_FREEIMAGE_API void load_asset(Shared_Asset_Material &user, const std::string &material_filename, const bool &threaded = true);
	DELTA_FREEIMAGE_API void load_asset(Shared_Asset_Texture &user, const string & filename, const bool &mipmap = false, const bool &anis = false, const bool &threaded = true);
}
#endif // DT_FREEIMAGE_API