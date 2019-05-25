#include "Assets/Texture.h"
#include "Engine.h"


constexpr char* DIRECTORY_TEXTURE = "\\Textures\\";

Shared_Texture::Shared_Texture(Engine * engine, const std::string & filename, const GLuint & type, const bool & mipmap, const bool & anis, const bool & threaded)
{
	(*(std::shared_ptr<Texture>*)(this)) = std::dynamic_pointer_cast<Texture>(
		engine->getManager_Assets().shareAsset(
			typeid(Texture).name(),
			filename,
			[engine, filename, type, mipmap, anis]() { return std::make_shared<Texture>(engine, filename, type, mipmap, anis); },
			threaded
		));
}

Texture::~Texture()
{
	if (existsYet()) {
		glDeleteBuffers(1, &m_pboID);
		glDeleteTextures(1, &m_glTexID);
	}
}

Texture::Texture(Engine * engine, const std::string & filename) : Asset(engine, filename) {}

Texture::Texture(Engine * engine, const std::string & filename, const GLuint & t, const bool & m, const bool & a) : Texture(engine, filename)
{
	m_type = t;
	m_mipmap = m;
	m_anis = a;
}

void Texture::initialize()
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
			glNamedBufferStorage(m_pboID, m_image->m_size.x * 4, m_image->m_pixelData, 0);
			glTextureStorage1D(m_glTexID, 1, GL_RGBA16F, m_image->m_size.x);
			glTextureSubImage1D(m_glTexID, 0, 0, m_image->m_size.x, GL_RGBA, GL_UNSIGNED_BYTE, (void *)0);
			glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		}
		case GL_TEXTURE_2D: {
			glNamedBufferStorage(m_pboID, m_image->m_size.x * m_image->m_size.y * 4, m_image->m_pixelData, 0);
			glTextureStorage2D(m_glTexID, 1, GL_RGBA16F, m_image->m_size.x, m_image->m_size.y);
			glTextureSubImage2D(m_glTexID, 0, 0, 0, m_image->m_size.x, m_image->m_size.y, GL_RGBA, GL_UNSIGNED_BYTE, (void *)0);
			if (m_anis)
				glTextureParameterf(m_glTexID, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
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
			glNamedBufferStorage(m_pboID, m_image->m_size.x * m_image->m_size.y * 4, m_image->m_pixelData, 0);
			glTextureStorage3D(m_glTexID, 1, GL_RGBA16F, m_image->m_size.x, m_image->m_size.y, 0);
			glTextureSubImage3D(m_glTexID, 0, 0, 0, 0, m_image->m_size.x, m_image->m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)0);
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

void Texture::bind(const unsigned int & texture_unit)
{
	glBindTextureUnit(texture_unit, m_glTexID);
}