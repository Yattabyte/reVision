#include "Assets\Asset_Primitive.h"
#include "Systems\Message_Manager.h"
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
		glDeleteBuffers(2, buffers);
}

Asset_Primitive::Asset_Primitive()
{
	filename = "";
	finalized = false;
	for each (auto &buffer in buffers)
		buffer = -1;
}

Asset_Primitive::Asset_Primitive(const string & _filename) : Asset_Primitive()
{
	filename = _filename;
}

int Asset_Primitive::GetAssetType()
{
	return ASSET_TYPE;
}

GLuint Asset_Primitive::GenerateVAO()
{
	GLuint vaoID = 0;

	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	return vaoID;
}

void Asset_Primitive::UpdateVAO(const GLuint & vaoID)
{
	shared_lock<shared_mutex> guard(m_mutex);
	glBindVertexArray(vaoID);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
}

size_t Asset_Primitive::GetSize() const
{
	return data.size();
}

// Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
// Will generate a default one itself if the default doesn't exist.
Shared_Asset_Primitive fetchDefaultAsset()
{
	shared_lock<shared_mutex> guard(Asset_Managera::GetMutex_Assets());
	std::map<int, Shared_Asset> &fallback_assets = Asset_Managera::GetFallbackAssets_Map();
	fallback_assets.insert(std::pair<int, Shared_Asset>(Asset_Primitive::GetAssetType(), Shared_Asset()));
	auto &default_asset = fallback_assets[Asset_Primitive::GetAssetType()];
	if (default_asset.get() == nullptr) { // Check if we already created the default asset
		default_asset = shared_ptr<Asset_Primitive>(new Asset_Primitive());
		Shared_Asset_Primitive cast_asset = dynamic_pointer_cast<Asset_Primitive>(default_asset);
		cast_asset->filename = "defaultPrimitive";
		string fulldirectory = ABS_DIRECTORY_PRIMITIVE("defaultPrimitive");
		Primitive_WorkOrder work_order(cast_asset, fulldirectory);
		if (FileReader::FileExistsOnDisk(fulldirectory)) { // Check if we have a default one on disk to load
			work_order.Initialize_Order();
			work_order.Finalize_Order();
			if (cast_asset->ExistsYet()) // did we successfully load the default asset from disk?
				return cast_asset;
		}
		// We didn't load a default asset from disk
		/* HARD CODE DEFAULT VALUES HERE */
		cast_asset->data = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
		cast_asset->uv_data = vector<vec2>{ vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 0), vec2(1, 1), vec2(0, 1) };
		work_order.Finalize_Order();
		return cast_asset;
	}
	return dynamic_pointer_cast<Asset_Primitive>(default_asset);
}

namespace Asset_Manager {
	void load_asset(Shared_Asset_Primitive &user, const string &filename, const bool &threaded)
	{
		// Check if a copy already exists
		shared_mutex &mutex_IO_assets = Asset_Managera::GetMutex_Assets();
		auto &assets_primitives = (Asset_Managera::GetAssets_List(Asset_Primitive::GetAssetType()));
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

		if (threaded)
			Asset_Managera::AddWorkOrder(new Primitive_WorkOrder(user, fulldirectory));
		else {
			Primitive_WorkOrder work_order(user, fulldirectory);
			work_order.Initialize_Order();
			work_order.Finalize_Order();
		}
	}
}

void Primitive_WorkOrder::Initialize_Order()
{
	vector<vec3> vertices;
	vector<vec2> uv_coords;
	if (!ModelImporter::Import_Model(m_filename, aiProcess_LimitBoneWeights | aiProcess_Triangulate, vertices, uv_coords)) {
		m_asset = fetchDefaultAsset();
		return;
	}

	unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
	m_asset->data = vertices;
	m_asset->uv_data = uv_coords;
}

void Primitive_WorkOrder::Finalize_Order()
{
	shared_lock<shared_mutex> read_guard(m_asset->m_mutex);
	if (!m_asset->ExistsYet()) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_asset->m_mutex);

		auto &data = m_asset->data;
		auto &uv_data = m_asset->uv_data;
		auto &buffers = m_asset->buffers;
		const size_t &arraySize = data.size();

		glGenBuffers(2, buffers);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec3), &data[0][0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec2), &uv_data[0][0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		m_asset->Finalize();
	}
}
