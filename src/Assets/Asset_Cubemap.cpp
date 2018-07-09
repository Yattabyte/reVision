#include "Assets\Asset_Cubemap.h"
#include "Engine.h"
#include "FreeImage.h"
#define EXT_CUBEMAP ".png"
#define DIRECTORY_CUBEMAP Engine::Get_Current_Dir() + "\\Textures\\Cubemaps\\"
#define ABS_DIRECTORY_CUBEMAP(filename) DIRECTORY_CUBEMAP + filename


Asset_Cubemap::~Asset_Cubemap()
{
	if (existsYet())
		glDeleteTextures(1, &m_glTexID);
}

Asset_Cubemap::Asset_Cubemap(const std::string & filename) : Asset(filename)
{
	m_glTexID = 0;
	m_size = vec2(0);
}

void Asset_Cubemap::CreateDefault(Engine * engine, Shared_Asset_Cubemap & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, "defaultCubemap"))
		return;
	
	// Create hard-coded alternative
	assetManager.createNewAsset(userAsset, "defaultCubemap");
	userAsset->m_pixelData[0] = new GLubyte[4]{ GLubyte(255), GLubyte(0), GLubyte(0), GLubyte(255) };
	userAsset->m_pixelData[1] = new GLubyte[4]{ GLubyte(0), GLubyte(255), GLubyte(0), GLubyte(255) };
	userAsset->m_pixelData[2] = new GLubyte[4]{ GLubyte(0), GLubyte(0), GLubyte(255), GLubyte(255) };
	userAsset->m_pixelData[3] = new GLubyte[4]{ GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(255) };
	userAsset->m_pixelData[4] = new GLubyte[4]{ GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(255) };
	userAsset->m_pixelData[5] = new GLubyte[4]{ GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255) };
	userAsset->m_size = vec2(1);

	// Create the asset
	assetManager.submitNewWorkOrder(userAsset, true,
		/* Initialization. */
		[]() {},
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); }
	);
}

void Asset_Cubemap::Create(Engine * engine, Shared_Asset_Cubemap & userAsset, const string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const string &fullDirectory = DIRECTORY_CUBEMAP + filename;
	if (!Engine::File_Exists(fullDirectory)) {
		engine->reportError(MessageManager::FILE_MISSING, fullDirectory);
		CreateDefault(engine, userAsset);
		return;
	}
	
	// Create the asset
	assetManager.submitNewAsset(userAsset, threaded,
		/* Initialization. */
		[engine, &userAsset, fullDirectory]() mutable { Initialize(engine, userAsset, fullDirectory); },
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); },
		/* Constructor Arguments. */
		filename
	);
}

void Asset_Cubemap::Initialize(Engine * engine, Shared_Asset_Cubemap & userAsset, const string & fullDirectory)
{
	static const std::string side_suffixes[6] = { "right", "left", "bottom", "top", "front", "back" };
	static const std::string extensions[3] = { ".png", ".jpg", ".tga" };
	for (int sides = 0; sides < 6; ++sides) {
		std::string specific_side_directory;
		const char * file;
		FREE_IMAGE_FORMAT format;

		for (int x = 0; x < 3; ++x) {
			specific_side_directory = fullDirectory + side_suffixes[sides] + extensions[x];
			file = specific_side_directory.c_str();
			format = FreeImage_GetFileType(file, 0);
			if (format != -1)
				break;
		}

		for (int x = 0; x < 3; ++x) {
			specific_side_directory = fullDirectory + "\\" + side_suffixes[sides] + extensions[x];
			file = specific_side_directory.c_str();
			format = FreeImage_GetFileType(file, 0);
			if (format != -1)
				break;
		}

		if (format == -1) {
			engine->reportError(MessageManager::FILE_MISSING, fullDirectory);
			CreateDefault(engine, userAsset);
			return;
		}

		if (format == FIF_UNKNOWN) {
			format = FreeImage_GetFIFFromFilename(file);
			if (!FreeImage_FIFSupportsReading(format)) { // Attempt to resolve texture file format
				engine->reportError(MessageManager::FILE_CORRUPT, fullDirectory, "Using default texture.");
				CreateDefault(engine, userAsset);
				return;
			}
		}

		FIBITMAP* bitmap, *bitmap32;
		FIMULTIBITMAP* mbitmap;

		//Load
		if (format == FIF_GIF) {
			mbitmap = FreeImage_OpenMultiBitmap(FIF_GIF, file, false, true, false, GIF_PLAYBACK);
			engine->reportMessage("GIF loading unsupported, using first frame...");
			bitmap = FreeImage_LockPage(mbitmap, 0);
		}
		else
			bitmap = FreeImage_Load(format, file);

		int bitsPerPixel = FreeImage_GetBPP(bitmap);
		if (bitsPerPixel == 32)
			bitmap32 = bitmap;
		else
			bitmap32 = FreeImage_ConvertTo32Bits(bitmap);

		vec2 size = vec2(FreeImage_GetWidth(bitmap32), FreeImage_GetHeight(bitmap32));
		userAsset->m_size = size;
		GLubyte* textureData = new GLubyte[4 * (int)size.x*(int)size.y];
		char* pixels = (char *)FreeImage_GetBits(bitmap32);

		for (int j = 0, total = (int)size.x*(int)size.y; j < total; j++) {
			const GLubyte
				&blue = pixels[j * 4 + 0],
				&green = pixels[j * 4 + 1],
				&red = pixels[j * 4 + 2],
				&alpha = pixels[j * 4 + 3];

			textureData[j * 4 + 0] = red;
			textureData[j * 4 + 1] = green;
			textureData[j * 4 + 2] = blue;
			textureData[j * 4 + 3] = alpha;
		}

		//Unload
		FreeImage_Unload(bitmap32);
		if (bitsPerPixel != 32)
			FreeImage_Unload(bitmap);
		userAsset->m_pixelData[sides] = textureData;
	}
}

void Asset_Cubemap::Finalize(Engine * engine, Shared_Asset_Cubemap & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();
	userAsset->finalize();

	// Create the final texture
	{
		unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &userAsset->m_glTexID);
	}

	// Load the final texture
	{
		shared_lock<shared_mutex> read_guard(userAsset->m_mutex);
		glTextureStorage2D(userAsset->m_glTexID, 1, GL_RGBA16F, userAsset->m_size.x, userAsset->m_size.x);
		for (int x = 0; x < 6; ++x)
			glTextureSubImage3D(userAsset->m_glTexID, 0, 0, 0, x, userAsset->m_size.x, userAsset->m_size.x, 1, GL_RGBA, GL_UNSIGNED_BYTE, userAsset->m_pixelData[x]);
		glTextureParameteri(userAsset->m_glTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(userAsset->m_glTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(userAsset->m_glTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(userAsset->m_glTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(userAsset->m_glTexID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// Notify completion
		for each (auto qwe in userAsset->m_callbacks)
			assetManager.submitNotifyee(qwe.second);
	}
}


void Asset_Cubemap::bind(const unsigned int & texture_unit)
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	glBindTextureUnit(texture_unit, m_glTexID);
}
