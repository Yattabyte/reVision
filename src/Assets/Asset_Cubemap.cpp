#include "Assets\Asset_Cubemap.h"
#include "Utilities\IO\Image_IO.h"
#include "Engine.h"

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
	m_size = glm::vec2(0);
}

Shared_Asset_Cubemap Asset_Cubemap::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Cubemap>(filename);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Cubemap>(filename);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string fullDirectory = DIRECTORY_CUBEMAP + filename;
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

void Asset_Cubemap::initializeDefault(Engine * engine)
{
	// Create hard-coded alternative
	m_pixelData[0] = new GLubyte[4]{ GLubyte(255), GLubyte(0), GLubyte(0), GLubyte(255) };
	m_pixelData[1] = new GLubyte[4]{ GLubyte(0), GLubyte(255), GLubyte(0), GLubyte(255) };
	m_pixelData[2] = new GLubyte[4]{ GLubyte(0), GLubyte(0), GLubyte(255), GLubyte(255) };
	m_pixelData[3] = new GLubyte[4]{ GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(255) };
	m_pixelData[4] = new GLubyte[4]{ GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(255) };
	m_pixelData[5] = new GLubyte[4]{ GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255) };
	m_size = glm::vec2(1);
}

void Asset_Cubemap::initialize(Engine * engine, const std::string & fullDirectory)
{
	static const std::string side_suffixes[6] = { "right", "left", "bottom", "top", "front", "back" };
	static const std::string extensions[3] = { ".png", ".jpg", ".tga" };
	for (int sides = 0; sides < 6; ++sides) {
		std::string specific_side_directory = "";
		const char * file;

		for (int x = 0; x < 3; ++x) {
			specific_side_directory = fullDirectory + side_suffixes[sides] + extensions[x];
			if (Engine::File_Exists(specific_side_directory))
				break;
			specific_side_directory = fullDirectory + "\\" + side_suffixes[sides] + extensions[x];
			if (Engine::File_Exists(specific_side_directory))
				break;
		}

		Image_Data dataContainer;
		if (!Image_IO::Import_Image(engine, specific_side_directory, dataContainer)) {
			engine->reportError(MessageManager::ASSET_FAILED, "Asset_Cubemap");
			initializeDefault(engine);
			return;
		}	
		
		std::unique_lock<std::shared_mutex> m_asset_guard(m_mutex);
		m_size = dataContainer.dimensions;
		m_pixelData[sides] = dataContainer.pixelData;
	}
}

void Asset_Cubemap::finalize(Engine * engine)
{
	// Create the final texture
	{
		std::unique_lock<std::shared_mutex> write_guard(m_mutex);
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_glTexID);
	}
	// Load the final texture
	{
		std::shared_lock<std::shared_mutex> read_guard(m_mutex);
		glTextureStorage2D(m_glTexID, 1, GL_RGBA16F, m_size.x, m_size.x);
		for (int x = 0; x < 6; ++x)
			glTextureSubImage3D(m_glTexID, 0, 0, 0, x, m_size.x, m_size.x, 1, GL_RGBA, GL_UNSIGNED_BYTE, m_pixelData[x]);
		glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_glTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_glTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_glTexID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		if (!glIsTexture(m_glTexID)) 
			engine->reportError(MessageManager::TEXTURE_INCOMPLETE, m_filename);		
	}
	Asset::finalize(engine);
}

void Asset_Cubemap::bind(const unsigned int & texture_unit)
{
	std::shared_lock<std::shared_mutex> read_guard(m_mutex);
	glBindTextureUnit(texture_unit, m_glTexID);
}
