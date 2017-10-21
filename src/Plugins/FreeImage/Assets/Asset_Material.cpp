/*
	dt_FreeImage: Asset_Material
	
	- FreeImage specific implementation of Asset_Material	
*/

#include "Assets\Asset_Material.h"
#include "Managers\Material_Manager.h"
#include "Managers\Message_Manager.h"
#include "dt_FreeImage.h"
#include "FreeImage.h"

using namespace Asset_Manager;

// Forward declaration
Shared_Asset_Material fetchDefaultMaterial();
// Forward declaration
void initialize_Material(Shared_Asset_Material &material, const std::string &filename, bool *complete);

// Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
// Will generate a default one itself if the default doesn't exist.
Shared_Asset_Material fetchDefaultMaterial()
{
	shared_lock<shared_mutex> guard(getMutexIOAssets());
	std::map<int, Shared_Asset> &fallback_assets = getFallbackAssets();
	fallback_assets.insert(std::pair<int, Shared_Asset>(Asset_Material::GetAssetType(), Shared_Asset()));
	auto &default_asset = fallback_assets[Asset_Material::GetAssetType()];
	if (default_asset.get() == nullptr) { // Check if we already created the default asset
		default_asset = shared_ptr<Asset_Material>(new Asset_Material());
		Shared_Asset_Material cast_asset = dynamic_pointer_cast<Asset_Material>(default_asset);
		if (fileOnDisk(ABS_DIRECTORY_MATERIAL("defaultMaterial"))) { // Check if we have a default one on disk to load
			bool complete = false;
			initialize_Material(cast_asset, ABS_DIRECTORY_MATERIAL("defaultMaterial"), &complete);
			cast_asset->Finalize();
			if (complete && cast_asset->ExistsYet()) // did we successfully load the default asset from disk?
				return cast_asset;
		}
		// We didn't load a default asset from disk
		// This will report many missing file errors, but will work.
		bool complete = false;
		initialize_Material(cast_asset, "", &complete);
		cast_asset->Finalize();
		return cast_asset;
	}
	return dynamic_pointer_cast<Asset_Material>(default_asset);
}

// Loads the material file from disk using dt_FreeImage into memory
void initialize_Material(Shared_Asset_Material &material, const std::string &filename, bool *complete)
{
	if (filename != "") {
		Asset_Material::getPBRProperties(filename, material->textures[0], material->textures[1], material->textures[2], material->textures[3], material->textures[4], material->textures[5]);
		for (int x = 0; x < MAX_PHYSICAL_IMAGES; ++x)
			material->textures[x] = DIRECTORY_MATERIAL + (material->textures[x]);
	}

	vec2 material_dimensions = vec2(0);
	{
		unique_lock<shared_mutex> surface_guard(material->m_mutex);

		bool success[MAX_PHYSICAL_IMAGES];
		vec2 dimensions[MAX_PHYSICAL_IMAGES];
		int dataSize[MAX_PHYSICAL_IMAGES];
		GLubyte *textureData[MAX_PHYSICAL_IMAGES];

		textureData[0] = dt_FreeImage::ReadImage_4channel(material->textures[0], dimensions[0], dataSize[0], success[0]);
		textureData[1] = dt_FreeImage::ReadImage_3channel(material->textures[1], dimensions[1], dataSize[1], success[1]);
		for (int x = 2; x < MAX_PHYSICAL_IMAGES; ++x)
			textureData[x] = dt_FreeImage::ReadImage_1channel(material->textures[x], dimensions[x], dataSize[x], success[x]);

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
			material->materialData = new GLubyte[material_data_size];
			GLubyte *materialData = material->materialData;

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
	material->size = material_dimensions;
	submitWorkorder(material);
	*complete = true;
}

namespace Asset_Manager {
	void load_asset(Shared_Asset_Material &user, const std::string(&textures)[6], const bool &threaded) {
		// Check if a copy already exists
		shared_mutex &mutex_IO_assets = getMutexIOAssets();
		auto &assets_materials = (fetchAssetList(Asset_Material::GetAssetType()));
		{
			shared_lock<shared_mutex> guard(mutex_IO_assets);
			for each (auto &asset in assets_materials) {
				bool identical = true;
				shared_lock<shared_mutex> asset_guard(asset->m_mutex);
				const Shared_Asset_Material derived_asset = dynamic_pointer_cast<Asset_Material>(asset);
				if (derived_asset) {
					for (int x = 0; x < 5; ++x) {
						if (derived_asset->textures[x] != textures[x]) {
							identical = false;
							break;
						}
					}
					if (identical) {
						asset_guard.unlock();
						asset_guard.release();
						user = derived_asset;
						if (!threaded)
							user->Finalize();
						return;
					}
				}
			}
		}

		{
			// Attempt to create the asset
			unique_lock<shared_mutex> guard(mutex_IO_assets);

			int array_spot = assets_materials.size();
			deque<int> &m_freed_material_spots = Material_Manager::getMatFreeSpots();
			if (m_freed_material_spots.size()) {
				array_spot = m_freed_material_spots.front();
				m_freed_material_spots.pop_front();
			}

			user = Shared_Asset_Material(new Asset_Material(textures, Material_Manager::getBufferSSBO(), array_spot));
			assets_materials.push_back(user);
		}

		// The texture ID of the surface that requested this texture has had it's ID pushed into this material's user list
		// We generate the texture object on the materials handle, and then when processing the work order we propagate the ID onto all the users
		bool *complete = new bool(false);
		if (threaded) {
			thread *import_thread = new thread(initialize_Material, user, "", complete);
			import_thread->detach();
			submitWorkthread(std::pair<thread*, bool*>(import_thread, complete));
		}
		else {
			initialize_Material(user, "", complete);
			user->Finalize();
		}
	}

	void load_asset(Shared_Asset_Material &user, const std::string &filename, const bool &threaded)
	{
		// Check if a copy already exists
		shared_mutex &mutex_IO_assets = getMutexIOAssets();
		auto &assets_materials = (fetchAssetList(Asset_Material::GetAssetType()));
		{
			shared_lock<shared_mutex> guard(mutex_IO_assets);
			for each (auto &asset in assets_materials) {
				bool identical = true;
				shared_lock<shared_mutex> asset_guard(asset->m_mutex);
				const Shared_Asset_Material derived_asset = dynamic_pointer_cast<Asset_Material>(asset);
				if (derived_asset) {
					if (derived_asset->material_filename == filename) {
						asset_guard.unlock();
						asset_guard.release();
						user = derived_asset;
						if (!threaded)
							user->Finalize();
						return;
					}
				}
			}
		}

		// Attempt to create the asset
		const std::string &fulldirectory = ABS_DIRECTORY_MATERIAL(filename);
		if (!fileOnDisk(fulldirectory) || (filename == "") || (filename == " ")) {
			MSG::Error(FILE_MISSING, fulldirectory);
			user = fetchDefaultMaterial();
			return;
		}

		{
			unique_lock<shared_mutex> guard(mutex_IO_assets);

			int array_spot = assets_materials.size();
			deque<int> &m_freed_material_spots = Material_Manager::getMatFreeSpots();
			if (m_freed_material_spots.size()) {
				array_spot = m_freed_material_spots.front();
				m_freed_material_spots.pop_front();
			}

			user = Shared_Asset_Material(new Asset_Material(filename, Material_Manager::getBufferSSBO(), array_spot));
			assets_materials.push_back(user);
		}

		// The texture ID of the surface that requested this texture has had it's ID pushed into this material's user list
		// We generate the texture object on the materials handle, and then when processing the work order we propagate the ID onto all the users
		bool *complete = new bool(false);
		if (threaded) {
			thread *import_thread = new thread(initialize_Material, user, fulldirectory, complete);
			import_thread->detach();
			submitWorkthread(std::pair<thread*, bool*>(import_thread, complete));
		}
		else {
			initialize_Material(user, fulldirectory, complete);
			user->Finalize();
		}
	}
}