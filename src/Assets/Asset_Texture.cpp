#include "Assets\Asset_Texture.h"
#include "Engine.h"


constexpr char* DIRECTORY_TEXTURE = "\\Textures\\";

Asset_Texture::~Asset_Texture()
{
	if (existsYet()) {
		glDeleteBuffers(1, &m_pboID);
		glDeleteTextures(1, &m_glTexID);
		glMakeTextureHandleNonResidentARB(m_glTexHandle);
	}
}

Asset_Texture::Asset_Texture(const std::string & filename) : Asset(filename) {}

Asset_Texture::Asset_Texture(const std::string & filename, const GLuint & t, const bool & m, const bool & a) : Asset_Texture(filename)
{
	m_type = t;
	m_mipmap = m;
	m_anis = a;
}

Shared_Asset_Texture Asset_Texture::Create(Engine * engine, const std::string & filename, const GLuint & type, const bool & mipmap, const bool & anis, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Texture>(filename, threaded);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Texture>(filename, type, mipmap, anis);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string &relativePath = DIRECTORY_TEXTURE + filename;
		const std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, relativePath);
		const std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);
		
		// Submit the work order
		assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);
	}
	return userAsset;
}

void Asset_Texture::initialize(Engine * engine, const std::string & relativePath)
{
	// Forward asset creation
	m_image = Asset_Image::Create(engine, relativePath, false);
}

void Asset_Texture::finalize(Engine * engine)
{		
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
			glTextureParameteri(m_glTexID, GL_GENERATE_MIPMAP, GL_TRUE);
			glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateTextureMipmap(m_glTexID);
			break;
		}
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	m_glTexHandle = glGetTextureHandleARB(m_glTexID);
	
	// Finalize
	Asset::finalize(engine);	
}

void Asset_Texture::bind(const unsigned int & texture_unit)
{
	glBindTextureUnit(texture_unit, m_glTexID);
}