#include "Utilities/IO/Image_IO.h"
#include "Engine.h"
#include "FreeImage.h"


GLubyte * RGBA_to_BGRA(const GLubyte * pixels, const unsigned int & size) 
{
	GLubyte * newPixels = new GLubyte[size * 4];
	for (int x = 0; x < size; ++x) {
		newPixels[x * 4 + 0] = pixels[x * 4 + 2];
		newPixels[x * 4 + 1] = pixels[x * 4 + 1];
		newPixels[x * 4 + 2] = pixels[x * 4 + 0];
		newPixels[x * 4 + 3] = pixels[x * 4 + 3];
	}
	return newPixels;
}

FIBITMAP * Image_IO::Import_Bitmap(Engine * engine, const std::string & fulldirectory)
{
	const char * file = fulldirectory.c_str();
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(file, 0);
	FIBITMAP * bitmap = nullptr;

	if (format == -1)
		engine->reportError(MessageManager::FILE_MISSING, fulldirectory);
	else if (format == FIF_UNKNOWN) {
		engine->reportError(MessageManager::FILE_CORRUPT, fulldirectory);
		format = FreeImage_GetFIFFromFilename(file);
		if (FreeImage_FIFSupportsReading(format))
			engine->reportMessage("Successfully resolved the texture file's format!");
		else
			engine->reportMessage("Failure, could not recover the file!");
	}
	else if (format == FIF_GIF)
		engine->reportMessage("GIF loading unsupported!");
	else {
		bitmap = FreeImage_Load(format, file);
		if (FreeImage_GetBPP(bitmap) != 32) {
			FIBITMAP * temp = FreeImage_ConvertTo32Bits(bitmap);
			FreeImage_Unload(bitmap);
			bitmap = temp;
		}
	}
	return bitmap;
}

void Image_IO::Initialize()
{
	FreeImage_Initialise();
}

void Image_IO::Deinitialize()
{
	FreeImage_DeInitialise();
}

bool Image_IO::Import_Image(Engine * engine, const std::string & fulldirectory, Image_Data & data_container)
{
	FIBITMAP * bitmap = Import_Bitmap(engine, fulldirectory);
	if (!bitmap) return false;
	Load_Pixel_Data(bitmap, data_container);
	FreeImage_Unload(bitmap);
	return true;
}

void Image_IO::Load_Pixel_Data(FIBITMAP * bitmap, Image_Data & data_container)
{
	const glm::ivec2 dimensions(FreeImage_GetWidth(bitmap), FreeImage_GetHeight(bitmap));
	const unsigned int size_mult = unsigned int(dimensions.x * dimensions.y);

	// Always create rgba format
	GLubyte * textureData = new GLubyte[size_mult * 4];
	const GLubyte * pixels = (GLubyte*)FreeImage_GetBits(bitmap);

	for (unsigned int i = 0; i < size_mult; ++i) {
		textureData[i * 4 + 2] = pixels[i * 4 + 0];
		textureData[i * 4 + 1] = pixels[i * 4 + 1];
		textureData[i * 4 + 0] = pixels[i * 4 + 2];
		textureData[i * 4 + 3] = pixels[i * 4 + 3];
	}

	data_container.dimensions = dimensions;
	data_container.pixelData = textureData;
	data_container.pitch = FreeImage_GetPitch(bitmap);
	data_container.bpp = FreeImage_GetBPP(bitmap);
}

void Image_IO::Resize_Image(const glm::ivec2 newSize, Image_Data & importedData)
{
	// Make sure new sizes AREN'T zero
	if (newSize.x && newSize.y && importedData.dimensions.x && importedData.dimensions.y) 
		// Proceed if dimensions aren't the same
		if (newSize != importedData.dimensions)	{
			// Create freeimage bitmap from data provided
			GLubyte * BGRA_Pixels = RGBA_to_BGRA(importedData.pixelData, importedData.dimensions.x * importedData.dimensions.y);
			delete importedData.pixelData;
			FIBITMAP * bitmap = FreeImage_ConvertFromRawBits(BGRA_Pixels, importedData.dimensions.x, importedData.dimensions.y, importedData.pitch, importedData.bpp, 0, 0, 0);
			delete BGRA_Pixels;
		
			// Resize the bitmap
			FIBITMAP * newBitmap = FreeImage_Rescale(bitmap, newSize.x, newSize.y);
			Load_Pixel_Data(newBitmap, importedData);

			// Free Resources	
			FreeImage_Unload(newBitmap);
			FreeImage_Unload(bitmap);
		}
}

const std::string Image_IO::Get_Version()
{
	return std::string(FreeImage_GetVersion());
}
