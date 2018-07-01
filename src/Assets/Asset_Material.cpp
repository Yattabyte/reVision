#include "Assets\Asset_Material.h"
#include "Managers\Material_Manager.h"
#include "Managers\Message_Manager.h"
#include "Utilities\Image_Importer.h"
#include "FreeImage.h"
#include <math.h>


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

void Asset_Material::CreateDefault(AssetManager & assetManager, Shared_Asset_Material & userAsset)
{
	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, "defaultMaterial"))
		return;

	// Create hard-coded alternative
	assetManager.createNewAsset(userAsset, "defaultMaterial", Material_Manager::Generate_ID());
	userAsset->m_materialData = new GLubyte[12]{
		GLubyte(255), GLubyte(255), GLubyte(255), GLubyte(255), // Albedo with full alpha
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), // Straight pointing normal with empty fourth channel
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255)  // Quarter metalness (mostly dielectric), half roughness, no height, and full ambience (no occlusion)
	};
	userAsset->m_size = vec2(1);

	// Create the asset
	assetManager.submitNewWorkOrder(userAsset, true,
		/* Initialization. */
		[]() {},
		/* Finalization. */
		[&assetManager, &userAsset]() { Finalize(assetManager, userAsset); }
	);
}

void Asset_Material::Create(AssetManager & assetManager, Shared_Asset_Material & userAsset, const std::string & filename, const bool & threaded, const std::string(&textures)[MAX_PHYSICAL_IMAGES])
{
	if (filename != "") {
		// Check if a copy already exists
		if (assetManager.queryExistingAsset(userAsset, filename))
			return;

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = ABS_DIRECTORY_MATERIAL(filename);
		if (!File_Reader::FileExistsOnDisk(fullDirectory) || (filename == "") || (filename == " ")) {
			MSG_Manager::Error(MSG_Manager::FILE_MISSING, fullDirectory);
			CreateDefault(assetManager, userAsset);
			return;
		}

		// Create the asset
		assetManager.submitNewAsset(userAsset, threaded,
			/* Initialization. */
			[&assetManager, &userAsset, fullDirectory]() mutable { Initialize(assetManager, userAsset, fullDirectory); },
			/* Finalization. */
			[&assetManager, &userAsset]() mutable { Finalize(assetManager, userAsset); },
			/* Constructor Arguments. */
			filename, Material_Manager::Generate_ID()
		);
	}
	else {
		assetManager.submitNewAsset<Asset_Material>(userAsset, threaded,
			/* Initialization. */
			[&assetManager, &userAsset]() mutable { Asset_Material::Initialize(assetManager, userAsset, ""); },
			/* Finalization. */
			[&assetManager, &userAsset]() mutable { Asset_Material::Finalize(assetManager, userAsset); },
			/* Constructor Arguments. */
			textures, Material_Manager::Generate_ID()
		);
	}
}

void Asset_Material::Initialize(AssetManager & assetManager, Shared_Asset_Material & userAsset, const string & fullDirectory)
{
	if (fullDirectory != "") {
		unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
		Asset_Material::Get_PBR_Properties(fullDirectory, userAsset->m_textures[0], userAsset->m_textures[1], userAsset->m_textures[2], userAsset->m_textures[3], userAsset->m_textures[4], userAsset->m_textures[5]);
		for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
			userAsset->m_textures[x] = ABS_DIRECTORY_MAT_TEX(userAsset->m_textures[x]);
	}

	shared_lock<shared_mutex> read_guard(userAsset->m_mutex);
	ivec2 material_dimensions = ivec2(1);
	GLubyte *textureData[MAX_PHYSICAL_IMAGES];
	FIBITMAP *bitmaps[MAX_PHYSICAL_IMAGES];

	// Load all images
	for (unsigned int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
		bitmaps[x] = Image_Importer::import_Image(userAsset->m_textures[x]);

	// unlock
	read_guard.unlock();
	read_guard.release();

	// Find the largest dimensions
	for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x) {
		if (bitmaps[x]) {
			const ivec2 dimensions(FreeImage_GetWidth(bitmaps[x]), FreeImage_GetHeight(bitmaps[x]));
			if (material_dimensions.x < dimensions.x)
				material_dimensions.x = dimensions.x;
			if (material_dimensions.y < dimensions.y)
				material_dimensions.y = dimensions.y;
		}
	}

	// Force all images to be the same size
	for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
		if (bitmaps[x]) {
			FIBITMAP *temp = FreeImage_Rescale(bitmaps[x], material_dimensions.x, material_dimensions.y);
			FreeImage_Unload(bitmaps[x]);
			bitmaps[x] = temp;
		}

	// Fetch and format the raw data
	textureData[0] = Image_Importer::parse_Image_4_channel(bitmaps[0], material_dimensions);
	textureData[1] = Image_Importer::parse_Image_3_channel(bitmaps[1], material_dimensions);
	for (int x = 2; x < MAX_PHYSICAL_IMAGES; ++x)
		textureData[x] = Image_Importer::parse_Image_1_channel(bitmaps[x], material_dimensions);

	// Zero initialize the material data to compensate for any holes / missing images
	const unsigned int size_mult = material_dimensions.x * material_dimensions.y;
	const unsigned int data_size = (size_mult * 4) + // Albedo size
		(size_mult * 4) + // Normal size
		(size_mult * 4); // Metalness, roughness, height, and ao size
	unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
	userAsset->m_size = material_dimensions;
	userAsset->m_materialData = new GLubyte[data_size];
	GLubyte *materialData = userAsset->m_materialData;
	unsigned int arrayIndex = 0;
	// First texture has white albedo with full alpha
	for (unsigned int x = 0, first_size = size_mult * 4; x < first_size; ++x, ++arrayIndex)
		materialData[arrayIndex] = GLubyte(255);
	// Second texture has straight pointing normal with empty fourth channel
	for (unsigned int x = 0, second_size = size_mult * 4; x < second_size; x += 4, arrayIndex += 4) {
		materialData[arrayIndex + 0] = GLubyte(128);
		materialData[arrayIndex + 1] = GLubyte(128);
		materialData[arrayIndex + 2] = GLubyte(255);
		materialData[arrayIndex + 3] = GLubyte(0);
	}
	// Third texture has quarter metalness (mostly dielectric), half roughness, no height, and full ambience
	for (unsigned int x = 0, third_size = size_mult * 4; x < third_size; x += 4, arrayIndex += 4) {
		materialData[arrayIndex + 0] = GLubyte(63);
		materialData[arrayIndex + 1] = GLubyte(128);
		materialData[arrayIndex + 2] = GLubyte(0);
		materialData[arrayIndex + 3] = GLubyte(255);
	}

	// Fill in remaining data with images
	arrayIndex = 0;
	if (bitmaps[0]) // Albedo
		for (unsigned int x = 0, size = size_mult * 4; x < size; ++x, ++arrayIndex)
			materialData[arrayIndex] = textureData[0][x];
	arrayIndex = (size_mult * 4);
	if (bitmaps[1]) // Normal
		for (unsigned int x = 0, size = size_mult * 3; x < size; x += 3, arrayIndex += 4) {
			materialData[arrayIndex + 0] = textureData[1][x + 0];
			materialData[arrayIndex + 1] = textureData[1][x + 1];
			materialData[arrayIndex + 2] = textureData[1][x + 2];
		}
	arrayIndex = (size_mult * 4) * 2;
	for (unsigned int x = 0, size = size_mult; x < size; ++x, arrayIndex += 4) {
		if (bitmaps[2]) // Metalness
			materialData[arrayIndex + 0] = textureData[2][x];
		if (bitmaps[3]) // Roughness
			materialData[arrayIndex + 1] = textureData[3][x];
		if (bitmaps[4]) // Height
			materialData[arrayIndex + 2] = textureData[4][x];
		if (bitmaps[5]) // AO
			materialData[arrayIndex + 3] = textureData[5][x];
	}

	// Delete old data
	for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
		if (bitmaps[x]) {
			delete textureData[x];
			FreeImage_Unload(bitmaps[x]);
		}
}

void Asset_Material::Finalize(AssetManager & assetManager, Shared_Asset_Material & userAsset)
{
	unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
	userAsset->m_finalized = true;
	auto &gl_array_ID = userAsset->m_glArrayID;
	auto &size = userAsset->m_size;
	auto *materialData = userAsset->m_materialData;
	auto &mat_spot = userAsset->m_matSpot;
	float anisotropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);

	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &gl_array_ID);
	// The equation beneath calculates the nubmer of mip levels needed, to mip down to a size of 1
	// Uses the smallest dimension of the image
	glTextureStorage3D(gl_array_ID, floor(log2f((min(size.x, size.y))) + 1), GL_RGBA16F, (int)size.x, (int)size.y, 3);
	glTextureSubImage3D(gl_array_ID, 0, 0, 0, 0, (int)size.x, (int)size.y, 3, GL_RGBA, GL_UNSIGNED_BYTE, materialData);
	glTextureParameteri(gl_array_ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(gl_array_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(gl_array_ID, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
	glGenerateTextureMipmap(gl_array_ID);

	GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	auto state = glClientWaitSync(fence, 0, 0);
	while (state != GL_SIGNALED && state != GL_ALREADY_SIGNALED && state == GL_CONDITION_SATISFIED)
		state = glClientWaitSync(fence, 0, 0);
	glDeleteSync(fence);
	Material_Manager::Generate_Handle(mat_spot, gl_array_ID);

	write_guard.unlock();
	write_guard.release();
	shared_lock<shared_mutex> read_guard(userAsset->m_mutex);
	for each (auto qwe in userAsset->m_callbacks)
		assetManager.submitNotifyee(qwe.second);
	/* To Do: Finalize call here*/
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