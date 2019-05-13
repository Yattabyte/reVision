#include "Managers/MaterialManager.h"
#include "Engine.h"


MaterialManager::~MaterialManager() 
{
}

MaterialManager::MaterialManager(Engine * engine)
{
	// Preference Values
	engine->getPreferenceState().getOrSetValue(PreferenceState::C_MATERIAL_SIZE, m_materialSize);

	// Size-dependent variable set up
	m_mipCount = GLsizei(floor(log2f(float(m_materialSize)) + 1.0f));
	glGetIntegerv(GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS, &m_textureLayers);

	// Initialize the material array
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_arrayID);
	glTextureParameterf(m_arrayID, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
	glTextureParameteri(m_arrayID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_arrayID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(m_arrayID, GL_TEXTURE_SPARSE_ARB, GL_TRUE);
	glTextureStorage3D(m_arrayID, m_mipCount, GL_RGBA16F, m_materialSize, m_materialSize, m_textureLayers);
	if (!glIsTexture(m_arrayID))
		engine->getManager_Messages().error("MaterialManager Texture array is incomplete.");
}

void MaterialManager::bind()
{
	glBindTextureUnit(0, m_arrayID);
}

const GLsizei MaterialManager::getMaterialSize() const
{
	return m_materialSize;
}

GLuint MaterialManager::generateID(const size_t & textureCount)
{
	std::unique_lock<std::shared_mutex> writeGuard(m_DataMutex);
	GLuint arraySpot = (GLuint)m_Count;
	m_Count += textureCount;
	return arraySpot;
}

void MaterialManager::writeMaterials(const GLuint & materialID, const GLubyte * imageData, const GLsizei & imageCount)
{
	std::unique_lock<std::shared_mutex> writeGuard(m_DataMutex);
	GLuint pboID = 0;
	glCreateBuffers(1, &pboID);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboID);
	glNamedBufferStorage(pboID, m_materialSize * m_materialSize * imageCount * 4, imageData, 0);
	for (int x = 0; x < m_mipCount; ++x) {
		const GLsizei mipsize = (GLsizei)std::max(1.0f, (floor(m_materialSize / pow(2.0f, (float)x))));
		glTexturePageCommitmentEXT(m_arrayID, x, 0, 0, materialID, mipsize, mipsize, imageCount, GL_TRUE);
	}
	glTextureSubImage3D(m_arrayID, 0, 0, 0, materialID, m_materialSize, m_materialSize, imageCount, GL_RGBA, GL_UNSIGNED_BYTE, (void *)0);
	glGenerateTextureMipmap(m_arrayID);
	glDeleteBuffers(1, &pboID);
	m_fences.push_back(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
	m_changed = true;
}

const bool MaterialManager::readyToUse()
{
	std::unique_lock<std::shared_mutex> writeGuard(m_DataMutex);
	// Check if we have a fence
	while (m_fences.size()) {
		// Check if the fence has passed
		auto fence = m_fences.front();
		const GLenum state = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
		if (state != GL_SIGNALED && state != GL_ALREADY_SIGNALED && state != GL_CONDITION_SATISFIED)
			return false;
		// Delete fence so we can skip these 2 branches next time
		glDeleteSync(fence);
		m_fences.pop_front();
	}
	return true;
}

const bool MaterialManager::hasChanged()
{
	// Changes every time materials are written to this material manager
	std::shared_lock<std::shared_mutex> readGuard(m_DataMutex);
	bool state = m_changed;
	m_changed = false;
	return state;
}
