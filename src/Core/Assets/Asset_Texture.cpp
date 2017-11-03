#include "Assets\Asset_Texture.h"
#include "Managers\Message_Manager.h"
#include "FreeImage.h"

/* -----ASSET TYPE----- */
#define ASSET_TYPE 7

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

// Forward declaration
Shared_Asset_Texture fetchDefaultTexture();
// Forward declaration
void initialize_Texture(Shared_Asset_Texture &user, const string & filename, bool *complete);

// Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
// Will generate a default one itself if the default doesn't exist.
Shared_Asset_Texture fetchDefaultTexture()
{
	shared_lock<shared_mutex> guard(getMutexIOAssets());
	map<int, Shared_Asset> &fallback_assets = getFallbackAssets();
	fallback_assets.insert(pair<int, Shared_Asset>(Asset_Texture::GetAssetType(), Shared_Asset()));
	auto &default_asset = fallback_assets[Asset_Texture::GetAssetType()];
	if (default_asset.get() == nullptr) { // Check if we already created the default asset
		default_asset = shared_ptr<Asset_Texture>(new Asset_Texture());
		Shared_Asset_Texture cast_asset = dynamic_pointer_cast<Asset_Texture>(default_asset);
		cast_asset->filename = "defaultTexture";
		if (fileOnDisk(ABS_DIRECTORY_TEXTURE("defaultTexture"))) { // Check if we have a default one on disk to load
			bool complete = false;
			initialize_Texture(cast_asset, ABS_DIRECTORY_TEXTURE("defaultTexture"), &complete);
			cast_asset->Finalize();
			if (complete && cast_asset->ExistsYet()) // did we successfully load the default asset from disk?
				return cast_asset;
		}
		// We didn't load a default asset from disk
		cast_asset->pixel_data = new GLubyte[4];
		for (int x = 0; x < 4; ++x)
			cast_asset->pixel_data[x] = GLubyte(255);
		cast_asset->Finalize();
		return cast_asset;
	}
	return dynamic_pointer_cast<Asset_Texture>(default_asset);
}

// Loads the texture file from disk using dt_FreeImage into memory
void initialize_Texture(Shared_Asset_Texture &user, const string & filename, bool *complete)
{
	const char * file = filename.c_str();
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(file, 0);

	if (format == -1) {
		MSG::Error(FILE_MISSING, filename);
		user = fetchDefaultTexture();
		return;
	}

	if (format == FIF_UNKNOWN) {
		format = FreeImage_GetFIFFromFilename(file);
		if (!FreeImage_FIFSupportsReading(format)) { // Attempt to resolve texture file format
			MSG::Error(FILE_CORRUPT, filename, "Using default texture.");
			user = fetchDefaultTexture();
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
	user->size = size;
	user->pixel_data = new GLubyte[4 * (int)size.x*(int)size.y];
	GLubyte *textureData = user->pixel_data;
	char* pixels = (char*)FreeImage_GetBits(bitmap32);

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
		user->type = GL_TEXTURE_1D;

	submitWorkorder(user);
	*complete = true;
}

namespace Asset_Manager {
	void load_asset(Shared_Asset_Texture &user, const string &filename, const bool &mipmap, const bool &anis, const bool &threaded)
	{
		// Check if a copy already finalized
		shared_mutex &mutex_IO_assets = getMutexIOAssets();
		auto &assets_textures = (fetchAssetList(Asset_Texture::GetAssetType()));
		{
			shared_lock<shared_mutex> guard(mutex_IO_assets);
			for each (auto &asset in assets_textures) {
				shared_lock<shared_mutex> asset_guard(asset->m_mutex);
				const Shared_Asset_Texture derived_asset = dynamic_pointer_cast<Asset_Texture>(asset);
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
		const string &fulldirectory = DIRECTORY_TEXTURE + filename;
		if (!fileOnDisk(fulldirectory)) {
			MSG::Error(FILE_MISSING, fulldirectory);
			user = fetchDefaultTexture();
			return;
		}

		{
			unique_lock<shared_mutex> guard(mutex_IO_assets);
			user = Shared_Asset_Texture(new Asset_Texture(filename, GL_TEXTURE_2D, mipmap, anis));
			assets_textures.push_back(user);
		}

		bool *complete = new bool(false);
		if (threaded) {
			thread *import_thread = new thread(initialize_Texture, user, fulldirectory, complete);
			import_thread->detach();
			submitWorkthread(pair<thread*, bool*>(import_thread, complete));
		}
		else {
			initialize_Texture(user, fulldirectory, complete);
			user->Finalize();
		}
	}
}