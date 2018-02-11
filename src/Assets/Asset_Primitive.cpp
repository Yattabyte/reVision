#include "Assets\Asset_Primitive.h"
#include "Managers\Message_Manager.h"
#include "Utilities\Model_Importer.h"
#include "ASSIMP\Importer.hpp"
#include "ASSIMP\postprocess.h"
#include "ASSIMP\scene.h"

/* -----ASSET TYPE----- */
#define ASSET_TYPE 5

using namespace Asset_Loader;

Asset_Primitive::~Asset_Primitive()
{
	if (ExistsYet())
		glDeleteBuffers(2, buffers);
	if (m_fence != nullptr)
		glDeleteSync(m_fence);
}

Asset_Primitive::Asset_Primitive(const string & filename) : Asset(filename)
{
	for each (auto &buffer in buffers)
		buffer = -1;
	m_fence = nullptr;
}

int Asset_Primitive::GetAssetType()
{
	return ASSET_TYPE;
}

bool Asset_Primitive::ExistsYet()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	if (Asset::ExistsYet() && m_fence != nullptr) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_mutex);
		const auto state = glClientWaitSync(m_fence, 0, 0);
		if (((state == GL_ALREADY_SIGNALED) || (state == GL_CONDITION_SATISFIED))
			&& (state != GL_WAIT_FAILED))
			return true;
	}
	return false;
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

size_t Asset_Primitive::GetSize()
{
	shared_lock<shared_mutex> guard(m_mutex);
	return data.size();
}

// Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
// Uses hardcoded values
void fetchDefaultAsset(Shared_Asset_Primitive & asset)
{
	// Check if a copy already exists
	if (Asset_Manager::query_Existing_Asset<Asset_Primitive>(asset, "defaultPrimitive"))
		return;

	// Create hardcoded alternative
	Asset_Manager::create_New_Asset<Asset_Primitive>(asset, "defaultPrimitive");
	asset->data = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	asset->uv_data = vector<vec2>{ vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 0), vec2(1, 1), vec2(0, 1) };
	Asset_Manager::add_Work_Order(new Primitive_WorkOrder(asset, ""), true);
}

namespace Asset_Loader {
	void load_asset(Shared_Asset_Primitive & user, const string & filename, const bool & threaded)
	{
		// Check if a copy already exists
		if (Asset_Manager::query_Existing_Asset<Asset_Primitive>(user, filename))
			return;

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = ABS_DIRECTORY_PRIMITIVE(filename);
		if (!File_Reader::FileExistsOnDisk(fullDirectory)) {
			MSG::Error(FILE_MISSING, fullDirectory);
			fetchDefaultAsset(user);
			return;
		}

		// Create the asset
		Asset_Manager::submit_New_Asset<Asset_Primitive, Primitive_WorkOrder>(user, threaded, fullDirectory, filename);
	}
}

void Primitive_WorkOrder::Initialize_Order()
{
	vector<vec3> vertices;
	vector<vec2> uv_coords;
	if (!Model_Importer::import_Model(m_filename, aiProcess_LimitBoneWeights | aiProcess_Triangulate, vertices, uv_coords)) {
		fetchDefaultAsset(m_asset);
		return;
	}

	unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
	m_asset->data = vertices;
	m_asset->uv_data = uv_coords;
}

void Primitive_WorkOrder::Finalize_Order()
{
	if (!m_asset->ExistsYet()) {
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
		m_asset->m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glFlush();

		write_guard.unlock();
		write_guard.release();
		m_asset->Finalize();
	}
}
