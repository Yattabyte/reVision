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
	userAsset->m_size = glm::vec2(1);

	// Create the asset
	assetManager.submitNewWorkOrder(userAsset, true,
		/* Initialization. */
		[]() {},
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); }
	);
}

void Asset_Cubemap::Create(Engine * engine, Shared_Asset_Cubemap & userAsset, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const std::string &fullDirectory = DIRECTORY_CUBEMAP + filename;
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

void Asset_Cubemap::Initialize(Engine * engine, Shared_Asset_Cubemap & userAsset, const std::string & fullDirectory)
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
			engine->reportError(MessageManager::OTHER_ERROR, "Failed to load cubemap asset, using default...");
			CreateDefault(engine, userAsset);
			return;
		}	
		
		std::unique_lock<std::shared_mutex> m_asset_guard(userAsset->m_mutex);
		userAsset->m_size = dataContainer.dimensions;
		userAsset->m_pixelData[sides] = dataContainer.pixelData;
	}
}

void Asset_Cubemap::Finalize(Engine * engine, Shared_Asset_Cubemap & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();
	userAsset->finalize();

	// Create the final texture
	{
		std::unique_lock<std::shared_mutex> write_guard(userAsset->m_mutex);
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &userAsset->m_glTexID);
	}

	// Load the final texture
	{
		std::shared_lock<std::shared_mutex> read_guard(userAsset->m_mutex);
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
	std::shared_lock<std::shared_mutex> read_guard(m_mutex);
	glBindTextureUnit(texture_unit, m_glTexID);
}
