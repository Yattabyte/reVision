#include "Assets\Asset_Cubemap.h"
#include "Managers\Message_Manager.h"
#include "FreeImage.h"

/* -----ASSET TYPE----- */
#define ASSET_TYPE 2


Asset_Cubemap::~Asset_Cubemap()
{
	if (existsYet())
		glDeleteTextures(1, &gl_tex_ID);
	if (m_fence != nullptr)
		glDeleteSync(m_fence);
}

Asset_Cubemap::Asset_Cubemap(const std::string & filename) : Asset(filename)
{
	gl_tex_ID = 0;
	size = vec2(0);
	m_fence = nullptr;
}

int Asset_Cubemap::Get_Asset_Type()
{
	return ASSET_TYPE;
}

void Asset_Cubemap::bind(const GLuint & texture_unit)
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	glActiveTexture(texture_unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, gl_tex_ID);
}

bool Asset_Cubemap::existsYet()
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

/** Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
 * @brief Uses hard-coded values
 * @param	asset	a shared pointer to fill with the default asset */
void fetch_default_asset(Shared_Asset_Cubemap & asset)
{	
	// Check if a copy already exists
	if (Asset_Manager::Query_Existing_Asset<Asset_Cubemap>(asset, "defaultCubemap"))
		return;

	// Create hard-coded alternative
	Asset_Manager::Create_New_Asset<Asset_Cubemap>(asset, "defaultCubemap");
	asset->pixel_data[0] = new GLubyte[4]{ GLubyte(255), GLubyte(0), GLubyte(0), GLubyte(255) };
	asset->pixel_data[1] = new GLubyte[4]{ GLubyte(0), GLubyte(255), GLubyte(0), GLubyte(255) };
	asset->pixel_data[2] = new GLubyte[4]{ GLubyte(0), GLubyte(0), GLubyte(255), GLubyte(255) };
	asset->pixel_data[3] = new GLubyte[4]{ GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(255) };
	asset->pixel_data[4] = new GLubyte[4]{ GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(255) };
	asset->pixel_data[5] = new GLubyte[4]{ GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255) };
	asset->size = vec2(1);
	Asset_Manager::Add_Work_Order(new Cubemap_WorkOrder(asset, ""), true);
}

namespace Asset_Loader {
	void load_asset(Shared_Asset_Cubemap & user, const string & filename, const bool & threaded)
	{
		// Check if a copy already exists
		if (Asset_Manager::Query_Existing_Asset<Asset_Cubemap>(user, filename))
			return;

		// Check if the file/directory exists on disk
		const string &fullDirectory = DIRECTORY_CUBEMAP + filename;
		if (!File_Reader::FileExistsOnDisk(fullDirectory)) {
			MSG::Error(FILE_MISSING, fullDirectory);
			fetch_default_asset(user);
			return;
		}

		// Create the asset
		Asset_Manager::Submit_New_Asset<Asset_Cubemap, Cubemap_WorkOrder>(user, threaded, fullDirectory, filename);
	}
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
			MSG::Error(FILE_MISSING, m_filename);
			fetch_default_asset(m_asset);
			return;
		}

		if (format == FIF_UNKNOWN) {
			format = FreeImage_GetFIFFromFilename(file);
			if (!FreeImage_FIFSupportsReading(format)) { // Attempt to resolve texture file format
				MSG::Error(FILE_CORRUPT, m_filename, "Using default texture.");
				 fetch_default_asset(m_asset);
				return;
			}
		}

		FIBITMAP* bitmap, *bitmap32;
		FIMULTIBITMAP* mbitmap;

		//Load
		if (format == FIF_GIF) {
			mbitmap = FreeImage_OpenMultiBitmap(FIF_GIF, file, false, true, false, GIF_PLAYBACK);
			MSG::Statement("GIF loading unsupported, using first frame...");
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
		m_asset->size = size;
		GLubyte* textureData = new GLubyte[4 * (int)size.x*(int)size.y];
		char* pixels = (char*)FreeImage_GetBits(bitmap32);

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
		m_asset->pixel_data[sides] = textureData;
	}	
}

void Cubemap_WorkOrder::finalizeOrder()
{
	if (!m_asset->existsYet()) {
		unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
		auto &gl_tex_ID = m_asset->gl_tex_ID;
		auto &size = m_asset->size;
		auto *pixel_data = m_asset->pixel_data;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		float anisotropy = 0.0f, maxAnisotropy = 0.0f;
		anisotropy = 16.0f;//CFG::getPreference(CFG_ENUM::C_TEXTURE_ANISOTROPY);
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
		anisotropy = max(0.0f, min(anisotropy, maxAnisotropy));

		// Create the source texture
		glGenTextures(1, &gl_tex_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, gl_tex_ID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
		for (int x = 0; x < 6; ++x)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + x, 0, GL_RGBA, size.x, size.x, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data[x]);

		/*// Create the final texture
		glGenTextures(1, &gl_tex_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, gl_tex_ID);
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
		glDeleteTextures(1, &sourceTexture);*/

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		m_asset->m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glFlush();
		write_guard.unlock();
		write_guard.release();
		m_asset->finalize();
	}
}
