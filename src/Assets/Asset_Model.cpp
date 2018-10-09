#include "Assets\Asset_Model.h"
#include "Engine.h"


constexpr char* DIRECTORY_MODEL = "\\Models\\";

Asset_Model::~Asset_Model()
{
	if (existsYet())
		m_modelManager->unregisterGeometry(m_data, m_offset, m_count);
}

Asset_Model::Asset_Model(const std::string & filename, ModelManager & modelManager) : Asset(filename), m_modelManager(&modelManager) {}

Shared_Asset_Model Asset_Model::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	return engine->getAssetManager().createAsset<Asset_Model>(
		filename,
		DIRECTORY_MODEL,
		"",
		&initialize,
		engine,
		threaded,
		engine->getModelManager()
	);
}

void Asset_Model::initialize(Engine * engine, const std::string & relativePath)
{
	// Forward asset creation
	m_mesh = Asset_Mesh::Create(engine, relativePath, false);

	// Generate all the required skins
	loadMaterial(engine, relativePath, m_materialArray, m_mesh->m_geometry.materials);

	const size_t vertexCount = m_mesh->m_geometry.vertices.size();
	m_data.m_vertices.resize(vertexCount);
	for (size_t x = 0; x < vertexCount; ++x) {
		m_data.m_vertices[x].vertex = m_mesh->m_geometry.vertices[x];
		m_data.m_vertices[x].normal = m_mesh->m_geometry.normals[x];
		m_data.m_vertices[x].tangent = m_mesh->m_geometry.tangents[x];
		m_data.m_vertices[x].bitangent = m_mesh->m_geometry.bitangents[x];
		m_data.m_vertices[x].uv = m_mesh->m_geometry.texCoords[x];
		m_data.m_vertices[x].boneIDs.x = m_mesh->m_geometry.bones[x].IDs[0];
		m_data.m_vertices[x].boneIDs.y = m_mesh->m_geometry.bones[x].IDs[1];
		m_data.m_vertices[x].boneIDs.z = m_mesh->m_geometry.bones[x].IDs[2];
		m_data.m_vertices[x].boneIDs.w = m_mesh->m_geometry.bones[x].IDs[3];
		m_data.m_vertices[x].weights.x = m_mesh->m_geometry.bones[x].Weights[0];
		m_data.m_vertices[x].weights.y = m_mesh->m_geometry.bones[x].Weights[1];
		m_data.m_vertices[x].weights.z = m_mesh->m_geometry.bones[x].Weights[2];
		m_data.m_vertices[x].weights.w = m_mesh->m_geometry.bones[x].Weights[3];
		m_data.m_vertices[x].matID = m_materialArray->m_matSpot;
	}

	// Calculate the mesh's min, max, center, and radius
	calculateAABB(m_data.m_vertices, m_bboxMin, m_bboxMax, m_bboxCenter, m_radius);
	
	// Register geometry
	m_modelManager->registerGeometry(m_data, m_offset, m_count);

	// Finalize
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	Asset::finalize(engine);
}

void Asset_Model::calculateAABB(const std::vector<SingleVertex>& mesh, glm::vec3 & minOut, glm::vec3 & maxOut, glm::vec3 & centerOut, float & radiusOut)
{
	if (mesh.size() >= 1) {
		const glm::vec3 & vector = mesh[0].vertex;
		float minX = vector.x, maxX = vector.x, minY = vector.y, maxY = vector.y, minZ = vector.z, maxZ = vector.z;
		for (size_t x = 1, total = mesh.size(); x < total; ++x) {
			const glm::vec3 &vertex = mesh[x].vertex;
			if (vertex.x < minX)
				minX = vertex.x;
			else if (vertex.x > maxX)
				maxX = vertex.x;
			if (vertex.y < minY)
				minY = vertex.y;
			else if (vertex.y > maxY)
				maxY = vertex.y;
			if (vertex.z < minZ)
				minZ = vertex.z;
			else if (vertex.z > maxZ)
				maxZ = vertex.z;
		}

		minOut = glm::vec3(minX, minY, minZ);
		maxOut = glm::vec3(maxX, maxY, maxZ);
		centerOut = ((maxOut - minOut) / 2.0f) + minOut;
		radiusOut = glm::distance(minOut, maxOut) / 2.0f;
	}
}

void Asset_Model::loadMaterial(Engine * engine, const std::string & relativePath, Shared_Asset_Material & modelMaterial, const std::vector<Material>& materials)
{
	// Retrieve texture directories from the mesh file
	const size_t slash1Index = relativePath.find_last_of('/'), slash2Index = relativePath.find_last_of('\\');
	const size_t furthestFolderIndex = std::max(slash1Index != std::string::npos ? slash1Index : 0, slash2Index != std::string::npos ? slash2Index : 0);
	const std::string meshDirectory = relativePath.substr(0, furthestFolderIndex + 1);
	std::vector<std::string> textures(materials.size() * (size_t)MAX_PHYSICAL_IMAGES);
	for (size_t tx = 0, mx = 0; tx < textures.size() && mx < materials.size(); tx += MAX_PHYSICAL_IMAGES, ++mx) {
		textures[tx + 0] = meshDirectory + materials[mx].albedo;
		textures[tx + 1] = meshDirectory + materials[mx].normal;
		textures[tx + 2] = meshDirectory + materials[mx].metalness;
		textures[tx + 3] = meshDirectory + materials[mx].roughness;
		textures[tx + 4] = meshDirectory + materials[mx].height;
		textures[tx + 5] = meshDirectory + materials[mx].ao;
	}

	// Attempt to find a .mat file if it exists
	std::string materialFilename = relativePath.substr(0, relativePath.find_first_of("."));
	modelMaterial = Asset_Material::Create(engine, materialFilename, textures);
}