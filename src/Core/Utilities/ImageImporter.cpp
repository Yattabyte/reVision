#include "Utilities\ImageImporter.h"
#include "Systems\Message_Manager.h"
#include "FreeImage.h"

FIBITMAP * ImageImporter::FetchImageFromDisk(const std::string &fileName, vec2 & dimensions, int & dataSize, bool & success)
{
	success = true;
	const string &str = fileName;
	const char * file = str.c_str();
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(file, 0);
	GLubyte* textureData = nullptr;

	if (format == -1) {
		MSG::Error(FILE_MISSING, str);
		success = false;
	}
	else if (format == FIF_UNKNOWN) {
		MSG::Error(FILE_CORRUPT, str);
		format = FreeImage_GetFIFFromFilename(file);

		if (!FreeImage_FIFSupportsReading(format)) {
			MSG::Statement("Failure, could not recover the file!");
			success = false;
		}
		else {
			MSG::Statement("Successfully resolved the texture file's format!");
			success = true;
		}
	}
	else if (format == FIF_GIF) {
		MSG::Statement("GIF loading unsupported!");
		success = false;
	}

	if (success)
		return FreeImage_Load(format, file);
	return nullptr;
}

GLubyte * ImageImporter::ReadImage_1channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success)
{
	FIBITMAP *bitmap = FetchImageFromDisk(fileName, dimensions, dataSize, success);
	if (success) {
		FIBITMAP *bitmap32;
		const int bitsPerPixel = FreeImage_GetBPP(bitmap);
		if (bitsPerPixel == 32)
			bitmap32 = bitmap;
		else
			bitmap32 = FreeImage_ConvertTo32Bits(bitmap);

		dimensions = vec2(FreeImage_GetWidth(bitmap32), FreeImage_GetHeight(bitmap32));
		dataSize = 1 * (int)dimensions.x * (int)dimensions.y;
		GLubyte* textureData = new GLubyte[dataSize];
		char* pixels = (char*)FreeImage_GetBits(bitmap32);

		for (int j = 0, total = (int)dimensions.x * (int)dimensions.y; j < total; j++) {
			// format gets read as BGRA
			GLubyte blue = pixels[j * 4 + 0],
				green = pixels[j * 4 + 1],
				red = pixels[j * 4 + 2],
				alpha = pixels[j * 4 + 3];

			// store as RGBA
			textureData[j * 1 + 0] = red;
		}

		//Unload
		FreeImage_Unload(bitmap32);
		if (bitsPerPixel != 32)
			FreeImage_Unload(bitmap);
		return textureData;
	}
	return nullptr;
}

GLubyte * ImageImporter::ReadImage_2channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success)
{
	FIBITMAP *bitmap = FetchImageFromDisk(fileName, dimensions, dataSize, success);
	if (success) {
		FIBITMAP *bitmap32;
		const int bitsPerPixel = FreeImage_GetBPP(bitmap);
		if (bitsPerPixel == 32)
			bitmap32 = bitmap;
		else
			bitmap32 = FreeImage_ConvertTo32Bits(bitmap);

		dimensions = vec2(FreeImage_GetWidth(bitmap32), FreeImage_GetHeight(bitmap32));
		dataSize = 2 * (int)dimensions.x * (int)dimensions.y;
		GLubyte* textureData = new GLubyte[dataSize];
		char* pixels = (char*)FreeImage_GetBits(bitmap32);

		for (int j = 0, total = (int)dimensions.x * (int)dimensions.y; j < total; j++) {
			// format gets read as BGRA
			GLubyte blue = pixels[j * 4 + 0],
				green = pixels[j * 4 + 1],
				red = pixels[j * 4 + 2],
				alpha = pixels[j * 4 + 3];

			// store as RGBA
			textureData[j * 2 + 0] = red;
			textureData[j * 2 + 1] = green;
		}

		//Unload
		FreeImage_Unload(bitmap32);
		if (bitsPerPixel != 32)
			FreeImage_Unload(bitmap);
		return textureData;
	}
	return nullptr;
}

GLubyte * ImageImporter::ReadImage_3channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success)
{
	FIBITMAP *bitmap = FetchImageFromDisk(fileName, dimensions, dataSize, success);
	if (success) {
		FIBITMAP *bitmap32;
		const int bitsPerPixel = FreeImage_GetBPP(bitmap);
		if (bitsPerPixel == 32)
			bitmap32 = bitmap;
		else
			bitmap32 = FreeImage_ConvertTo32Bits(bitmap);

		dimensions = vec2(FreeImage_GetWidth(bitmap32), FreeImage_GetHeight(bitmap32));
		dataSize = 3 * (int)dimensions.x * (int)dimensions.y;
		GLubyte* textureData = new GLubyte[dataSize];
		char* pixels = (char*)FreeImage_GetBits(bitmap32);

		for (int j = 0, total = (int)dimensions.x * (int)dimensions.y; j < total; j++) {
			// format gets read as BGRA
			GLubyte blue = pixels[j * 4 + 0],
				green = pixels[j * 4 + 1],
				red = pixels[j * 4 + 2],
				alpha = pixels[j * 4 + 3];

			// store as RGBA
			textureData[j * 3 + 0] = red;
			textureData[j * 3 + 1] = green;
			textureData[j * 3 + 2] = blue;
		}

		//Unload
		FreeImage_Unload(bitmap32);
		if (bitsPerPixel != 32)
			FreeImage_Unload(bitmap);
		return textureData;
	}
	return nullptr;
}

GLubyte * ImageImporter::ReadImage_4channel(const std::string & fileName, vec2 & dimensions, int & dataSize, bool & success)
{
	FIBITMAP *bitmap = FetchImageFromDisk(fileName, dimensions, dataSize, success);
	if (success) {
		FIBITMAP *bitmap32;
		const int bitsPerPixel = FreeImage_GetBPP(bitmap);
		if (bitsPerPixel == 32)
			bitmap32 = bitmap;
		else
			bitmap32 = FreeImage_ConvertTo32Bits(bitmap);

		dimensions = vec2(FreeImage_GetWidth(bitmap32), FreeImage_GetHeight(bitmap32));
		dataSize = 4 * (int)dimensions.x * (int)dimensions.y;
		GLubyte* textureData = new GLubyte[dataSize];
		char* pixels = (char*)FreeImage_GetBits(bitmap32);

		for (int j = 0, total = (int)dimensions.x * (int)dimensions.y; j < total; j++) {
			// format gets read as BGRA
			GLubyte blue = pixels[j * 4 + 0],
				green = pixels[j * 4 + 1],
				red = pixels[j * 4 + 2],
				alpha = pixels[j * 4 + 3];

			// store as RGBA
			textureData[j * 4 + 0] = red;
			textureData[j * 4 + 1] = green;
			textureData[j * 4 + 2] = blue;
			textureData[j * 4 + 3] = alpha;
		}

		//Unload
		FreeImage_Unload(bitmap32);
		if (bitsPerPixel != 32)
			FreeImage_Unload(bitmap);
		return textureData;
	}
	return nullptr;
}