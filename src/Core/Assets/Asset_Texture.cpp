#include "Assets\Asset_Texture.h"

/* -----ASSET TYPE----- */
#define ASSET_TYPE 4

using namespace Asset_Manager;

Asset_Texture::~Asset_Texture()
{
	if (finalized)
		glDeleteTextures(1, &gl_tex_ID);
}

Asset_Texture::Asset_Texture()
{
	gl_tex_ID = 0;
	type = 0;
	size = vec2(0);
	filename = "";
	pixel_data = nullptr;
	mipmap = false;
	anis = false;
}

Asset_Texture::Asset_Texture(const string & f, const GLuint & t, const bool & m, const bool & a) : Asset_Texture()
{
	filename = f;
	type = t;
	mipmap = m;
	anis = a;
}

int Asset_Texture::GetAssetType()
{
	return ASSET_TYPE;
}

void Asset_Texture::Finalize()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	if (!finalized) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_mutex);
		glGenTextures(1, &gl_tex_ID);
		glBindTexture(type, gl_tex_ID);
		switch (type) {
			case GL_TEXTURE_1D: {
				glTexStorage1D(type, 1, GL_RGBA, size.x);
				glTexImage1D(type, 0, GL_RGBA, size.x, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);
				glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				break;
			}
			case GL_TEXTURE_2D: {
				glTexStorage2D(type, 1, GL_RGBA, size.x, size.y);
				glTexImage2D(type, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);
				if (anis)
					glTexParameterf(type, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
				if (mipmap) {
					glTexParameteri(type, GL_GENERATE_MIPMAP, GL_TRUE);
					glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glGenerateMipmap(type);
				}
				else {
					glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				}
				break;
			}
			case GL_TEXTURE_2D_ARRAY: {
				glTexStorage3D(type, 1, GL_RGBA, size.x, size.y, 0);
				glTexImage3D(type, 0, GL_RGBA, size.x, size.y, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);
				glTexParameteri(type, GL_GENERATE_MIPMAP, GL_TRUE);
				glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glGenerateMipmap(type);
				break;
			}
		}
		finalized = true;
	}
}

void Asset_Texture::Bind(const GLuint & texture_unit)
{
	glActiveTexture(texture_unit);
	glBindTexture(type, gl_tex_ID);
}