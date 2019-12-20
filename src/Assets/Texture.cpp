#include "Assets/Texture.h"
#include "Engine.h"


constexpr const char* DIRECTORY_TEXTURE = "\\Textures\\";
constexpr const float MAX_ANISOTROPY = 16.0F;

Shared_Texture::Shared_Texture(Engine& engine, const std::string& filename, const GLuint& type, const bool& mipmap, const bool& anisotropy, const bool& threaded)
{
	auto newAsset = std::dynamic_pointer_cast<Texture>(engine.getManager_Assets().shareAsset(
			typeid(Texture).name(),
			filename,
			[&engine, filename, type, mipmap, anisotropy]() { return std::make_shared<Texture>(engine, filename, type, mipmap, anisotropy); },
			threaded
		));
	swap(newAsset);
}

Texture::~Texture() noexcept
{
	if (ready()) 
		glDeleteTextures(1, &m_glTexID);	
}

Texture::Texture(Engine& engine, const std::string& filename) : Asset(engine, filename) {}

Texture::Texture(Engine& engine, const std::string& filename, const GLuint& type, const bool& mipmap, const bool& anisotropy) :
	Asset(engine, filename),
	m_type(type),
	m_mipmap(mipmap),
	m_anis(anisotropy)
{
}

void Texture::initialize()
{
	// Forward asset creation
	m_image = Shared_Image(m_engine, DIRECTORY_TEXTURE + getFileName(), {}, false);

	// Create Texture
	glCreateTextures(m_type, 1, &m_glTexID);

	// Load Texture
	switch (m_type) {
	case GL_TEXTURE_1D: {
		glTextureStorage1D(m_glTexID, 1, GL_RGBA16F, m_image->m_size.x);
		glTextureSubImage1D(m_glTexID, 0, 0, m_image->m_size.x, GL_RGBA, GL_UNSIGNED_BYTE, &m_image->m_pixelData[0]);
		glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		break;
	}
	case GL_TEXTURE_2D: {
		glTextureStorage2D(m_glTexID, 1, GL_RGBA16F, m_image->m_size.x, m_image->m_size.y);
		glTextureSubImage2D(m_glTexID, 0, 0, 0, m_image->m_size.x, m_image->m_size.y, GL_RGBA, GL_UNSIGNED_BYTE, &m_image->m_pixelData[0]);
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
		glTextureStorage3D(m_glTexID, 1, GL_RGBA16F, m_image->m_size.x, m_image->m_size.y, 0);
		glTextureSubImage3D(m_glTexID, 0, 0, 0, 0, m_image->m_size.x, m_image->m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, &m_image->m_pixelData[0]);
		glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateTextureMipmap(m_glTexID);
		break;
	}
	};

	// Finalize
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	Asset::finalize();
}

void Texture::bind(const unsigned int& texture_unit) noexcept
{
	glBindTextureUnit(texture_unit, m_glTexID);
}