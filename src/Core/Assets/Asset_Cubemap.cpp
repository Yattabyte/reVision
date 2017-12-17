#include "Assets\Asset_Cubemap.h"
#include "Systems\Message_Manager.h"
#include "FreeImage.h"

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
	size = vec2(0);
	filename = "";
	finalized = false;
}

Asset_Cubemap::Asset_Cubemap(const std::string & f) : Asset_Cubemap()
{
	filename = f;
}

int Asset_Cubemap::GetAssetType()
{
	return ASSET_TYPE;
}

void Asset_Cubemap::Bind(const GLuint & texture_unit)
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	glActiveTexture(texture_unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, gl_tex_ID);
}

// Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
// Will generate a default one itself if the default doesn't exist.
Shared_Asset_Cubemap fetchDefaultAsset()
{
	shared_lock<shared_mutex> guard(Asset_Managera::GetMutex_Assets());
	map<int, Shared_Asset> &fallback_assets = Asset_Managera::GetFallbackAssets_Map();
	fallback_assets.insert(pair<int, Shared_Asset>(Asset_Cubemap::GetAssetType(), Shared_Asset()));
	auto &default_asset = fallback_assets[Asset_Cubemap::GetAssetType()];
	if (default_asset.get() == nullptr) { // Check if we already created the default asset
		default_asset = shared_ptr<Asset_Cubemap>(new Asset_Cubemap());
		Shared_Asset_Cubemap cast_asset = dynamic_pointer_cast<Asset_Cubemap>(default_asset);
		cast_asset->filename = "defaultCubemap";
		string fulldirectory = ABS_DIRECTORY_CUBEMAP("defaultCubemap");
		Cubemap_WorkOrder work_order(cast_asset, fulldirectory);
		if (FileReader::FileExistsOnDisk(fulldirectory)) { // Check if we have a default one on disk to load
			work_order.Initialize_Order();
			work_order.Finalize_Order();
			if (cast_asset->ExistsYet()) // did we successfully load the default asset from disk?
				return cast_asset;
		}
		// We didn't load a default asset from disk
		cast_asset->pixel_data[0] = new GLubyte[4];
		cast_asset->pixel_data[1] = new GLubyte[4];
		cast_asset->pixel_data[2] = new GLubyte[4];
		cast_asset->pixel_data[3] = new GLubyte[4];
		cast_asset->pixel_data[4] = new GLubyte[4];
		cast_asset->pixel_data[5] = new GLubyte[4];
		for (int x = 0; x < 6; ++x)
			for (int y = 0; y < 4; ++y)
			cast_asset->pixel_data[x][y] = GLubyte(255);
		work_order.Finalize_Order();
		return cast_asset;
	}
	return dynamic_pointer_cast<Asset_Cubemap>(default_asset);
}

namespace Asset_Manager {
	void load_asset(Shared_Asset_Cubemap &user, const string &filename, const bool &threaded)
	{
		// Check if a copy already finalized
		shared_mutex &mutex_IO_assets = Asset_Managera::GetMutex_Assets();
		auto &assets_cubemaps = (Asset_Managera::GetAssets_List(Asset_Cubemap::GetAssetType()));
		{
			shared_lock<shared_mutex> guard(mutex_IO_assets);
			for each (auto &asset in assets_cubemaps) {
				shared_lock<shared_mutex> asset_guard(asset->m_mutex);
				const Shared_Asset_Cubemap derived_asset = dynamic_pointer_cast<Asset_Cubemap>(asset);
				if (derived_asset) {
					if (derived_asset->filename == filename) {
						asset_guard.unlock();
						asset_guard.release();
						user = derived_asset;
						if (!threaded)
							user->Finalize();
						return;
					}
				}
			}
		}

		// Attempt to create the asset
		const string &fulldirectory = DIRECTORY_CUBEMAP + filename;
		if (!FileReader::FileExistsOnDisk(fulldirectory)) {
			MSG::Error(FILE_MISSING, fulldirectory);
			user = fetchDefaultAsset();
			return;
		}

		{
			unique_lock<shared_mutex> guard(mutex_IO_assets);
			user = Shared_Asset_Cubemap(new Asset_Cubemap(filename));
			assets_cubemaps.push_back(user);
		}

		if (threaded)
			Asset_Managera::AddWorkOrder(new Cubemap_WorkOrder(user, fulldirectory));
		else {
			Cubemap_WorkOrder work_order(user, fulldirectory);
			work_order.Initialize_Order();
			work_order.Finalize_Order();
		}
	}
}

void Cubemap_WorkOrder::Initialize_Order()
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

		if (format == -1) {
			MSG::Error(FILE_MISSING, m_filename);
			m_asset = fetchDefaultAsset();
			return;
		}

		if (format == FIF_UNKNOWN) {
			format = FreeImage_GetFIFFromFilename(file);
			if (!FreeImage_FIFSupportsReading(format)) { // Attempt to resolve texture file format
				MSG::Error(FILE_CORRUPT, m_filename, "Using default texture.");
				m_asset = fetchDefaultAsset();
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

void Cubemap_WorkOrder::Finalize_Order()
{
	shared_lock<shared_mutex> read_guard(m_asset->m_mutex);
	if (!m_asset->ExistsYet()) {
		read_guard.unlock();
		read_guard.release();
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
		m_asset->Finalize();
	}
}
