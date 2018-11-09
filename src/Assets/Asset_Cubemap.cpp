#include "Assets\Asset_Cubemap.h"
#include "Engine.h"


constexpr char* DIRECTORY_CUBEMAP = "\\Textures\\Cubemaps\\";

Asset_Cubemap::~Asset_Cubemap()
{
	if (existsYet()) {
		glDeleteBuffers(6, m_pboIDs);
		glDeleteTextures(1, &m_glTexID);
	}
}

Asset_Cubemap::Asset_Cubemap(const std::string & filename) : Asset(filename) {}

Shared_Asset_Cubemap Asset_Cubemap::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	return engine->getAssetManager().createAsset<Asset_Cubemap>(
		filename,
		DIRECTORY_CUBEMAP,
		"",
		&initialize,
		engine,
		threaded
	);
}

void Asset_Cubemap::initialize(Engine * engine, const std::string & relativePath)
{
	static const std::string side_suffixes[6] = { "right", "left", "bottom", "top", "front", "back" };
	static const std::string extensions[3] = { ".png", ".jpg", ".tga" };
	glm::ivec2 size(0);
	for (int side = 0; side < 6; ++side) {
		std::string specific_side_directory = "";
		for (int x = 0; x < 3; ++x) {
			specific_side_directory = relativePath + side_suffixes[side] + extensions[x];
			if (Engine::File_Exists(specific_side_directory))
				break;
			specific_side_directory = relativePath + "\\" + side_suffixes[side] + extensions[x];
			if (Engine::File_Exists(specific_side_directory))
				break;
		}

		// Forward image creation
		// Enforce same size for all images, use the size of the first found image
		m_images[side] = Asset_Image::Create(
			engine, specific_side_directory, 
			(size == glm::ivec2(0)) ? std::optional<glm::ivec2>() : size, 
			false
		);
		size = m_images[side]->m_size;
	}	

	// Create the final texture	
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_glTexID);
	glCreateBuffers(6, m_pboIDs);

	// Load the final texture
	glTextureStorage2D(m_glTexID, 1, GL_RGBA16F, m_images[0]->m_size.x, m_images[0]->m_size.x);
	for (int x = 0; x < 6; ++x) {
		glNamedBufferStorage(m_pboIDs[x], m_images[x]->m_size.x * m_images[x]->m_size.x * 4, m_images[x]->m_pixelData, 0);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIDs[x]);
		glTextureSubImage3D(m_glTexID, 0, 0, 0, x, m_images[x]->m_size.x, m_images[x]->m_size.x, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_glTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glTexID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	if (!glIsTexture(m_glTexID))
		engine->getMessageManager().error("Asset_Texture \"" + m_filename + "\" failed to initialize.");

	// Finalize
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	Asset::finalize(engine);
}

void Asset_Cubemap::bind(const unsigned int & texture_unit)
{
	glBindTextureUnit(texture_unit, m_glTexID);
}
