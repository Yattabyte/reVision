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

Shared_Asset_Texture Asset_Texture::Create(Engine * engine, const std::string & filename, const GLuint & type, const bool & mipmap, const bool & anis, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Texture>(filename);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Texture>(filename, type, mipmap, anis);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = DIRECTORY_TEXTURE + filename;
		std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, fullDirectory);
		std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);
		if (!Engine::File_Exists(fullDirectory)) {
			engine->reportError(MessageManager::FILE_MISSING, fullDirectory);
			initFunc = std::bind(&initializeDefault, &assetRef, engine);
		}

		// Submit the work order
		assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);
	}
	return userAsset;
}

void Asset_Texture::initializeDefault(Engine * engine)
{
	// Create hard-coded alternative
	m_pixelData = new GLubyte[4];
	for (int x = 0; x < 4; ++x)
		m_pixelData[x] = GLubyte(255);
	m_size = glm::vec2(1);
}

void Asset_Texture::initialize(Engine * engine, const std::string & fullDirectory)
{
	Image_Data dataContainer;
	if (!Image_IO::Import_Image(engine, fullDirectory, dataContainer)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Texture");
		initializeDefault(engine);
		return;
	}

	std::unique_lock<std::shared_mutex> m_asset_guard(m_mutex);
	m_size = dataContainer.dimensions;
	m_pixelData = dataContainer.pixelData;
	GLubyte *textureData = m_pixelData;
	if (dataContainer.dimensions.x < 1.5f || dataContainer.dimensions.y < 1.5f)
		m_type = GL_TEXTURE_1D;	
}

void Asset_Texture::finalize(Engine * engine)
{		
	// Create Texture
	{
		std::unique_lock<std::shared_mutex> write_guard(m_mutex);
		glCreateTextures(m_type, 1, &m_glTexID);
	}
	// Load Texture
	{
		std::shared_lock<std::shared_mutex> read_guard(m_mutex);
		switch (m_type) {
			case GL_TEXTURE_1D: {
				glTextureStorage1D(m_glTexID, 1, GL_RGBA8, m_size.x);
				glTextureSubImage1D(m_glTexID, 0, 0, m_size.x, GL_RGBA, GL_UNSIGNED_BYTE, m_pixelData);
				glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				break;
			}
			case GL_TEXTURE_2D: {
				glTextureStorage2D(m_glTexID, 1, GL_RGBA8, m_size.x, m_size.y);
				glTextureSubImage2D(m_glTexID, 0, 0, 0, m_size.x, m_size.y, GL_RGBA, GL_UNSIGNED_BYTE, m_pixelData);
				if (m_anis)
					glTextureParameterf(m_glTexID, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
				if (m_mipmap) {
					glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glGenerateTextureMipmap(m_glTexID);
				}
				else {
					glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				}
				break;
			}
			case GL_TEXTURE_2D_ARRAY: {
				glTextureStorage3D(m_glTexID, 1, GL_RGBA8, m_size.x, m_size.y, 0);
				glTextureSubImage3D(m_glTexID, 0, 0, 0, 0, m_size.x, m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pixelData);
				glTextureParameteri(m_glTexID, GL_GENERATE_MIPMAP, GL_TRUE);
				glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glGenerateTextureMipmap(m_glTexID);
				break;
			}
		}
	}
	Asset::finalize(engine);
}

void Asset_Texture::bind(const unsigned int & texture_unit)
{
	glBindTextureUnit(texture_unit, m_glTexID);
}