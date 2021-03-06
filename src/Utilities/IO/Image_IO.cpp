#include "Utilities/IO/Image_IO.h"
#include "Engine.h"
#include "FreeImagePlus.h"


inline auto RGBA_to_BGRA(const GLubyte* pixels, const unsigned int& size)
{
	std::vector<GLubyte> newPixels(size_t(size) * 4ULL);
	for (size_t x = 0; x < size; ++x) {
		newPixels[x * 4 + 0] = pixels[x * 4 + 2];
		newPixels[x * 4 + 1] = pixels[x * 4 + 1];
		newPixels[x * 4 + 2] = pixels[x * 4 + 0];
		newPixels[x * 4 + 3] = pixels[x * 4 + 3];
	}
	return newPixels;
}

FIBITMAP* Image_IO::Import_Bitmap(Engine& engine, const std::string& relativePath)
{
	FIBITMAP* bitmap = nullptr;
	if (!relativePath.empty()) {
		std::string fullPath(Engine::Get_Current_Dir() + relativePath);
		const char* file = fullPath.c_str();
		FREE_IMAGE_FORMAT format = FreeImage_GetFileType(file, 0);

		auto& messageManager = engine.getManager_Messages();
		if (format == -1)
			messageManager.error("The file \"" + relativePath + "\" does not exist.");
		else if (format == FIF_UNKNOWN) {
			messageManager.error("The file \"" + relativePath + "\" exists, but is corrupted. Attempting to recover...");
			format = FreeImage_GetFIFFromFilename(file);
			if (FreeImage_FIFSupportsReading(format) == 0)
				messageManager.warning("Failed to recover the file \"" + relativePath + ".");
		}
		else if (format == FIF_GIF)
			messageManager.warning("GIF loading unsupported!");
		else {
			bitmap = FreeImage_Load(format, file);
			if (FreeImage_GetBPP(bitmap) != 32) {
				FIBITMAP* temp = FreeImage_ConvertTo32Bits(bitmap);
				FreeImage_Unload(bitmap);
				bitmap = temp;
			}
		}
	}
	return bitmap;
}

void Image_IO::Initialize() noexcept
{
	FreeImage_Initialise();
}

void Image_IO::Deinitialize() noexcept
{
	FreeImage_DeInitialise();
}

bool Image_IO::Import_Image(Engine& engine, const std::string& relativePath, Image_Data& importedData, const Resize_Policy& resizePolicy)
{
	const glm::ivec2 containerSize = importedData.dimensions;
	FIBITMAP* bitmap = Import_Bitmap(engine, relativePath);
	if (bitmap == nullptr) return false;
	Load_Pixel_Data(bitmap, importedData);
	FreeImage_Unload(bitmap);
	// If the image container already has a determined size, resize this new image to fit it (if it is a different size)
	if (containerSize != glm::ivec2(0) && containerSize != importedData.dimensions)
		Resize_Image(containerSize, importedData, resizePolicy);
	return true;
}

void Image_IO::Load_Pixel_Data(FIBITMAP* bitmap, Image_Data& importedData)
{
	const glm::ivec2 dimensions(FreeImage_GetWidth(bitmap), FreeImage_GetHeight(bitmap));
	const size_t size_mult = static_cast<size_t>(dimensions.x) * static_cast<size_t>(dimensions.y);

	// Always create RGBA format
	importedData.pixelData.resize(size_mult * 4ULL);
	const GLubyte* pixels = static_cast<GLubyte*>(FreeImage_GetBits(bitmap));

	for (size_t i = 0; i < size_mult; ++i) {
		importedData.pixelData[i * 4 + 2] = pixels[i * 4 + 0];
		importedData.pixelData[i * 4 + 1] = pixels[i * 4 + 1];
		importedData.pixelData[i * 4 + 0] = pixels[i * 4 + 2];
		importedData.pixelData[i * 4 + 3] = pixels[i * 4 + 3];
	}

	importedData.dimensions = dimensions;
	importedData.pitch = static_cast<int>(FreeImage_GetPitch(bitmap));
	importedData.bpp = FreeImage_GetBPP(bitmap);
}

void Image_IO::Resize_Image(const glm::ivec2 newSize, Image_Data& importedData, const Resize_Policy& resizePolicy)
{
	// Make sure new sizes AREN'T zero
	if ((newSize.x != 0) && (newSize.y != 0) && (importedData.dimensions.x != 0) && (importedData.dimensions.y != 0))
		// Proceed if dimensions aren't the same
		if (newSize != importedData.dimensions) {
			// Create FreeImage bitmap from data provided
			auto BGRA_Pixels = RGBA_to_BGRA(&importedData.pixelData[0], importedData.dimensions.x * importedData.dimensions.y);
			importedData.pixelData.clear();
			FIBITMAP* bitmap = FreeImage_ConvertFromRawBits(BGRA_Pixels.data(), importedData.dimensions.x, importedData.dimensions.y, importedData.pitch, importedData.bpp, 0, 0, 0);
			BGRA_Pixels.clear();
			BGRA_Pixels.shrink_to_fit();

			// Resize the bitmap
			FIBITMAP* newBitmap = FreeImage_Rescale(bitmap, newSize.x, newSize.y, resizePolicy == Resize_Policy::LINEAR ? FREE_IMAGE_FILTER::FILTER_CATMULLROM : FREE_IMAGE_FILTER::FILTER_BOX);
			Load_Pixel_Data(newBitmap, importedData);

			// Free Resources
			FreeImage_Unload(newBitmap);
			FreeImage_Unload(bitmap);
		}
}

std::string Image_IO::Get_Version()
{
	return std::string(FreeImage_GetVersion());
}