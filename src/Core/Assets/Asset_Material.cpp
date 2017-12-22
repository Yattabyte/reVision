#include "Assets\Asset_Material.h"
#include "Managers\Material_Manager.h"
#include "Systems\Message_Manager.h"
#include "Utilities\ImageImporter.h"
#include "FreeImage.h"

/* -----ASSET TYPE----- */
#define ASSET_TYPE 3

using namespace Asset_Loader;

Asset_Material::~Asset_Material()
{
	if (ExistsYet())
		glDeleteTextures(1, &gl_array_ID);
	if (materialData)
		delete materialData;
}

Asset_Material::Asset_Material(const string & filename) : Asset(filename)
{
	gl_array_ID = 0; // So we don't bind a texture with an autogenerated int like 3465384972
}

Asset_Material::Asset_Material(const std::string & filename, const GLuint & spot) : Asset_Material(filename)
{
	mat_spot = spot;
}

Asset_Material::Asset_Material(const std::string(&tx)[MAX_PHYSICAL_IMAGES], const GLuint & spot) : Asset_Material("")
{
	for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
		textures[x] = tx[x];
	mat_spot = spot;
}

void Asset_Material::setTextures(const std::string(&tx)[MAX_PHYSICAL_IMAGES])
{
	for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
		textures[x] = tx[x];
}

int Asset_Material::GetAssetType()
{
	return ASSET_TYPE;
}

void Asset_Material::getPBRProperties(const string & filename, string & albedo, string & normal, string & metalness, string & roughness, string & height, string & occlusion)
{
	std::ifstream file_stream(filename);
	for (std::string line; std::getline(file_stream, line); ) {
		if (file_stream.good()) {
			if (line == "PBR") {
				bool end = false;
				std::getline(file_stream, line);

				const size_t propertycount = 6;

				for (int x = 0; x < propertycount; ++x) {
					FileReader::DocParser::Property property;
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

// Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
// Will generate a default one itself if the default doesn't exist.
Shared_Asset_Material fetchDefaultAsset()
{
	shared_lock<shared_mutex> guard(Asset_Manager::GetMutex_Assets());
	std::map<int, Shared_Asset> &fallback_assets = Asset_Manager::GetFallbackAssets_Map();
	fallback_assets.insert(std::pair<int, Shared_Asset>(Asset_Material::GetAssetType(), Shared_Asset()));
	auto &default_asset = fallback_assets[Asset_Material::GetAssetType()];
	guard.unlock();
	guard.release();
	if (default_asset.get() == nullptr) { // Check if we already created the default asset
		default_asset = shared_ptr<Asset_Material>(new Asset_Material("defaultMaterial"));
		Shared_Asset_Material cast_asset = dynamic_pointer_cast<Asset_Material>(default_asset);
		string fulldirectory = ABS_DIRECTORY_MATERIAL("defaultMaterial");
		if (FileReader::FileExistsOnDisk(fulldirectory)) { // Check if we have a default one on disk to load
			Material_WorkOrder work_order(cast_asset, fulldirectory);
			work_order.Initialize_Order();
			work_order.Finalize_Order();
			if (cast_asset->ExistsYet()) // did we successfully load the default asset from disk?
				return cast_asset;
		}
		// We didn't load a default asset from disk
		// This will report many missing file errors, but will work.
		bool complete = false; 
		Material_WorkOrder work_order(cast_asset,"");
		work_order.Initialize_Order();
		work_order.Finalize_Order();
		return cast_asset;
	}
	return dynamic_pointer_cast<Asset_Material>(default_asset);
}

namespace Asset_Loader {
	void load_asset(Shared_Asset_Material & user, const std::string(&textures)[MAX_PHYSICAL_IMAGES], const bool & threaded) {
		// Check if a copy already exists
		shared_mutex &mutex_IO_assets = Asset_Manager::GetMutex_Assets();
		auto &assets_materials = (Asset_Manager::GetAssets_List(Asset_Material::GetAssetType()));
		{
			shared_lock<shared_mutex> guard(mutex_IO_assets);
			for each (auto &asset in assets_materials) {
				bool identical = true;
				shared_lock<shared_mutex> asset_guard(asset->m_mutex);
				const Shared_Asset_Material derived_asset = dynamic_pointer_cast<Asset_Material>(asset);
				if (derived_asset) {
					for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x) {
						if (derived_asset->textures[x] != textures[x]) {
							identical = false;
							break;
						}
					}
					if (identical) {
						asset_guard.unlock();
						asset_guard.release();
						user = derived_asset;
						// Can't guarantee that the asset isn't already being worked on, so no finalization here if threaded
						return;
					}
				}
			}
		}
		
		Asset_Manager::CreateNewAsset<Asset_Material, Material_WorkOrder>(user, threaded, "", textures, Material_Manager::GenerateMaterialBufferID());
	}

	void load_asset(Shared_Asset_Material & user, const std::string & filename, const bool & threaded)
	{
		// Check if a copy already exists
		if (Asset_Manager::QueryExistingAsset<Asset_Material>(user, filename))
			return;

		// Attempt to create the asset
		const std::string &fullDirectory = ABS_DIRECTORY_MATERIAL(filename);
		if (!FileReader::FileExistsOnDisk(fullDirectory) || (filename == "") || (filename == " ")) {
			MSG::Error(FILE_MISSING, fullDirectory);
			user = fetchDefaultAsset();
			return;
		}

		Asset_Manager::CreateNewAsset<Asset_Material, Material_WorkOrder>(user, threaded, fullDirectory, filename, Material_Manager::GenerateMaterialBufferID());
	}
}

void Material_WorkOrder::Initialize_Order()
{
	if (m_filename != "") {
		Asset_Material::getPBRProperties(m_filename, m_asset->textures[0], m_asset->textures[1], m_asset->textures[2], m_asset->textures[3], m_asset->textures[4], m_asset->textures[5]);
		for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
			m_asset->textures[x] = DIRECTORY_MATERIAL + (m_asset->textures[x]);
	}

	vec2 material_dimensions = vec2(0);
	{
		unique_lock<shared_mutex> surface_guard(m_asset->m_mutex);

		bool success[MAX_PHYSICAL_IMAGES];
		vec2 dimensions[MAX_PHYSICAL_IMAGES];
		int dataSize[MAX_PHYSICAL_IMAGES];
		GLubyte *textureData[MAX_PHYSICAL_IMAGES];

		textureData[0] = ImageImporter::ReadImage_4channel(m_asset->textures[0], dimensions[0], dataSize[0], success[0]);
		textureData[1] = ImageImporter::ReadImage_3channel(m_asset->textures[1], dimensions[1], dataSize[1], success[1]);
		for (int x = 2; x < MAX_PHYSICAL_IMAGES; ++x)
			textureData[x] = ImageImporter::ReadImage_1channel(m_asset->textures[x], dimensions[x], dataSize[x], success[x]);

		// Material MUST be entirely the same dimensions
		// enforce the first found dimension
		material_dimensions = dimensions[0];
		for (int x = 1; x < MAX_PHYSICAL_IMAGES; ++x) {
			if (success[x]) {
				if (material_dimensions.x < dimensions[x].x)
					material_dimensions.x = dimensions[x].x;
				if (material_dimensions.y < dimensions[x].y)
					material_dimensions.y = dimensions[x].y;
			}
		}

		// If we didn't load a single damn file
		bool did_we_ever_succeed = false;
		for (int x = 0; x < MAX_PHYSICAL_IMAGES && did_we_ever_succeed == false; ++x)
			did_we_ever_succeed += success[x];
		if (!did_we_ever_succeed)
			material_dimensions = vec2(1);
		
		// Stitch data together 
		{	// 3 textures with 4 data channels (RGBA) of X*Y size
			const int mat_data_size_1 = int(material_dimensions.x * material_dimensions.y);
			const int mat_data_size_2 = mat_data_size_1 * 2;
			const int mat_data_size_3 = mat_data_size_1 * 3;
			const int mat_data_size_4 = mat_data_size_1 * 4;
			const int material_data_size = MAX_DIGITAL_IMAGES * mat_data_size_4;
			m_asset->materialData = new GLubyte[material_data_size];
			GLubyte *materialData = m_asset->materialData;

			// Running through whole thing, setting defaults in case an image is smaller than the max size
			// First texture has white albedo with full alpha
			for (int x = 0, size = mat_data_size_4; x < size; ++x)
				materialData[x] = GLubyte(255);
			// Second texture has straight pointing normal with no height
			for (int x = mat_data_size_4, size = mat_data_size_4 * 2; x < size; x += 4) {
				materialData[x + 0] = GLubyte(127);
				materialData[x + 1] = GLubyte(127);
				materialData[x + 2] = GLubyte(255);
				materialData[x + 3] = GLubyte(0);
			}
			// Third texture has quarter metalness (mostly dielectric), half roughness, empty third channel, and full ambience
			for (int x = mat_data_size_4 * 2, size = mat_data_size_4 * 3; x < size; x += 4) {
				materialData[x + 0] = GLubyte(63);
				materialData[x + 1] = GLubyte(127);
				materialData[x + 2] = GLubyte(0);
				materialData[x + 3] = GLubyte(255);
			}

			// ALBEDO
			if (success[0])
				// Fill with the minimum available amount of data, either from the pool or limited to max amount
				// ORDER R, G, B, A, R, G, B, A, etc...
				for (int x = 0, mat_data_spot = 0, total = min(dataSize[0], mat_data_size_4); x < total; ++x, ++mat_data_spot)
					materialData[mat_data_spot] = textureData[0][x];

			// NORMAL
			if (success[1])
				for (int n_x = 0, mat_data_spot = mat_data_size_4, total = min(dataSize[1], mat_data_size_3); n_x < total; n_x += 3, mat_data_spot += 4) {
					materialData[mat_data_spot] = textureData[1][n_x];
					materialData[mat_data_spot + 1] = textureData[1][n_x + 1];
					materialData[mat_data_spot + 2] = textureData[1][n_x + 2];
				}

			// METALNESS
			if (success[2])
				for (int x = 0, mat_data_spot = mat_data_size_4 * 2, m_t = min(dataSize[2], mat_data_size_1); x < m_t; x++, mat_data_spot += 4)
					materialData[mat_data_spot] = textureData[2][x];

			// ROUGHNESS
			if (success[3])
				for (int x = 0, mat_data_spot = mat_data_size_4 * 2, r_t = min(dataSize[3], mat_data_size_1); x < r_t; x++, mat_data_spot += 4)
					materialData[mat_data_spot + 1] = textureData[3][x];

			// HEIGHT
			if (success[4])
				for (int h_x = 0, mat_data_spot = mat_data_size_4, total = min(dataSize[4], mat_data_size_3); h_x < total; h_x++, mat_data_spot += 4)
					materialData[mat_data_spot + 3] = textureData[4][h_x];

			// AO
			if (success[5])
				for (int x = 0, mat_data_spot = mat_data_size_4 * 2, a_t = min(dataSize[5], mat_data_size_1); x < a_t; x++, mat_data_spot += 4)
					materialData[mat_data_spot + 3] = textureData[5][x];
		}

		// Delete old data
		for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
			if (success[x])
				delete textureData[x];
	}
	m_asset->size = material_dimensions;
}

void Material_WorkOrder::Finalize_Order()
{
	shared_lock<shared_mutex> read_guard(m_asset->m_mutex);
	if (!m_asset->ExistsYet()) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
		auto &gl_array_ID = m_asset->gl_array_ID;
		auto &size = m_asset->size;
		auto *materialData = m_asset->materialData;
		auto &mat_spot = m_asset->mat_spot;

		glGenTextures(1, &gl_array_ID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, gl_array_ID);

		float anisotropy = 0.0f, maxAnisotropy = 0.0f;
		anisotropy = 16.0f;// CFG::getPreference(CFG_ENUM::C_TEXTURE_ANISOTROPY);
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
		anisotropy = max(0.0f, min(anisotropy, maxAnisotropy));

		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA, (int)size.x, (int)size.y, 3);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, (int)size.x, (int)size.y, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, materialData);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
		glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

		Material_Manager::GenerateHandle(mat_spot, gl_array_ID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		m_asset->Finalize();
	}
}
