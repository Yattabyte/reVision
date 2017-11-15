#include "Assets\Asset_Cubemap.h"
#include "Systems\Config_Manager.h"

/* -----ASSET TYPE----- */
#define ASSET_TYPE 2

Asset_Cubemap::~Asset_Cubemap()
{
	if (finalized)
		glDeleteTextures(1, &gl_tex_ID);
}

Asset_Cubemap::Asset_Cubemap()
{
	gl_tex_ID = 0;
	type = 0;
	size = vec2(0);
	filename = "";
	finalized = false;
}

Asset_Cubemap::Asset_Cubemap(const std::string & f, const GLuint & t) : Asset_Cubemap()
{
	filename = f;
	type = t;
}

int Asset_Cubemap::GetAssetType()
{
	return ASSET_TYPE;
}

void Asset_Cubemap::Finalize()
{
	/*shared_lock<shared_mutex> read_guard(m_mutex);
	if (!finalized) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_mutex);
		GLuint sourceTexture = 0;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		float anisotropy = 0.0f, maxAnisotropy = 0.0f;
		anisotropy = CFG::getPreference(CFG_ENUM::C_TEXTURE_ANISOTROPY);
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
		anisotropy = max(0.0f, min(anisotropy, maxAnisotropy));

		// Create the source texture
		glGenTextures(1, &sourceTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, sourceTexture);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
		for (int x = 0; x < 6; ++x)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + x, 0, GL_RGBA, size.x, size.x, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data[x]);

		// Create the final texture
		glGenTextures(1, &gl_tex_ID);
		glBindTexture(type, gl_tex_ID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_LOD, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LOD, CF_MIP_LODS - 1);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, CF_MIP_LODS - 1);
		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
		for (int x = 0; x < 6; ++x)
			for (int y = 0; y < 6; ++y)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + x, y, GL_RGBA32F, size.x, size.x, 0, GL_RGBA, GL_FLOAT, NULL);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		VisualFX::APPLY_FX_CUBE_FILTER(sourceTexture, gl_tex_ID, size.x);
		glDeleteTextures(1, &sourceTexture);
		finalized = true;
	}*/
}

void Asset_Cubemap::Bind(const GLuint & texture_unit)
{
	glActiveTexture(texture_unit);
	glBindTexture(type, gl_tex_ID);
}