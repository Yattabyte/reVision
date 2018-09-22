#include "Assets\Asset_Image.h"
#include "Utilities\IO\Image_IO.h"
#include "Engine.h"
#include "glm\gtc\type_ptr.hpp"

#define DIRECTORY_IMAGE Engine::Get_Current_Dir()

Asset_Image::~Asset_Image()
{
	delete m_pixelData;
}

Asset_Image::Asset_Image(const std::string & filename) : Asset(filename) {}

Shared_Asset_Image Asset_Image::Create(Engine * engine, const std::string & filename, const bool & threaded, const GLenum & policyFill, const GLenum & policyResize)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Image>(filename);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Image>(filename);
		auto & assetRef = *userAsset.get();
		assetRef.m_policyFill = policyFill;
		assetRef.m_policyResize = policyResize;

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = DIRECTORY_IMAGE + filename;
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
	fill();
}

void Asset_Image::initialize(Engine * engine, const std::string & fullDirectory)
{
	Image_Data dataContainer;
	if (!Image_IO::Import_Image(engine, fullDirectory, dataContainer)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Image");
		initializeDefault(engine);
		return;
	}

	m_size = dataContainer.dimensions;
	m_pixelData = dataContainer.pixelData;
	m_pitch = dataContainer.pitch;
	m_bpp = dataContainer.bpp;
}

void Asset_Image::finalize(Engine * engine)
{
	Asset::finalize(engine);
}

void Asset_Image::fill(const glm::uvec4 primaryColor, const glm::uvec4 secondaryColor)
{
	constexpr size_t defaultSize = 256;
	constexpr size_t pixelCount = defaultSize * defaultSize;
	constexpr size_t componentCount = pixelCount * 4;
	m_size = glm::ivec2(defaultSize);
	m_pixelData = new GLubyte[componentCount];
	m_pitch = defaultSize * 4;
	m_bpp = 32;
	switch (m_policyFill) {
		case Solid: {
			size_t pxComponent = 0;
			std::generate(m_pixelData, m_pixelData + componentCount, [&](void)->GLubyte {
				const unsigned int ID = (pxComponent++) % 4;
				const GLubyte component = GLubyte(primaryColor[ID]);
				return component;
			});
			break;
		}
		case Checkered: {
			// How many pixels wide and tall the checkers should be
			constexpr size_t checkerSize = 32;
			constexpr size_t rowWidth = defaultSize;
			const GLubyte colors[2][4] = {
				{ GLubyte(primaryColor.x), GLubyte(primaryColor.y), GLubyte(primaryColor.z), GLubyte(primaryColor.w) },
				{ GLubyte(secondaryColor.x), GLubyte(secondaryColor.y), GLubyte(secondaryColor.z), GLubyte(secondaryColor.w) }
			};
			bool cFlip = false;
			for (size_t pixel = 0, depth = -1; pixel < pixelCount; ++pixel) {
				// Flip color if we've drawn enough of the checker
				if ((pixel % checkerSize) == 0)
					cFlip = !cFlip;
				// Increment depth when we finish a row
				if ((pixel % rowWidth) == 0) {
					depth++;
					if (depth % checkerSize == 0)
						cFlip = !cFlip;
				}
				// Copy color info into pixel spot
				std::memcpy(&m_pixelData[pixel * 4], colors[cFlip], sizeof(GLubyte) * 4);
			}
			break;
		}
	};
}

void Asset_Image::resize(const glm::ivec2 newSize)
{
	Image_Data dataContainer{ m_pixelData, m_size, m_pitch, m_bpp };
	Image_IO::Resize_Image(newSize, dataContainer, m_policyResize);
	m_size = dataContainer.dimensions;
	m_pixelData = dataContainer.pixelData;
	m_pitch = dataContainer.pitch;
	m_bpp = dataContainer.bpp;
}