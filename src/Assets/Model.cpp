#include "Assets/Model.h"
#include "Engine.h"


constexpr const char* DIRECTORY_MODEL = "\\Models\\";

Shared_Model::Shared_Model(Engine* engine, const std::string& filename, const bool& threaded) noexcept
{
	(*(std::shared_ptr<Model>*)(this)) = std::dynamic_pointer_cast<Model>(
		engine->getManager_Assets().shareAsset(
			typeid(Model).name(),
			filename,
			[engine, filename]() { return std::make_shared<Model>(engine, filename); },
			threaded
		));
}

Model::Model(Engine* engine, const std::string& filename) noexcept : Asset(engine, filename) {}

void Model::initialize() noexcept
{
	// Forward asset creation
	m_mesh = Shared_Mesh(m_engine, DIRECTORY_MODEL + getFileName(), false);

	// Generate all the required skins
	loadMaterial(DIRECTORY_MODEL + getFileName(), m_materialArray, m_mesh->m_geometry.materials);

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
		m_data.m_vertices[x].matID = (m_mesh->m_geometry.materialIndices[x] * 3);
	}

	// Calculate the mesh's min, max, center, and radius
	calculateAABB(m_data.m_vertices, m_bboxMin, m_bboxMax, m_bboxScale, m_bboxCenter, m_radius);

	// Finalize
	Asset::finalize();
}

void Model::calculateAABB(const std::vector<SingleVertex>& mesh, glm::vec3& minOut, glm::vec3& maxOut, glm::vec3& scaleOut, glm::vec3& centerOut, float& radiusOut) noexcept
{
	if (!mesh.empty()) {
		const auto& vector = mesh[0].vertex;
		auto minX = vector.x, maxX = vector.x, minY = vector.y, maxY = vector.y, minZ = vector.z, maxZ = vector.z;
		for (size_t x = 1, total = mesh.size(); x < total; ++x) {
			const glm::vec3& vertex = mesh[x].vertex;
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
		scaleOut = (maxOut - minOut) / 2.0f;
		centerOut = ((maxOut - minOut) / 2.0f) + minOut;
		radiusOut = glm::distance(minOut, maxOut) / 2.0f;
	}
}

void Model::loadMaterial(const std::string& relativePath, Shared_Material& modelMaterial, const std::vector<Material_Strings>& materials) noexcept
{
	// Retrieve texture directories from the mesh file
	const size_t slash1Index = relativePath.find_last_of('/');
	const size_t slash2Index = relativePath.find_last_of('\\');
	const size_t furthestFolderIndex = std::max(slash1Index != std::string::npos ? slash1Index : 0, slash2Index != std::string::npos ? slash2Index : 0);
	const std::string meshDirectory = relativePath.substr(0, furthestFolderIndex + 1);
	std::vector<std::string> textures(materials.size() * (size_t)MAX_PHYSICAL_IMAGES);
	for (size_t tx = 0, mx = 0; tx < textures.size() && mx < materials.size(); tx += MAX_PHYSICAL_IMAGES, ++mx) {
		if (!materials[mx].albedo.empty())
			textures[tx + 0] = meshDirectory + materials[mx].albedo;
		if (!materials[mx].normal.empty())
			textures[tx + 1] = meshDirectory + materials[mx].normal;
		if (!materials[mx].metalness.empty())
			textures[tx + 2] = meshDirectory + materials[mx].metalness;
		if (!materials[mx].roughness.empty())
			textures[tx + 3] = meshDirectory + materials[mx].roughness;
		if (!materials[mx].height.empty())
			textures[tx + 4] = meshDirectory + materials[mx].height;
		if (!materials[mx].albedo.empty())
			textures[tx + 5] = meshDirectory + materials[mx].ao;
	}

	// Attempt to find a .mat file if it exists
	std::string materialFilename = relativePath.substr(0, relativePath.find_first_of('.'));
	modelMaterial = Shared_Material(m_engine, materialFilename, textures);
}