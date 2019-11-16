#include "Assets/Cubemap.h"
#include "Engine.h"


constexpr const char* DIRECTORY_CUBEMAP = R"(\Textures\Cubemaps\)";
constexpr const auto CUBEMAP_SIDE_COUNT = 6;

Shared_Cubemap::Shared_Cubemap(Engine* engine, const std::string& filename, const bool& threaded) noexcept
{
	(*(std::shared_ptr<Cubemap>*)(this)) = std::dynamic_pointer_cast<Cubemap>(
		engine->getManager_Assets().shareAsset(
			typeid(Cubemap).name(),
			filename,
			[engine, filename]() { return std::make_shared<Cubemap>(engine, filename); },
			threaded
		));
}

Cubemap::~Cubemap() noexcept
{
	if (existsYet()) {
		glDeleteBuffers(CUBEMAP_SIDE_COUNT, m_pboIDs);
		glDeleteTextures(1, &m_glTexID);
	}
}

Cubemap::Cubemap(Engine* engine, const std::string& filename) noexcept : Asset(engine, filename) {}

void Cubemap::initialize() noexcept
{
	static const std::string side_suffixes[CUBEMAP_SIDE_COUNT] = { "right", "left", "bottom", "top", "front", "back" };
	static const std::string extensions[3] = { ".png", ".jpg", ".tga" };
	glm::ivec2 size(0);
	for (int side = 0; side < CUBEMAP_SIDE_COUNT; ++side) {
		std::string specific_side_directory;
		for (const auto& extension : extensions) {
			specific_side_directory = DIRECTORY_CUBEMAP + getFileName() + side_suffixes[side] + extension;
			if (Engine::File_Exists(specific_side_directory))
				break;
			specific_side_directory = DIRECTORY_CUBEMAP + getFileName() + "\\" + side_suffixes[side] + extension;
			if (Engine::File_Exists(specific_side_directory))
				break;
		}

		// Forward image creation
		// Enforce same size for all images, use the size of the first found image
		m_images[side] = Shared_Image(
			m_engine, specific_side_directory,
			(size == glm::ivec2(0)) ? std::optional<glm::ivec2>() : size,
			false
		);
		size = m_images[side]->m_size;
	}

	// Create the final texture
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_glTexID);
	glCreateBuffers(CUBEMAP_SIDE_COUNT, m_pboIDs);

	// Load the final texture
	glTextureStorage2D(m_glTexID, 1, GL_RGBA16F, m_images[0]->m_size.x, m_images[0]->m_size.x);
	for (int x = 0; x < CUBEMAP_SIDE_COUNT; ++x) {
		glNamedBufferStorage(m_pboIDs[x], GLsizeiptr(m_images[x]->m_size.x) * GLsizeiptr(m_images[x]->m_size.x) * 4LL, m_images[x]->m_pixelData, 0);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIDs[x]);
		glTextureSubImage3D(m_glTexID, 0, 0, 0, x, m_images[x]->m_size.x, m_images[x]->m_size.x, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void*)nullptr);
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glTextureParameteri(m_glTexID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_glTexID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_glTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glTexID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	if (!glIsTexture(m_glTexID))
		m_engine->getManager_Messages().error("Texture \"" + m_filename + "\" failed to initialize.");

	// Finalize
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	Asset::finalize();
}

void Cubemap::bind(const unsigned int& texture_unit) noexcept
{
	glBindTextureUnit(texture_unit, m_glTexID);
}