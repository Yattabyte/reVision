#include "Modules/Graphics/Geometry/Prop/PropUpload_System.h"
#include "Modules/Graphics/Geometry/Prop/PropData.h"
#include "Modules/ECS/component_types.h"
#include "Engine.h"


PropUpload_System::~PropUpload_System() 
{
	glDeleteBuffers(1, &m_vboID);
	glDeleteVertexArrays(1, &m_vaoID);
	for (const auto& [pboID, fence] : m_pixelBuffers)
		glDeleteBuffers(1, &pboID);
}

PropUpload_System::PropUpload_System(Engine& engine, PropData& frameData) :
	m_engine(engine),
	m_frameData(frameData)
{
	addComponentType(Prop_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);

	// Create VBO's
	glCreateBuffers(1, &m_vboID);
	glNamedBufferStorage(m_vboID, 1, nullptr, GL_DYNAMIC_STORAGE_BIT);
	// Create VAO
	glCreateVertexArrays(1, &m_vaoID);
	// Enable 7 attribute locations which all source data from binding point 0
	for (unsigned int x = 0; x < NUM_VERTEX_ATTRIBUTES; ++x) {
		glEnableVertexArrayAttrib(m_vaoID, x);
		glVertexArrayAttribBinding(m_vaoID, x, 0);
	}
	// Specify how the data should be broken up, and into what layout locations in the shaders (7 attribute locations)
	glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, vertex));
	glVertexArrayAttribFormat(m_vaoID, 1, 3, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, normal));
	glVertexArrayAttribFormat(m_vaoID, 2, 3, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, tangent));
	glVertexArrayAttribFormat(m_vaoID, 3, 3, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, bitangent));
	glVertexArrayAttribFormat(m_vaoID, 4, 2, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, uv));
	glVertexArrayAttribIFormat(m_vaoID, 5, 1, GL_INT, offsetof(SingleVertex, matID));
	glVertexArrayAttribIFormat(m_vaoID, 6, 4, GL_INT, offsetof(SingleVertex, boneIDs));
	glVertexArrayAttribFormat(m_vaoID, 7, 4, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, weights));
	// Specify data from the one vertex buffer to binding point 0
	glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(SingleVertex));

	// Share VAO for rendering purposes
	frameData.m_geometryVAOID = m_vaoID;

	// Preference Values
	engine.getPreferenceState().getOrSetValue(PreferenceState::Preference::C_MATERIAL_SIZE, m_materialSize);

	// Size-dependent variable set up
	m_maxMips = GLsizei(floor(log2f(float(m_materialSize)) + 1.0F));
	glGetIntegerv(GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS, &m_maxTextureLayers);

	// Initialize the material array
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_matID);
	glTextureParameterf(m_matID, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0F);
	glTextureParameteri(m_matID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_matID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(m_matID, GL_TEXTURE_SPARSE_ARB, GL_TRUE);
	glTextureStorage3D(m_matID, m_maxMips, GL_RGBA16F, m_materialSize, m_materialSize, m_maxTextureLayers);

	// Share material array for rendering purposes
	frameData.m_materialArrayID = m_matID;

	// Initialize all our pixel buffers
	for (auto& [pboID, fence] : m_pixelBuffers) {
		pboID = 0ULL;
		fence = nullptr;
		glCreateBuffers(1, &pboID); 
		glNamedBufferStorage(pboID, size_t(m_materialSize) * size_t(m_materialSize) * MAX_DIGITAL_IMAGES * 4ULL, nullptr, GL_DYNAMIC_STORAGE_BIT);
	}
}

void PropUpload_System::updateComponents(const float& /*deltaTime*/, const std::vector<std::vector<ecsBaseComponent*>>& components)
{
	for (const auto& componentParam : components) {
		auto* propComponent = static_cast<Prop_Component*>(componentParam[0]);
		auto& model = propComponent->m_model;

		// Try to upload model data
		if (!model)
			model = Shared_Model(m_engine, propComponent->m_modelName);
		if (!propComponent->m_uploadModel && model->ready()) {
			tryInsertModel(model);
			propComponent->m_offset = m_modelMap[model].first;
			propComponent->m_count = m_modelMap[model].second;
			propComponent->m_uploadModel = true;
		}

		// Try to upload material data
		if (!propComponent->m_uploadMaterial && Asset::All_Ready(model, model->m_materialArray)) {
			// Get spot in the material array
			tryInsertMaterial(model->m_materialArray);
			propComponent->m_materialID = m_materialMap[model->m_materialArray];

			// Prepare fence
			propComponent->m_uploadMaterial = true;
		}
	}
}

void PropUpload_System::tryInsertModel(const Shared_Model& model) 
{
	if (m_modelMap.find(model) == m_modelMap.end()) {
		// Prop hasn't been uploaded yet
		const size_t arraySize = model->m_data.m_vertices.size() * sizeof(SingleVertex);
		// Check if we can fit the desired data
		waitOnFence();
		tryToExpand(arraySize);
		const auto offset = static_cast<GLuint>(m_currentSize / sizeof(SingleVertex));
		const auto count = static_cast<GLuint>(arraySize / sizeof(SingleVertex));

		// Upload vertex data
		glNamedBufferSubData(m_vboID, m_currentSize, arraySize, &model->m_data.m_vertices[0]);
		m_currentSize += arraySize;

		// Prepare fence
		m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		m_modelMap[model] = { offset, count };
	}
}

void PropUpload_System::waitOnFence() noexcept 
{
	if (m_fence != nullptr) {
		// Wait for data fence to be passed
		GLenum state = GL_UNSIGNALED;
		while (state != GL_SIGNALED && state != GL_ALREADY_SIGNALED && state != GL_CONDITION_SATISFIED)
			state = glClientWaitSync(m_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		glDeleteSync(m_fence);
		m_fence = nullptr;
	}
}

void PropUpload_System::tryToExpand(const size_t& arraySize) noexcept 
{
	if (m_currentSize + arraySize > m_maxCapacity) {
		// Create new set of VBO's large enough to fit old data + desired data
		m_maxCapacity += arraySize * 2;

		// Create the new VBO's
		GLuint newVBOID = 0;
		glCreateBuffers(1, &newVBOID);
		glNamedBufferStorage(newVBOID, m_maxCapacity, nullptr, GL_DYNAMIC_STORAGE_BIT);

		// Copy old VBO's
		auto fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glCopyNamedBufferSubData(m_vboID, newVBOID, 0, 0, m_currentSize);

		// Delete the old VBO's
		GLenum state = GL_UNSIGNALED;
		while (state != GL_SIGNALED && state != GL_ALREADY_SIGNALED && state != GL_CONDITION_SATISFIED)
			state = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		glDeleteSync(fence);
		glDeleteBuffers(1, &m_vboID);

		// Overwrite old VBO ID's
		m_vboID = newVBOID;

		// Assign VAO to new VBO's
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(SingleVertex));
	}
}

void PropUpload_System::tryInsertMaterial(const Shared_Material& material)
{
	if (m_materialMap.find(material) == m_materialMap.end()) {
		// Get spot in the material array
		const auto imageCount = static_cast<GLsizei>((material->m_textures.size() / MAX_PHYSICAL_IMAGES) * MAX_DIGITAL_IMAGES);
		const auto materialID = static_cast<GLuint>(m_matCount);
		m_materialMap[material] = materialID;
		m_matCount += imageCount;
		if (m_matCount >= m_maxTextureLayers)
			m_engine.getManager_Messages().error("Out of room for more materials!");

		// Try to upload the images piece-meal
		size_t offset(0ULL);
		const auto materialCount = int(material->m_textures.size() / MAX_PHYSICAL_IMAGES);
		for (int x = 0; x < materialCount; ++x) {
			// Find a free pixel buffer
			const auto [pboID, fence] = getFreePBO();
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, *pboID);
			glNamedBufferSubData(*pboID, 0, size_t(m_materialSize) * size_t(m_materialSize) * MAX_DIGITAL_IMAGES * 4ULL, &material->m_materialData[offset]);

			// Upload material data
			for (int m = 0; m < m_maxMips; ++m) {
				const GLsizei mipsize = static_cast<GLsizei>(std::max(1.0F, (floor(m_materialSize / pow(2.0F, static_cast<float>(m))))));
				glTexturePageCommitmentEXT(m_matID, m, 0, 0, materialID, mipsize, mipsize, imageCount, GL_TRUE);
			}
			glTextureSubImage3D(m_matID, 0, 0, 0, materialID + (x * MAX_DIGITAL_IMAGES), m_materialSize, m_materialSize, MAX_DIGITAL_IMAGES, GL_RGBA, GL_UNSIGNED_BYTE, (void*)nullptr);
			offset += size_t(m_materialSize) * size_t(m_materialSize) * MAX_DIGITAL_IMAGES * 4ULL;

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			*fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		}
		glGenerateTextureMipmap(m_matID);
	}
}

std::pair<GLuint*, GLsync*> PropUpload_System::getFreePBO() noexcept
{
	GLenum state = GL_UNSIGNALED;
	while (state != GL_SIGNALED && state != GL_ALREADY_SIGNALED && state != GL_CONDITION_SATISFIED) {
		for (auto& [pboID, fence] : m_pixelBuffers) {
			// Check if fence is free
			if (fence == nullptr)
				return { &pboID, &fence };
			// Check if fence has passed
			state = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
			if (state == GL_SIGNALED || state == GL_ALREADY_SIGNALED || state == GL_CONDITION_SATISFIED) {
				glDeleteSync(fence);
				fence = nullptr;
				return { &pboID, &fence };
			}
		}
	}
	return { nullptr, nullptr };
}

void PropUpload_System::clear() noexcept 
{
	// Reset size, and half the capacity
	m_currentSize = 0ULL;
	m_maxCapacity = std::max<size_t>(256ULL, m_maxCapacity / 2ULL);
	m_modelMap.clear();

	// Replace old VBO
	waitOnFence();
	GLuint newVBOID = 0;
	glCreateBuffers(1, &newVBOID);
	glNamedBufferStorage(newVBOID, m_maxCapacity, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glDeleteBuffers(1, &m_vboID);
	m_vboID = newVBOID;
	glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(SingleVertex));

	// Reset materials
	m_matCount = 0;
	m_materialMap.clear();
}