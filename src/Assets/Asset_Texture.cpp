#include "Assets\Asset_Texture.h"
#include "Managers\Message_Manager.h"
#include "FreeImage.h"


/** Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
 * @brief Uses hard-coded values
 * @param	asset	a shared pointer to fill with the default asset */
void fetch_default_asset(Shared_Asset_Texture & userAsset)
{
	// Check if a copy already exists
	if (Asset_Manager::Query_Existing_Asset<Asset_Texture>(userAsset, "defaultTexture"))
		return;

	// Create hard-coded alternative
	Asset_Manager::Create_New_Asset<Asset_Texture>(userAsset, "defaultTexture");
	userAsset->m_pixelData = new GLubyte[4];
	for (int x = 0; x < 4; ++x)
		userAsset->m_pixelData[x] = GLubyte(255);
	userAsset->m_size = vec2(1);
	Asset_Manager::Add_Work_Order(new Texture_WorkOrder(userAsset, ""), true);
}

Asset_Texture::~Asset_Texture()
{
	if (existsYet())
		glDeleteTextures(1, &m_glTexID);
}

Asset_Texture::Asset_Texture(const string & filename) : Asset(filename)
{
	m_glTexID = 0;
	m_type = 0;
	m_size = vec2(0);
	m_pixelData = nullptr;
	m_mipmap = false;
	m_anis = false;
}

Asset_Texture::Asset_Texture(const string & filename, const GLuint & t, const bool & m, const bool & a) : Asset_Texture(filename)
{
	m_type = t;
	m_mipmap = m;
	m_anis = a;
}

void Asset_Texture::Create(Shared_Asset_Texture & userAsset, const string & filename, const bool & mipmap, const bool & anis, const bool & threaded)
{
	// Check if a copy already exists
	if (Asset_Manager::Query_Existing_Asset<Asset_Texture>(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const string &fullDirectory = DIRECTORY_TEXTURE + filename;
	if (!File_Reader::FileExistsOnDisk(fullDirectory)) {
		MSG_Manager::Error(MSG_Manager::FILE_MISSING, fullDirectory);
		fetch_default_asset(userAsset);
		return;
	}

	// Create the asset
	Asset_Manager::Submit_New_Asset<Asset_Texture, Texture_WorkOrder>(userAsset, threaded, fullDirectory, filename, GL_TEXTURE_2D, mipmap, anis);
}

void Asset_Texture::bind(const unsigned int & texture_unit)
{
	glBindTextureUnit(texture_unit, m_glTexID);
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
		const auto &type = m_asset->m_type;
		glCreateTextures(type, 1, &gl_tex_ID);
		write_guard.unlock();
		write_guard.release();
		shared_lock<shared_mutex> read_guard(m_asset->m_mutex);
		const auto &size = m_asset->m_size;
		const auto &pixel_data = m_asset->m_pixelData;
		const auto &anis = m_asset->m_anis;
		const auto &mipmap = m_asset->m_mipmap;
		switch (type) {
			case GL_TEXTURE_1D: {
				glTextureStorage1D(gl_tex_ID, 1, GL_RGBA16F, size.x);
				glTextureSubImage1D(gl_tex_ID, 0, 0, size.x, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);
				glTextureParameteri(gl_tex_ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTextureParameteri(gl_tex_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				break;
			}
			case GL_TEXTURE_2D: {
				glTextureStorage2D(gl_tex_ID, 1, GL_RGBA16F, size.x, size.y);
				glTextureSubImage2D(gl_tex_ID, 0, 0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);
				if (anis)
					glTextureParameterf(gl_tex_ID, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
				if (mipmap) {
					glTextureParameteri(gl_tex_ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTextureParameteri(gl_tex_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glGenerateTextureMipmap(gl_tex_ID);
				}
				else {
					glTextureParameteri(gl_tex_ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTextureParameteri(gl_tex_ID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				}
				break;
			}
			case GL_TEXTURE_2D_ARRAY: {
				glTextureStorage3D(gl_tex_ID, 1, GL_RGBA16F, size.x, size.y, 0);
				glTextureSubImage3D(gl_tex_ID, 0, 0, 0, 0, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);
				glTextureParameteri(gl_tex_ID, GL_GENERATE_MIPMAP, GL_TRUE);
				glTextureParameteri(gl_tex_ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTextureParameteri(gl_tex_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glGenerateTextureMipmap(gl_tex_ID);
				break;
			}
		}
		read_guard.unlock();
		read_guard.release();
		m_asset->finalize();
	}
}