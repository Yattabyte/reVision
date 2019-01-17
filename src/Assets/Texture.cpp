#include "Assets/Texture.h"
#include "Engine.h"


constexpr char* DIRECTORY_TEXTURE = "\\Textures\\";

Shared_Texture::Shared_Texture(Engine * engine, const std::string & filename, const GLuint & type, const bool & mipmap, const bool & anis, const bool & threaded)
	: std::shared_ptr<Texture>(std::dynamic_pointer_cast<Texture>(engine->getManager_Assets().shareAsset(typeid(Texture).name(), filename)))	
{
	// Find out if the asset needs to be created
	if (!get()) {
		// Create new asset on shared_ptr portion of this class 
		(*(std::shared_ptr<Texture>*)(this)) = std::make_shared<Texture>(filename, type, mipmap, anis);
		// Submit data to asset manager
		engine->getManager_Assets().submitNewAsset(typeid(Texture).name(), (*(std::shared_ptr<Asset>*)(this)), std::move(std::bind(&Texture::initialize, get(), engine, (DIRECTORY_TEXTURE + filename))), threaded);
	}
	// Check if we need to wait for initialization
	else
		if (!threaded)
			// Stay here until asset finalizes
			while (!get()->existsYet())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));	
}

Texture::~Texture()
{
	if (existsYet()) {
		glDeleteBuffers(1, &m_pboID);
		glDeleteTextures(1, &m_glTexID);
	}
}

Texture::Texture(const std::string & filename) : Asset(filename) {}

Texture::Texture(const std::string & filename, const GLuint & t, const bool & m, const bool & a) : Texture(filename)
{
	m_type = t;
	m_mipmap = m;
	m_anis = a;
}

void Texture::initialize(Engine * engine, const std::string & relativePath)
{
	// Forward asset creation
	m_image = Shared_Image(engine, relativePath, {}, false);

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
	Asset::finalize(engine);
}

void Texture::bind(const unsigned int & texture_unit)
{
	glBindTextureUnit(texture_unit, m_glTexID);
}