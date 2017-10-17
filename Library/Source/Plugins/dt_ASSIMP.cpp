#include "Plugins\dt_ASSIMP.h"
#include "Managers\Message_Manager.h"
#include "ASSIMP\Importer.hpp"
#include "ASSIMP\postprocess.h"
#include "ASSIMP\scene.h"

using namespace Asset_Manager;

void initialize_Primitive(Shared_Asset_Primitive &primitive, const string & filename, bool *complete)
{
	Assimp::Importer &importer = *new Assimp::Importer();
	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_LimitBoneWeights |
		aiProcess_Triangulate /* |
							  aiProcess_CalcTangentSpace |
							  aiProcess_SplitLargeMeshes |
							  aiProcess_OptimizeMeshes |
							  aiProcess_OptimizeGraph |
							  aiProcess_GenSmoothNormals
							  aiProcess_ImproveCacheLocality*/);
							  // Scene cannot be read
	if (!scene) {
		MSG::Error(FILE_CORRUPT, filename);
		//primitive = fetchDefaultAsset();
		return;
	}
	Asset_Primitive *prim_ptr = primitive.get();
	{
		unique_lock<shared_mutex> model_guard(prim_ptr->m_mutex);
		vector<vec3> &data = prim_ptr->data;
		vector<vec2> &uv_data = prim_ptr->uv_data;

		// Combine mesh data into single struct
		for (int a = 0, atotal = scene->mNumMeshes; a < atotal; a++) {
			const aiMesh *mesh = scene->mMeshes[a];
			for (int x = 0, faceCount = mesh->mNumFaces; x < faceCount; ++x) {
				const aiFace& face = mesh->mFaces[x];
				for (int b = 0, indCount = face.mNumIndices; b < indCount; ++b) {
					const int index = face.mIndices[b];
					const aiVector3D vertex = mesh->mVertices[index];
					const aiVector3D uvmap = mesh->HasTextureCoords(0) ? (mesh->mTextureCoords[0][index]) : aiVector3D(0, 0, 0);
					data.push_back(vec3(vertex.x, vertex.y, vertex.z));
					uv_data.push_back(vec2(uvmap.x, uvmap.y));
				}
			}
		}
	}

	submitWorkorder(primitive);
	*complete = true;
}

Shared_Asset_Primitive fetchDefaultPrimitive()
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
		cast_asset->data = vector<vec3> { vec3(-1, -1, 0), vec3(1, -1, 0), vec3 (1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
		cast_asset->uv_data = vector<vec2>{ vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 0), vec2(1, 1), vec2(0, 1) };
		cast_asset->Finalize();
		return cast_asset;
	}
	return dynamic_pointer_cast<Asset_Primitive>(default_asset);
}

namespace dt_ASSIMP {
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

		// Attempt to create the asset
		const std::string &fulldirectory = ABS_DIRECTORY_PRIMITIVE(filename);
		if (!fileOnDisk(fulldirectory)) {
			MSG::Error(FILE_MISSING, fulldirectory);
			user = fetchDefaultPrimitive();
			return;
		}
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