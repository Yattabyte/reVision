#include "Assets\Asset_Texture.h"
#include "Utilities\IO\Image_IO.h"
#include "Engine.h"
#define EXT_TEXTURE	".png"
#define DIRECTORY_TEXTURE Engine::Get_Current_Dir() + "\\Textures\\"
#define ABS_DIRECTORY_TEXTURE(filename) DIRECTORY_TEXTURE + filename + EXT_TEXTURE


Asset_Texture::~Asset_Texture()
{
	if (existsYet())
		glDeleteTextures(1, &m_glTexID);
	delete m_pixelData;
}

Asset_Texture::Asset_Texture(const std::string & filename) : Asset(filename)
{
	m_glTexID = GL_TEXTURE_2D;
	m_type = 0;
	m_size = glm::vec2(0);
	m_pixelData = nullptr;
	m_mipmap = false;
	m_anis = false;
}

Asset_Texture::Asset_Texture(const std::string & filename, const GLuint & t, const bool & m, const bool & a) : Asset_Texture(filename)
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
	userAsset->m_size = glm::vec2(1);

	// Create the asset
	assetManager.submitNewWorkOrder(userAsset, true,
		/* Initialization. */
		[]() {},	
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); }
	);
}

void Asset_Texture::Create(Engine * engine, Shared_Asset_Texture & userAsset, const std::string & filename, const GLuint & type, const bool & mipmap, const bool & anis, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const std::string &fullDirectory = DIRECTORY_TEXTURE + filename;
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
		filename, type, mipmap, anis
	);
}

void Asset_Texture::Initialize(Engine * engine, Shared_Asset_Texture & userAsset, const std::string & fullDirectory)
{
	Image_Data dataContainer;
	if (!Image_IO::Import_Image(engine, fullDirectory, dataContainer)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Texture");
		CreateDefault(engine, userAsset);
		return;
	}

	std::unique_lock<std::shared_mutex> m_asset_guard(userAsset->m_mutex);
	userAsset->m_size = dataContainer.dimensions;
	userAsset->m_pixelData = dataContainer.pixelData;
	GLubyte *textureData = userAsset->m_pixelData;
	if (dataContainer.dimensions.x < 1.5f || dataContainer.dimensions.y < 1.5f)
		userAsset->m_type = GL_TEXTURE_1D;	
}

void Asset_Texture::Finalize(Engine * engine, Shared_Asset_Texture & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();
	userAsset->finalize();
		
	// Create Texture
	{
		std::unique_lock<std::shared_mutex> write_guard(userAsset->m_mutex);
		glCreateTextures(userAsset->m_type, 1, &userAsset->m_glTexID);
	}

	// Load Texture
	{
		std::shared_lock<std::shared_mutex> read_guard(userAsset->m_mutex);
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

		// Notify Completion
		for each (auto qwe in userAsset->m_callbacks)
			assetManager.submitNotifyee(qwe.first, qwe.second);
	}
}

void Asset_Texture::bind(const unsigned int & texture_unit)
{
	glBindTextureUnit(texture_unit, m_glTexID);
}