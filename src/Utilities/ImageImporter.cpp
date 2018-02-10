#include "Utilities\ImageImporter.h"
#include "Managers\Message_Manager.h"
#include "FreeImage.h"

FIBITMAP * ImageImporter::Import_Image(const std::string & fileName)
{
	const char * file = fileName.c_str();
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(file, 0);
	GLubyte* textureData = nullptr;

	if (format == -1) 
		MSG::Error(FILE_MISSING, fileName);
	else if (format == FIF_UNKNOWN) {
		MSG::Error(FILE_CORRUPT, fileName);
		format = FreeImage_GetFIFFromFilename(file);
		if (!FreeImage_FIFSupportsReading(format)) 
			MSG::Statement("Failure, could not recover the file!");		
		else 
			MSG::Statement("Successfully resolved the texture file's format!");		
	}
	else if (format == FIF_GIF) 
		MSG::Statement("GIF loading unsupported!");
	else {
		FIBITMAP *bitmap = FreeImage_Load(format, file);
		FIBITMAP *bitmap32;
		if (FreeImage_GetBPP(bitmap) == 32)
			return bitmap;
		else {
			bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
			FreeImage_Unload(bitmap);
			return bitmap32;
		}
	}
	return nullptr;
}

GLubyte * ImageImporter::Parse_Image_1channel(FIBITMAP * bitmap, const ivec2 & dimensions)
{
	const unsigned int size_mult = unsigned int(dimensions.x * dimensions.y);
	GLubyte* textureData = new GLubyte[size_mult * 1];

	if (bitmap) {
		char* pixels = (char*)FreeImage_GetBits(bitmap);
		for (unsigned int i = 0; i < size_mult; ++i) {
			// format gets read as BGRA
			const GLubyte &red		= pixels[i * 4 + 2];

			// store as RGBA
			textureData[i * 1 + 0]	= red;
		}
	}

	return textureData;
}

GLubyte * ImageImporter::Parse_Image_2channel(FIBITMAP * bitmap, const ivec2 & dimensions)
{
	const unsigned int size_mult = unsigned int(dimensions.x * dimensions.y);
	GLubyte* textureData = new GLubyte[size_mult * 2];

	if (bitmap) {
		char* pixels = (char*)FreeImage_GetBits(bitmap);
		for (unsigned int i = 0; i < size_mult; ++i) {
			// format gets read as BGRA
			const GLubyte &green	= pixels[i * 4 + 1],
						  &red		= pixels[i * 4 + 2];

			// store as RGBA
			textureData[i * 2 + 0]	= red;
			textureData[i * 2 + 1]	= green;
		}
	}

	return textureData;
}

GLubyte * ImageImporter::Parse_Image_3channel(FIBITMAP * bitmap, const ivec2 & dimensions)
{
	const unsigned int size_mult = unsigned int(dimensions.x * dimensions.y);
	GLubyte* textureData = new GLubyte[size_mult * 3];
	
	if (bitmap) {
		char* pixels = (char*)FreeImage_GetBits(bitmap);
		for (unsigned int i = 0; i < size_mult; ++i) {
			// format gets read as BGRA
			const GLubyte &blue		= pixels[i * 4 + 0],
						  &green	= pixels[i * 4 + 1],
						  &red		= pixels[i * 4 + 2];

			// store as RGBA
			textureData[i * 3 + 0]	= red;
			textureData[i * 3 + 1]	= green;
			textureData[i * 3 + 2]	= blue;
		}
	}

	return textureData;
}

GLubyte * ImageImporter::Parse_Image_4channel(FIBITMAP * bitmap, const ivec2 & dimensions)
{
	const unsigned int size_mult = unsigned int(dimensions.x * dimensions.y);
	GLubyte* textureData = new GLubyte[size_mult * 4];

	if (bitmap) {
		char* pixels = (char*)FreeImage_GetBits(bitmap);
		for (unsigned int i = 0; i < size_mult; ++i) {
			// format gets read as BGRA
			const GLubyte &blue		= pixels[i * 4 + 0],
						  &green	= pixels[i * 4 + 1],
						  &red		= pixels[i * 4 + 2],
						  &alpha	= pixels[i * 4 + 3];

			// store as RGBA
			textureData[i * 4 + 0]	= red;
			textureData[i * 4 + 1]	= green;
			textureData[i * 4 + 2]	= blue;
			textureData[i * 4 + 3]	= alpha;
		}
	}

	return textureData;
}