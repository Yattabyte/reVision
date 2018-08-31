#include "Assets\Asset_Image.h"
#include "Utilities\IO\Image_IO.h"
#include "Engine.h"


Asset_Image::~Asset_Image()
{
	delete m_pixelData;
}

Asset_Image::Asset_Image(const std::string & filename) : Asset(filename) {}

Shared_Asset_Image Asset_Image::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Image>(filename);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Image>(filename);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = filename;
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

void Asset_Image::initializeDefault(Engine * engine)
{
	// Create hard-coded alternative
	m_pixelData = new GLubyte[4];
	for (int x = 0; x < 4; ++x)
		m_pixelData[x] = GLubyte(255);
	m_size = glm::ivec2(1);
}

void Asset_Image::initialize(Engine * engine, const std::string & fullDirectory)
{
	Image_Data dataContainer;
	if (!Image_IO::Import_Image(engine, fullDirectory, dataContainer)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Image");
		initializeDefault(engine);
		return;
	}

	std::unique_lock<std::shared_mutex> m_asset_guard(m_mutex);
	m_size = dataContainer.dimensions;
	m_pixelData = dataContainer.pixelData;
	m_pitch = dataContainer.pitch;
	m_bpp = dataContainer.bpp;
}

void Asset_Image::finalize(Engine * engine)
{
	Asset::finalize(engine);
}