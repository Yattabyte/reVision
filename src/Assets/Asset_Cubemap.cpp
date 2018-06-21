#include "Assets\Asset_Cubemap.h"
#include "Managers\Message_Manager.h"
#include "FreeImage.h"


/** Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
 * @brief Uses hard-coded values
 * @param	asset	a shared pointer to fill with the default asset */
void fetch_default_asset(Shared_Asset_Cubemap & userAsset)
{
	// Check if a copy already exists
	if (Asset_Manager::Query_Existing_Asset<Asset_Cubemap>(userAsset, "defaultCubemap"))
		return;

	// Create hard-coded alternative
	Asset_Manager::Create_New_Asset<Asset_Cubemap>(userAsset, "defaultCubemap");
	userAsset->m_pixelData[0] = new GLubyte[4]{ GLubyte(255), GLubyte(0), GLubyte(0), GLubyte(255) };
	userAsset->m_pixelData[1] = new GLubyte[4]{ GLubyte(0), GLubyte(255), GLubyte(0), GLubyte(255) };
	userAsset->m_pixelData[2] = new GLubyte[4]{ GLubyte(0), GLubyte(0), GLubyte(255), GLubyte(255) };
	userAsset->m_pixelData[3] = new GLubyte[4]{ GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(255) };
	userAsset->m_pixelData[4] = new GLubyte[4]{ GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(255) };
	userAsset->m_pixelData[5] = new GLubyte[4]{ GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255) };
	userAsset->m_size = vec2(1);
	Asset_Manager::Add_Work_Order(new Cubemap_WorkOrder(userAsset, ""), true);
}

Asset_Cubemap::~Asset_Cubemap()
{
	if (existsYet())
		glDeleteTextures(1, &m_glTexID);
}

Asset_Cubemap::Asset_Cubemap(const std::string & filename) : Asset(filename)
{
	m_glTexID = 0;
	m_size = vec2(0);
}

void Asset_Cubemap::Create(Shared_Asset_Cubemap & userAsset, const string & filename, const bool & threaded)
{
	// Check if a copy already exists
	if (Asset_Manager::Query_Existing_Asset<Asset_Cubemap>(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const string &fullDirectory = DIRECTORY_CUBEMAP + filename;
	if (!File_Reader::FileExistsOnDisk(fullDirectory)) {
		MSG_Manager::Error(MSG_Manager::FILE_MISSING, fullDirectory);
		fetch_default_asset(userAsset);
		return;
	}

	// Create the asset
	Asset_Manager::Submit_New_Asset<Asset_Cubemap, Cubemap_WorkOrder>(userAsset, threaded, fullDirectory, filename);
}

void Asset_Cubemap::bind(const unsigned int & texture_unit)
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	glBindTextureUnit(texture_unit, m_glTexID);
}

void Cubemap_WorkOrder::initializeOrder()
{
	static const std::string side_suffixes[6] = { "right", "left", "bottom", "top", "front", "back" };
	static const std::string extensions[3] = { ".png", ".jpg", ".tga" };
	for (int sides = 0; sides < 6; ++sides) {
		std::string specific_side_directory;
		const char * file;
		FREE_IMAGE_FORMAT format;

		for (int x = 0; x < 3; ++x) {
			specific_side_directory = m_filename + side_suffixes[sides] + extensions[x];
			file = specific_side_directory.c_str();
			format = FreeImage_GetFileType(file, 0);
			if (format != -1)
				break;
		}

		for (int x = 0; x < 3; ++x) {
			specific_side_directory = m_filename + "\\" + side_suffixes[sides]+ extensions[x];
			file = specific_side_directory.c_str();
			format = FreeImage_GetFileType(file, 0);
			if (format != -1)
				break;
		}

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
		GLubyte* textureData = new GLubyte[4 * (int)size.x*(int)size.y];
		char* pixels = (char *)FreeImage_GetBits(bitmap32);

		for (int j = 0, total = (int)size.x*(int)size.y; j < total; j++) {
			const GLubyte
				&blue = pixels[j * 4 + 0],
				&green = pixels[j * 4 + 1],
				&red = pixels[j * 4 + 2],
				&alpha = pixels[j * 4 + 3];

			textureData[j * 4 + 0] = red;
			textureData[j * 4 + 1] = green;
			textureData[j * 4 + 2] = blue;
			textureData[j * 4 + 3] = alpha;
		}

		//Unload
		FreeImage_Unload(bitmap32);
		if (bitsPerPixel != 32)
			FreeImage_Unload(bitmap);
		m_asset->m_pixelData[sides] = textureData;
	}	
}

void Cubemap_WorkOrder::finalizeOrder()
{
	if (!m_asset->existsYet()) {
		unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
		auto &gl_tex_ID = m_asset->m_glTexID;
		auto &size = m_asset->m_size;
		auto *pixel_data = m_asset->m_pixelData;
		
		// Create the final texture
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &gl_tex_ID);
		glTextureStorage2D(gl_tex_ID, 1, GL_RGBA16F, size.x, size.x);
		for (int x = 0; x < 6; ++x)
			glTextureSubImage3D(gl_tex_ID, 0, 0, 0, x, size.x, size.x, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data[x]);
		glTextureParameteri(gl_tex_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(gl_tex_ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(gl_tex_ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(gl_tex_ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(gl_tex_ID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		write_guard.unlock();
		write_guard.release();
		m_asset->finalize();
	}	
}
