#include "Assets/Texture.h"
#include "Engine.h"


constexpr const char* DIRECTORY_TEXTURE = "\\Textures\\";
constexpr const float MAX_ANISOTROPY = 16.0F;

Shared_Texture::Shared_Texture(Engine& engine, const std::string& filename, const GLuint& type, const bool& mipmap, const bool& anisotropy, const bool& threaded) noexcept
{
	(*(std::shared_ptr<Texture>*)(this)) = std::dynamic_pointer_cast<Texture>(
		engine.getManager_Assets().shareAsset(
			typeid(Texture).name(),
			filename,
			[&engine, filename, type, mipmap, anisotropy]() { return std::make_shared<Texture>(engine, filename, type, mipmap, anisotropy); },
			threaded
		));
}

Texture::~Texture() noexcept
{
	if (existsYet()) {
		glDeleteBuffers(1, &m_pboID);
		glDeleteTextures(1, &m_glTexID);
	}
}

Texture::Texture(Engine& engine, const std::string& filename) noexcept : Asset(engine, filename) {}

Texture::Texture(Engine& engine, const std::string& filename, const GLuint& type, const bool& mipmap, const bool& anisotropy) noexcept :
	Asset(engine, filename),
	m_type(type),
	m_mipmap(mipmap),
	m_anis(anisotropy)
{
}

void Texture::initialize() noexcept
{
	// Forward asset creation
	m_image = Shared_Image(m_engine, DIRECTORY_TEXTURE + getFileName(), {}, false);

	// Create Texture
	glCreateTextures(m_type, 1, &m_glTexID);
	glCreateBuffers(1, &m_pboID);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboID);

	// Load Texture
	switch (m_type) {
	case GL_TEXTURE_1D: {
		glNamedBufferStorage(m_pboID, GLsizeiptr(m_image->m_size.x) * 4LL, m_image->m_pixelData, 0);
		glTextureStorage1D(m_glTexID, 1, GL_RGBA16F, m_image->m_size.x);
		glTextureSubImage1D(m_glTexID, 0, 0, m_image->m_size.x, GL_RGBA, GL_UNSIGNED_BYTE, (void*)nullptr);
		glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		break;
	}
	case GL_TEXTURE_2D: {
		glNamedBufferStorage(m_pboID, GLsizeiptr(m_image->m_size.x) * GLsizeiptr(m_image->m_size.y) * 4LL, m_image->m_pixelData, 0);
		glTextureStorage2D(m_glTexID, 1, GL_RGBA16F, m_image->m_size.x, m_image->m_size.y);
		glTextureSubImage2D(m_glTexID, 0, 0, 0, m_image->m_size.x, m_image->m_size.y, GL_RGBA, GL_UNSIGNED_BYTE, (void*)nullptr);
		if (m_anis)
			glTextureParameterf(m_glTexID, GL_TEXTURE_MAX_ANISOTROPY_EXT, MAX_ANISOTROPY);
		if (m_mipmap) {
			glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateTextureMipmap(m_glTexID);
		}
		else {
			glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		break;
	}
	case GL_TEXTURE_2D_ARRAY: {
		glNamedBufferStorage(m_pboID, GLsizeiptr(m_image->m_size.x) * GLsizeiptr(m_image->m_size.y) * 4LL, m_image->m_pixelData, 0);
		glTextureStorage3D(m_glTexID, 1, GL_RGBA16F, m_image->m_size.x, m_image->m_size.y, 0);
		glTextureSubImage3D(m_glTexID, 0, 0, 0, 0, m_image->m_size.x, m_image->m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)nullptr);
		glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateTextureMipmap(m_glTexID);
		break;
	}
	};
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	// Finalize
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	Asset::finalize();
}

void Texture::bind(const unsigned int& texture_unit) noexcept
{
	glBindTextureUnit(texture_unit, m_glTexID);
}