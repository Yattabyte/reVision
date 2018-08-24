#include "Assets\Asset_Material.h"
#include "Utilities\IO\Image_IO.h"
#include "Engine.h"
#include <math.h>
#include <minmax.h>
#include <fstream>
#include <sstream>

#define EXT_MATERIAL ".mat"
#define ABS_DIRECTORY_MATERIAL(filename) Engine::Get_Current_Dir() + "\\Materials\\" + filename + EXT_MATERIAL
#define ABS_DIRECTORY_MAT_TEX(filename) Engine::Get_Current_Dir() + "\\Textures\\Environment\\" + filename


Asset_Material::~Asset_Material()
{
	if (existsYet())
		glDeleteTextures(1, &m_glArrayID);
	if (m_materialData)
		delete m_materialData;
}

Asset_Material::Asset_Material(const std::string & filename) : Asset(filename)
{
	m_glArrayID = 0; // So we don't bind a texture with an auto-generated int like 3465384972
	m_matSpot = 0;
}

Asset_Material::Asset_Material(const std::string(&tx)[MAX_PHYSICAL_IMAGES]) : Asset_Material("")
{
	for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
		m_textures[x] = tx[x];
}

Shared_Asset_Material Asset_Material::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();
	MaterialManager & materialManager = engine->getMaterialManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Material>(filename);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Material>(filename);
		auto & assetRef = *userAsset.get();
		assetRef.m_matSpot = materialManager.generateID();

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = ABS_DIRECTORY_MATERIAL(filename);
		std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, fullDirectory);
		std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);
		if (!Engine::File_Exists(fullDirectory) || (filename == "") || (filename == " ")) {
			engine->reportError(MessageManager::FILE_MISSING, fullDirectory);
			initFunc = std::bind(&initializeDefault, &assetRef, engine);
		}

		// Submit the work order
		assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);
	}
	return userAsset;
}

Shared_Asset_Material Asset_Material::Create(Engine * engine, const std::string(&textures)[MAX_PHYSICAL_IMAGES], const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();
	MaterialManager & materialManager = engine->getMaterialManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.createNewAsset<Asset_Material>(textures);
	auto & assetRef = *userAsset.get();
	assetRef.m_matSpot = materialManager.generateID();

	// Check if the file/directory exists on disk
	std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, "");
	std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);

	// Submit the work order
	assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);	
	return userAsset;
}

void Asset_Material::initializeDefault(Engine * engine)
{
	m_materialData = new GLubyte[192]{
		// Albedo with full alpha
		GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255),
		GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255),
		GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255),
		GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255),
		GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255),
		GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255),
		GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255),
		GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255),

		// Straight pointing normal with empty fourth channel
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),

		// Quarter metalness (mostly dielectric), half roughness, no height, and full ambience (no occlusion)
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255)
	};
	m_size = glm::vec2(4);
}

void Asset_Material::initialize(Engine * engine, const std::string & fullDirectory)
{
	if (fullDirectory != "") {
		std::unique_lock<std::shared_mutex> write_guard(m_mutex);
		Asset_Material::Get_PBR_Properties(fullDirectory, m_textures[0], m_textures[1], m_textures[2], m_textures[3], m_textures[4], m_textures[5]);
		for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
			m_textures[x] = ABS_DIRECTORY_MAT_TEX(m_textures[x]);
	}

	Image_Data dataContainers[MAX_PHYSICAL_IMAGES];
	glm::ivec2 material_dimensions = glm::ivec2(1);

	// Load all images
	constexpr GLubyte defaultMaterial[MAX_PHYSICAL_IMAGES][4] = {
		{ GLubyte(255), GLubyte(255), GLubyte(255), GLubyte(255) },
		{ GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(0) },
		{ GLubyte(63), GLubyte(0), GLubyte(0), GLubyte(0) },
		{ GLubyte(128), GLubyte(0), GLubyte(0), GLubyte(0) },
		{ GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(0) },
		{ GLubyte(255), GLubyte(0), GLubyte(0), GLubyte(0) } 
	};
	std::shared_lock<std::shared_mutex> read_guard(m_mutex);
	for (unsigned int x = 0; x < MAX_PHYSICAL_IMAGES; ++x) 
		if (!Image_IO::Import_Image(engine, m_textures[x], dataContainers[x])) {
			dataContainers[x].pixelData = new GLubyte[4];
			for (int p = 0; p < 4; ++p)
				dataContainers[x].pixelData[p] = defaultMaterial[x][p];
			dataContainers[x].dimensions = glm::ivec2(1);
			dataContainers[x].pitch = 4;
			dataContainers[x].bpp = 32;
		}
	read_guard.unlock();
	read_guard.release();

	// Find the largest dimensions
	for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x) {
		const glm::ivec2 & dimensions = dataContainers[x].dimensions;
		if (material_dimensions.x < dimensions.x)
			material_dimensions.x = dimensions.x;
		if (material_dimensions.y < dimensions.y)
			material_dimensions.y = dimensions.y;
	}

	// Force all images to be the same size
	for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
		Image_IO::Resize_Image(material_dimensions, dataContainers[x]);

	// Merge data into single array
	const unsigned int pixelsPerImage = material_dimensions.x * material_dimensions.y * 4;
	GLubyte * materialData = new GLubyte[(pixelsPerImage) * 3]();
	unsigned int arrayIndex = 0;
	for (unsigned int x = 0, size = pixelsPerImage; x < size; ++x, ++arrayIndex)		
		materialData[arrayIndex] = dataContainers[0].pixelData[x]; // ALBEDO	
	for (unsigned int x = 0, size = pixelsPerImage; x < size;  ++x, ++arrayIndex) 
		materialData[arrayIndex] = dataContainers[1].pixelData[x]; // NORMAL
	for (unsigned int x = 0, size = pixelsPerImage; x < size; x += 4, arrayIndex += 4) {
		materialData[arrayIndex + 0] = dataContainers[2].pixelData[x]; // METALNESS
		materialData[arrayIndex + 1] = dataContainers[3].pixelData[x]; // ROUGHNESS
		materialData[arrayIndex + 2] = dataContainers[4].pixelData[x]; // HEIGHT
		materialData[arrayIndex + 3] = dataContainers[5].pixelData[x]; // AO
	}

	// Delete old data
	for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
		delete dataContainers[x].pixelData;

	// Assign data to asset
	std::unique_lock<std::shared_mutex> write_guard(m_mutex);
	m_size = material_dimensions;
	m_materialData = materialData;
}

void Asset_Material::finalize(Engine * engine)
{
	MaterialManager & materialManager = engine->getMaterialManager();

	// Create Material
	{
		std::unique_lock<std::shared_mutex> write_guard(m_mutex);
		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_glArrayID);
	}
	// Load material
	{
		
		std::shared_lock<std::shared_mutex> read_guard(m_mutex);
		float anisotropy;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);
		// The equation beneath calculates the nubmer of mip levels needed, to mip down to a size of 1
		// Uses the smallest dimension of the image
		glTextureStorage3D(m_glArrayID, floor(log2f((min(m_size.x, m_size.y))) + 1), GL_RGBA16F, (int)m_size.x, (int)m_size.y, 3);
		glTextureSubImage3D(m_glArrayID, 0, 0, 0, 0, (int)m_size.x, (int)m_size.y, 3, GL_RGBA, GL_UNSIGNED_BYTE, m_materialData);
		glTextureParameteri(m_glArrayID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_glArrayID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_glArrayID, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
		glGenerateTextureMipmap(m_glArrayID);

		// Synchronize because sometimes driver hasn't completed generating mipmap's before the handle is created 
		// That IS a problem, because once the handle is issued, the texture object CAN NOT and MUST NOT be changed!!!
		GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		auto state = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
		while (state != GL_SIGNALED && state != GL_ALREADY_SIGNALED && state == GL_CONDITION_SATISFIED)
			state = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
		glDeleteSync(fence);
		materialManager.generateHandle(m_matSpot, m_glArrayID);
		if (!glIsTexture(m_glArrayID))
			engine->reportError(MessageManager::MATERIAL_INCOMPLETE, m_filename, m_textures[0] + ", " + m_textures[1] + ", " + m_textures[2] + ", " + m_textures[3] + ", " + m_textures[4] + ", " + m_textures[5]);
	}
	Asset::finalize(engine);
}

/** Fetch the directory of a material texture from its definition file. */
bool getString(std::istringstream & string_stream, std::string & target, std::string & input = std::string(""))
{
	string_stream >> input;
	if (input == "std::string") {
		int size = 0; string_stream >> size;
		target.reserve(size);
		for (int x = 0; x < size; ++x) {
			std::string v;
			string_stream >> v;
			target += v;
		}
	}
	else return false;
	return true;
}

void Asset_Material::Get_PBR_Properties(const std::string & filename, std::string & albedo, std::string & normal, std::string & metalness, std::string & roughness, std::string & height, std::string & occlusion)
{
	std::ifstream file_stream(filename);
	for (std::string line; std::getline(file_stream, line); ) {
		if (file_stream.good()) {
			if (line == "PBR") {
				bool end = false;
				std::getline(file_stream, line);

				const size_t propertycount = 6;

				for (int x = 0; x < propertycount; ++x) {
					std::string string;
					std::getline(file_stream, line);
					std::istringstream string_stream(line);
					string_stream >> line;
					if (getString(string_stream, string)) {
						if (line == "albedo") albedo = string;
						else if (line == "normal") normal = string;
						else if (line == "metalness") metalness = string;
						else if (line == "roughness") roughness = string;
						else if (line == "height") height = string;
						else if (line == "occlusion") occlusion = string;
						else if (line == "}") break;
						else break;
					}
					else break;
				}
				// ensure we are at end of class
				while (line != "}" && !end) {
					file_stream >> line;
					end = file_stream.bad();
				}
			}
		}
	}
}