#include "Assets\Asset_Cubemap.h"
#include "Engine.h"

#define EXT_CUBEMAP ".png"
#define DIRECTORY_CUBEMAP Engine::Get_Current_Dir() + "\\Textures\\Cubemaps\\"
#define ABS_DIRECTORY_CUBEMAP(filename) DIRECTORY_CUBEMAP + filename


Asset_Cubemap::~Asset_Cubemap()
{
	if (existsYet()) {
		glDeleteBuffers(6, m_pboIDs);
		glDeleteTextures(1, &m_glTexID);
	}
}

Asset_Cubemap::Asset_Cubemap(const std::string & filename) : Asset(filename) {}

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
	for (int side = 0; side < 6; ++side)
		// Forward image creation
		m_images[side] = Asset_Image::Create(engine, "", false); 	
}

void Asset_Cubemap::initialize(Engine * engine, const std::string & fullDirectory)
{
	static const std::string side_suffixes[6] = { "right", "left", "bottom", "top", "front", "back" };
	static const std::string extensions[3] = { ".png", ".jpg", ".tga" };
	for (int side = 0; side < 6; ++side) {
		std::string specific_side_directory = "";
		for (int x = 0; x < 3; ++x) {
			specific_side_directory = fullDirectory + side_suffixes[side] + extensions[x];
			if (Engine::File_Exists(specific_side_directory))
				break;
			specific_side_directory = fullDirectory + "\\" + side_suffixes[side] + extensions[x];
			if (Engine::File_Exists(specific_side_directory))
				break;
		}

		// Forward image creation
		m_images[side] = Asset_Image::Create(engine, specific_side_directory, false);
	}

	// Ensure each face is the same dimension
	glm::ivec2 size = glm::ivec2(1);
	for each (const auto & image in m_images) {
		if (size.x < image->m_size.x)
			size.x = image->m_size.x;
		if (size.y < image->m_size.y)
			size.y = image->m_size.y;
	}
	for each (auto image in m_images)
		if (image->m_size != size)
			image->resize(size);
}

void Asset_Cubemap::finalize(Engine * engine)
{
	// Create the final texture	
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_glTexID);
	glCreateBuffers(6, m_pboIDs);
	
	// Load the final texture
	glTextureStorage2D(m_glTexID, 1, GL_RGBA16F, m_images[0]->m_size.x, m_images[0]->m_size.x);
	for (int x = 0; x < 6; ++x) {
		glNamedBufferStorage(m_pboIDs[x], m_images[x]->m_size.x * m_images[x]->m_size.x * 4, m_images[x]->m_pixelData, 0);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIDs[x]);
		glTextureSubImage3D(m_glTexID, 0, 0, 0, x, m_images[x]->m_size.x, m_images[x]->m_size.x, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_glTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glTexID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	if (!glIsTexture(m_glTexID))
		engine->reportError(MessageManager::TEXTURE_INCOMPLETE, m_filename);

	// Finalize
	Asset::finalize(engine);
}

void Asset_Cubemap::bind(const unsigned int & texture_unit)
{
	glBindTextureUnit(texture_unit, m_glTexID);
}
