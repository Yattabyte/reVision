#include "Assets\Asset_Material.h"
#include "Utilities\IO\Image_IO.h"
#include "Engine.h"
#include <math.h>
#include <minmax.h>
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

Asset_Material::Asset_Material(const string & filename) : Asset(filename)
{
	m_glArrayID = 0; // So we don't bind a texture with an auto-generated int like 3465384972
}

Asset_Material::Asset_Material(const std::string & filename, const GLuint & spot) : Asset_Material(filename)
{
	m_matSpot = spot;
}

Asset_Material::Asset_Material(const std::string(&tx)[MAX_PHYSICAL_IMAGES], const GLuint & spot) : Asset_Material("")
{
	for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
		m_textures[x] = tx[x];
	m_matSpot = spot;
}

void Asset_Material::CreateDefault(Engine * engine, Shared_Asset_Material & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();
	MaterialManager & materialManager = engine->getMaterialManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, "defaultMaterial"))
		return;

	// Create hard-coded alternative
	assetManager.createNewAsset(userAsset, "defaultMaterial", materialManager.generateID());
	userAsset->m_materialData = new GLubyte[192]{
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
	userAsset->m_size = vec2(4);

	// Create the asset
	assetManager.submitNewWorkOrder(userAsset, true,
		/* Initialization. */
		[]() {},
		/* Finalization. */
		[engine, &materialManager, &userAsset]() mutable { Finalize(engine, userAsset); }
	);
}

void Asset_Material::Create(Engine * engine, Shared_Asset_Material & userAsset, const std::string & filename, const bool & threaded, const std::string(&textures)[MAX_PHYSICAL_IMAGES])
{
	AssetManager & assetManager = engine->getAssetManager();
	MaterialManager & materialManager = engine->getMaterialManager();

	if (filename != "") {
		// Check if a copy already exists
		if (assetManager.queryExistingAsset(userAsset, filename))
			return;

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = ABS_DIRECTORY_MATERIAL(filename);
		if (!Engine::File_Exists(fullDirectory) || (filename == "") || (filename == " ")) {
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
			filename, materialManager.generateID()
		);
	}
	else {
		assetManager.submitNewAsset<Asset_Material>(userAsset, threaded,
			/* Initialization. */
			[engine, &userAsset]() mutable { Asset_Material::Initialize(engine, userAsset, ""); },
			/* Finalization. */
			[engine, &userAsset]() mutable { Finalize(engine, userAsset); },
			/* Constructor Arguments. */
			textures, materialManager.generateID()
		);
	}
}

void Asset_Material::Initialize(Engine * engine, Shared_Asset_Material & userAsset, const string & fullDirectory)
{
	if (fullDirectory != "") {
		unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
		Asset_Material::Get_PBR_Properties(fullDirectory, userAsset->m_textures[0], userAsset->m_textures[1], userAsset->m_textures[2], userAsset->m_textures[3], userAsset->m_textures[4], userAsset->m_textures[5]);
		for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
			userAsset->m_textures[x] = ABS_DIRECTORY_MAT_TEX(userAsset->m_textures[x]);
	}

	Image_Data dataContainers[MAX_PHYSICAL_IMAGES];
	ivec2 material_dimensions = ivec2(1);

	// Load all images
	constexpr GLubyte defaultMaterial[MAX_PHYSICAL_IMAGES][4] = {
		{ GLubyte(255), GLubyte(255), GLubyte(255), GLubyte(255) },
		{ GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(0) },
		{ GLubyte(63), GLubyte(0), GLubyte(0), GLubyte(0) },
		{ GLubyte(128), GLubyte(0), GLubyte(0), GLubyte(0) },
		{ GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(0) },
		{ GLubyte(255), GLubyte(0), GLubyte(0), GLubyte(0) } 
	};
	shared_lock<shared_mutex> read_guard(userAsset->m_mutex);
	for (unsigned int x = 0; x < MAX_PHYSICAL_IMAGES; ++x) 
		if (!Image_IO::Import_Image(engine, userAsset->m_textures[x], dataContainers[x])) {
			dataContainers[x].pixelData = new GLubyte[4];
			for (int p = 0; p < 4; ++p)
				dataContainers[x].pixelData[p] = defaultMaterial[x][p];
			dataContainers[x].dimensions = ivec2(1);
			dataContainers[x].pitch = 4;
			dataContainers[x].bpp = 32;
		}
	read_guard.unlock();
	read_guard.release();

	// Find the largest dimensions
	for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x) {
		const ivec2 & dimensions = dataContainers[x].dimensions;
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
	unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
	userAsset->m_size = material_dimensions;
	userAsset->m_materialData = materialData;
}

void Asset_Material::Finalize(Engine * engine, Shared_Asset_Material & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();
	MaterialManager & materialManager = engine->getMaterialManager();
	userAsset->finalize();

	// Create Material
	{
		unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
		userAsset->m_finalized = true;
		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &userAsset->m_glArrayID);
	}
	{
		// Load material
		shared_lock<shared_mutex> read_guard(userAsset->m_mutex);
		float anisotropy;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);
		// The equation beneath calculates the nubmer of mip levels needed, to mip down to a size of 1
		// Uses the smallest dimension of the image
		glTextureStorage3D(userAsset->m_glArrayID, floor(log2f((min(userAsset->m_size.x, userAsset->m_size.y))) + 1), GL_RGBA16F, (int)userAsset->m_size.x, (int)userAsset->m_size.y, 3);
		glTextureSubImage3D(userAsset->m_glArrayID, 0, 0, 0, 0, (int)userAsset->m_size.x, (int)userAsset->m_size.y, 3, GL_RGBA, GL_UNSIGNED_BYTE, userAsset->m_materialData);
		glTextureParameteri(userAsset->m_glArrayID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(userAsset->m_glArrayID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(userAsset->m_glArrayID, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
		glGenerateTextureMipmap(userAsset->m_glArrayID);

		// Synchronize because sometimes driver hasn't completed generating mipmap's before the handle is created 
		// That IS a problem, because once the handle is issued, the texture object CAN NOT and MUST NOT be changed!!!
		GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		auto state = glClientWaitSync(fence, 0, 0);
		while (state != GL_SIGNALED && state != GL_ALREADY_SIGNALED && state == GL_CONDITION_SATISFIED)
			state = glClientWaitSync(fence, 0, 0);
		glDeleteSync(fence);
		materialManager.generateHandle(userAsset->m_matSpot, userAsset->m_glArrayID);

		// Notify Completion
		for each (auto qwe in userAsset->m_callbacks)
			assetManager.submitNotifyee(qwe.second); 
	}
}

void Asset_Material::Get_PBR_Properties(const string & filename, string & albedo, string & normal, string & metalness, string & roughness, string & height, string & occlusion)
{
	std::ifstream file_stream(filename);
	for (std::string line; std::getline(file_stream, line); ) {
		if (file_stream.good()) {
			if (line == "PBR") {
				bool end = false;
				std::getline(file_stream, line);

				const size_t propertycount = 6;

				for (int x = 0; x < propertycount; ++x) {
					File_Reader::DocParser::Property property;
					std::getline(file_stream, line);
					std::istringstream string_stream(line);
					string_stream >> line;
					if (getProperty(string_stream, property)) {
						if (line == "albedo") albedo = property.s;
						else if (line == "normal") normal = property.s;
						else if (line == "metalness") metalness = property.s;
						else if (line == "roughness") roughness = property.s;
						else if (line == "height") height = property.s;
						else if (line == "occlusion") occlusion = property.s;
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