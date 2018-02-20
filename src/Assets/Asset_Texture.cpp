#include "Assets\Asset_Texture.h"
#include "Managers\Message_Manager.h"
#include "FreeImage.h"


Asset_Texture::~Asset_Texture()
{
	if (existsYet())
		glDeleteTextures(1, &m_glTexID);
	if (m_fence != nullptr)
		glDeleteSync(m_fence);
}

Asset_Texture::Asset_Texture(const string & filename) : Asset(filename)
{
	m_glTexID = 0;
	m_type = 0;
	m_size = vec2(0);
	m_pixelData = nullptr;
	m_mipmap = false;
	m_anis = false;
	m_fence = nullptr;
}

Asset_Texture::Asset_Texture(const string & filename, const GLuint & t, const bool & m, const bool & a) : Asset_Texture(filename)
{
	m_type = t;
	m_mipmap = m;
	m_anis = a;
}

bool Asset_Texture::existsYet()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	if (Asset::existsYet() && m_fence != nullptr) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_mutex);
		const auto state = glClientWaitSync(m_fence, 0, 0);
		if (((state == GL_ALREADY_SIGNALED) || (state == GL_CONDITION_SATISFIED))
			&& (state != GL_WAIT_FAILED))
			return true;
	}
	return false;
}

void Asset_Texture::bind(const GLuint & texture_unit)
{
	glActiveTexture(texture_unit);
	glBindTexture(m_type, m_glTexID);
}

/** Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
 * @brief Uses hard-coded values
 * @param	asset	a shared pointer to fill with the default asset */
void fetch_default_asset(Shared_Asset_Texture & asset)
{
	// Check if a copy already exists
	if (Asset_Manager::Query_Existing_Asset<Asset_Texture>(asset, "defaultTexture"))
		return;

	// Create hard-coded alternative
	Asset_Manager::Create_New_Asset<Asset_Texture>(asset, "defaultTexture");
	asset->m_pixelData = new GLubyte[4];
	for (int x = 0; x < 4; ++x)
		asset->m_pixelData[x] = GLubyte(255);
	asset->m_size = vec2(1);
	Asset_Manager::Add_Work_Order(new Texture_WorkOrder(asset, ""), true);
}

namespace Asset_Loader {
	void load_asset(Shared_Asset_Texture & user, const string & filename, const bool & mipmap, const bool & anis, const bool & threaded)
	{
		// Check if a copy already exists
		if (Asset_Manager::Query_Existing_Asset<Asset_Texture>(user, filename))
			return;
		
		// Check if the file/directory exists on disk
		const string &fullDirectory = DIRECTORY_TEXTURE + filename;
		if (!File_Reader::FileExistsOnDisk(fullDirectory)) {
			MSG_Manager::Error(MSG_Manager::FILE_MISSING, fullDirectory);
			fetch_default_asset(user);
			return;
		}

		// Create the asset
		Asset_Manager::Submit_New_Asset<Asset_Texture, Texture_WorkOrder>(user, threaded, fullDirectory, filename, GL_TEXTURE_2D, mipmap, anis);
	}
}

void Texture_WorkOrder::initializeOrder()
{
	const char * file = m_filename.c_str();
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(file, 0);

	if (format == -1) {
		MSG_Manager::Error(MSG_Manager::FILE_MISSING, m_filename);
		fetch_default_asset(m_asset);
		return;
	}

	if (format == FIF_UNKNOWN) {
		format = FreeImage_GetFIFFromFilename(file);
		if (!FreeImage_FIFSupportsReading(format)) { // Attempt to resolve texture file format
			MSG_Manager::Error(MSG_Manager::FILE_CORRUPT, m_filename, "Using default texture.");
			fetch_default_asset(m_asset);
			return;
		}
	}

	FIBITMAP* bitmap, *bitmap32;
	FIMULTIBITMAP* mbitmap;

	//Load
	if (format == FIF_GIF) {
		mbitmap = FreeImage_OpenMultiBitmap(FIF_GIF, file, false, true, false, GIF_PLAYBACK);
		MSG_Manager::Statement("GIF loading unsupported, using first frame...");
		bitmap = FreeImage_LockPage(mbitmap, 0);
	}
	else
		bitmap = FreeImage_Load(format, file);

	int bitsPerPixel = FreeImage_GetBPP(bitmap);
	if (bitsPerPixel == 32)
		bitmap32 = bitmap;
	else
		bitmap32 = FreeImage_ConvertTo32Bits(bitmap);

	vec2 size = vec2(FreeImage_GetWidth(bitmap32), FreeImage_GetHeight(bitmap32));
	m_asset->m_size = size;
	m_asset->m_pixelData = new GLubyte[4 * (int)size.x*(int)size.y];
	GLubyte *textureData = m_asset->m_pixelData;
	char* pixels = (char *)FreeImage_GetBits(bitmap32);

	for (int j = 0, total = (int)size.x*(int)size.y; j < total; j++) {
		GLubyte blue = pixels[j * 4 + 0],
			green = pixels[j * 4 + 1],
			red = pixels[j * 4 + 2],
			alpha = pixels[j * 4 + 3];
		textureData[j * 4 + 0] = red;
		textureData[j * 4 + 1] = green;
		textureData[j * 4 + 2] = blue;
		textureData[j * 4 + 3] = alpha;
	}

	//Unload
	FreeImage_Unload(bitmap32);
	if (bitsPerPixel != 32)
		FreeImage_Unload(bitmap);

	if (size.x < 1.5f || size.y < 1.5f)
		m_asset->m_type = GL_TEXTURE_1D;
}

void Texture_WorkOrder::finalizeOrder()
{
	if (!m_asset->existsYet()) {
		unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
		auto &gl_tex_ID = m_asset->m_glTexID;
		auto &type = m_asset->m_type;
		auto &size = m_asset->m_size;
		auto &pixel_data = m_asset->m_pixelData;
		auto &anis = m_asset->m_anis;
		auto &mipmap = m_asset->m_mipmap;
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

		m_asset->m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glFlush();

		write_guard.unlock();
		write_guard.release();
		m_asset->finalize();
	}
}