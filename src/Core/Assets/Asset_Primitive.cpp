#include "Assets\Asset_Primitive.h"
#include "Managers\Message_Manager.h"
#include "Utilities\ModelImporter.h"
#include "ASSIMP\Importer.hpp"
#include "ASSIMP\postprocess.h"
#include "ASSIMP\scene.h"

/* -----ASSET TYPE----- */
#define ASSET_TYPE 5

using namespace Asset_Manager;

Asset_Primitive::~Asset_Primitive()
{
	if (finalized)
		glDeleteVertexArrays(1, &ID);
}

Asset_Primitive::Asset_Primitive()
{
	ID = 0;
	filename = "";
	finalized = false;
}

Asset_Primitive::Asset_Primitive(const string & _filename) : Asset_Primitive()
{
	filename = _filename;
}

int Asset_Primitive::GetAssetType()
{
	return ASSET_TYPE;
}

void Asset_Primitive::Finalize()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	if (!finalized) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_mutex);
		GLuint buffers[2];
		const size_t arraySize = data.size();

		glGenVertexArrays(1, &ID);
		glBindVertexArray(ID);
		glGenBuffers(2, buffers);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec3), &data[0][0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec2), &uv_data[0][0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindVertexArray(0);
		glDeleteBuffers(2, buffers);
		finalized = true;
	}
}

void Asset_Primitive::Bind()
{
	glBindVertexArray(ID);
}

void Asset_Primitive::Unbind()
{
	glBindVertexArray(0);
}

void Asset_Primitive::Draw()
{
	glBindVertexArray(ID);
	glDrawArrays(GL_TRIANGLES, 0, (int)data.size());
	glBindVertexArray(0);
}

size_t Asset_Primitive::GetSize() const
{
	return data.size();
}

// Forward declaration
Shared_Asset_Primitive fetchDefaultAsset();
// Forward declaration
void initialize_Primitive(Shared_Asset_Primitive &primitive, const string & filename, bool *complete);

// Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
// Will generate a default one itself if the default doesn't exist.
Shared_Asset_Primitive fetchDefaultAsset()
{
	shared_lock<shared_mutex> guard(getMutexIOAssets());
	std::map<int, Shared_Asset> &fallback_assets = getFallbackAssets();
	fallback_assets.insert(std::pair<int, Shared_Asset>(Asset_Primitive::GetAssetType(), Shared_Asset()));
	auto &default_asset = fallback_assets[Asset_Primitive::GetAssetType()];
	if (default_asset.get() == nullptr) { // Check if we already created the default asset
		default_asset = shared_ptr<Asset_Primitive>(new Asset_Primitive());
		Shared_Asset_Primitive cast_asset = dynamic_pointer_cast<Asset_Primitive>(default_asset);
		cast_asset->filename = "defaultPrimitive";
		if (fileOnDisk(ABS_DIRECTORY_PRIMITIVE("defaultPrimitive"))) { // Check if we have a default one on disk to load
			bool complete = false;
			initialize_Primitive(cast_asset, ABS_DIRECTORY_PRIMITIVE("defaultPrimitive"), &complete);
			cast_asset->Finalize();
			if (complete && cast_asset->ExistsYet()) // did we successfully load the default asset from disk?
				return cast_asset;
		}
		// We didn't load a default asset from disk
		/* HARD CODE DEFAULT VALUES HERE */
		cast_asset->data = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
		cast_asset->uv_data = vector<vec2>{ vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 0), vec2(1, 1), vec2(0, 1) };
		cast_asset->Finalize();
		return cast_asset;
	}
	return dynamic_pointer_cast<Asset_Primitive>(default_asset);
}

// Loads the model file from disk into memory
void initialize_Primitive(Shared_Asset_Primitive &primitive, const string & filename, bool *complete)
{
	vector<vec3> vertices;
	vector<vec2> uv_coords;
	if (!ModelImporter::Import_Model(filename, aiProcess_LimitBoneWeights | aiProcess_Triangulate, vertices, uv_coords)) {
		primitive = fetchDefaultAsset();
		return;
	}	
	Asset_Primitive *prim_ptr = primitive.get();
	{
		unique_lock<shared_mutex> model_guard(prim_ptr->m_mutex);
		prim_ptr->data = vertices;
		prim_ptr->uv_data = uv_coords;
	}

	submitWorkorder(primitive);
	*complete = true;
}

namespace Asset_Manager {
	void load_asset(Shared_Asset_Primitive &user, const string &filename, const bool &threaded)
	{
		// Check if a copy already exists
		shared_mutex &mutex_IO_assets = getMutexIOAssets();
		auto &assets_primitives = (fetchAssetList(Asset_Primitive::GetAssetType()));
		{
			shared_lock<shared_mutex> guard(mutex_IO_assets);
			for each (const auto &asset in assets_primitives) {
				shared_lock<shared_mutex> asset_guard(asset->m_mutex);
				const Shared_Asset_Primitive derived_asset = dynamic_pointer_cast<Asset_Primitive>(asset);
				if (derived_asset) {
					if (derived_asset->filename == filename) {
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

		const std::string &fulldirectory = ABS_DIRECTORY_PRIMITIVE(filename);
		{
			unique_lock<shared_mutex> guard(mutex_IO_assets);
			user = Shared_Asset_Primitive(new Asset_Primitive(filename));
			assets_primitives.push_back(user);
		}

		bool *complete = new bool(false);
		if (threaded) {
			thread *import_thread = new thread(initialize_Primitive, user, fulldirectory, complete);
			import_thread->detach();
			submitWorkthread(std::pair<thread*, bool*>(import_thread, complete));
		}
		else {
			initialize_Primitive(user, fulldirectory, complete);
			user->Finalize();
		}
	}
}