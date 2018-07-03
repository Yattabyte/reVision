#include "Assets\Asset_Texture.h"
#include "Engine.h"
#include "FreeImage.h"


Asset_Texture::~Asset_Texture()
{
	if (existsYet())
		glDeleteTextures(1, &m_glTexID);
}

Asset_Texture::Asset_Texture(const string & filename) : Asset(filename)
{
	m_glTexID = GL_TEXTURE_2D;
	m_type = 0;
	m_size = vec2(0);
	m_pixelData = nullptr;
	m_mipmap = false;
	m_anis = false;
}

Asset_Texture::Asset_Texture(const string & filename, const GLuint & t, const bool & m, const bool & a) : Asset_Texture(filename)
{
	m_type = t;
	m_mipmap = m;
	m_anis = a;
}

void Asset_Texture::CreateDefault(Engine * engine, Shared_Asset_Texture & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, "defaultTexture"))
		return;

	// Create hard-coded alternative
	assetManager.createNewAsset(userAsset, "defaultTexture");
	userAsset->m_pixelData = new GLubyte[4];
	for (int x = 0; x < 4; ++x)
		userAsset->m_pixelData[x] = GLubyte(255);
	userAsset->m_size = vec2(1);

	// Create the asset
	assetManager.submitNewWorkOrder(userAsset, true,
		/* Initialization. */
		[]() {},	
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); }
	);
}

void Asset_Texture::Create(Engine * engine, Shared_Asset_Texture & userAsset, const string & filename, const GLuint & type, const bool & mipmap, const bool & anis, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const string &fullDirectory = DIRECTORY_TEXTURE + filename;
	if (!File_Reader::FileExistsOnDisk(fullDirectory)) {
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
		filename, type, mipmap, anis
	);
}

void Asset_Texture::Initialize(Engine * engine, Shared_Asset_Texture & userAsset, const string & fullDirectory)
{
	const char * file = fullDirectory.c_str();
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(file, 0);

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
	userAsset->m_pixelData = new GLubyte[4 * (int)size.x*(int)size.y];
	GLubyte *textureData = userAsset->m_pixelData;
	char* pixels = (char *)FreeImage_GetBits(bitmap32);

	for (int j = 0, total = (int)size.x*(int)size.y; j < total; j++) {
		GLubyte blue = pixels[j * 4 + 0],
			green = pixels[j * 4 + 1],
			red = pixels[j * 4 + 2],
			alpha = pixels[j * 4 + 3];
		textureData[j * 4 + 0] = red;
		textureData[j * 4 + 1] = green;
		textureData[j * 4 + 2] = blue;
		textureData[j * 4 + 3] = alpha;
	}

	//Unload
	FreeImage_Unload(bitmap32);
	if (bitsPerPixel != 32)
		FreeImage_Unload(bitmap);

	if (size.x < 1.5f || size.y < 1.5f)
		userAsset->m_type = GL_TEXTURE_1D;	
}

void Asset_Texture::Finalize(Engine * engine, Shared_Asset_Texture & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();

	unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
	glCreateTextures(userAsset->m_type, 1, &userAsset->m_glTexID);
	userAsset->m_finalized = true;
	write_guard.unlock();
	write_guard.release();
	shared_lock<shared_mutex> read_guard(userAsset->m_mutex);
	switch (userAsset->m_type) {
		case GL_TEXTURE_1D: {
			glTextureStorage1D(userAsset->m_glTexID, 1, GL_RGBA16F, userAsset->m_size.x);
			glTextureSubImage1D(userAsset->m_glTexID, 0, 0, userAsset->m_size.x, GL_RGBA, GL_UNSIGNED_BYTE, userAsset->m_pixelData);
			glTextureParameteri(userAsset->m_glTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(userAsset->m_glTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		}
		case GL_TEXTURE_2D: {
			glTextureStorage2D(userAsset->m_glTexID, 1, GL_RGBA16F, userAsset->m_size.x, userAsset->m_size.y);
			glTextureSubImage2D(userAsset->m_glTexID, 0, 0, 0, userAsset->m_size.x, userAsset->m_size.y, GL_RGBA, GL_UNSIGNED_BYTE, userAsset->m_pixelData);
			if (userAsset->m_anis)
				glTextureParameterf(userAsset->m_glTexID, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
			if (userAsset->m_mipmap) {
				glTextureParameteri(userAsset->m_glTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTextureParameteri(userAsset->m_glTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glGenerateTextureMipmap(userAsset->m_glTexID);
			}
			else {
				glTextureParameteri(userAsset->m_glTexID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTextureParameteri(userAsset->m_glTexID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}
			break;
		}
		case GL_TEXTURE_2D_ARRAY: {
			glTextureStorage3D(userAsset->m_glTexID, 1, GL_RGBA16F, userAsset->m_size.x, userAsset->m_size.y, 0);
			glTextureSubImage3D(userAsset->m_glTexID, 0, 0, 0, 0, userAsset->m_size.x, userAsset->m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, userAsset->m_pixelData);
			glTextureParameteri(userAsset->m_glTexID, GL_GENERATE_MIPMAP, GL_TRUE);
			glTextureParameteri(userAsset->m_glTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(userAsset->m_glTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateTextureMipmap(userAsset->m_glTexID);
			break;
		}
	}
	for each (auto qwe in userAsset->m_callbacks)
		assetManager.submitNotifyee(qwe.second);	
	/* To Do: Finalize call here*/
}

void Asset_Texture::bind(const unsigned int & texture_unit)
{
	glBindTextureUnit(texture_unit, m_glTexID);
}