#include "Assets\Asset_Texture.h"
#include "Systems\Message_Manager.h"
#include "FreeImage.h"

/* -----ASSET TYPE----- */
#define ASSET_TYPE 7

using namespace Asset_Loader;

Asset_Texture::~Asset_Texture()
{
	if (ExistsYet())
		glDeleteTextures(1, &gl_tex_ID);
	if (m_fence != nullptr)
		glDeleteSync(m_fence);
}

Asset_Texture::Asset_Texture(const string & filename) : Asset(filename)
{
	gl_tex_ID = 0;
	type = 0;
	size = vec2(0);
	pixel_data = nullptr;
	mipmap = false;
	anis = false;
	m_fence = nullptr;
}

Asset_Texture::Asset_Texture(const string & filename, const GLuint & t, const bool & m, const bool & a) : Asset_Texture(filename)
{
	type = t;
	mipmap = m;
	anis = a;
}

int Asset_Texture::GetAssetType()
{
	return ASSET_TYPE;
}

bool Asset_Texture::ExistsYet()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	if (Asset::ExistsYet() && m_fence != nullptr) {
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

void Asset_Texture::Bind(const GLuint & texture_unit)
{
	glActiveTexture(texture_unit);
	glBindTexture(type, gl_tex_ID);
}

// Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
// Uses hardcoded values
void fetchDefaultAsset(Shared_Asset_Texture & asset)
{
	// Check if a copy already exists
	if (Asset_Manager::QueryExistingAsset<Asset_Texture>(asset, "defaultTexture"))
		return;

	// Create hardcoded alternative
	Asset_Manager::CreateNewAsset<Asset_Texture>(asset, "defaultTexture");
	asset->pixel_data = new GLubyte[4];
	for (int x = 0; x < 4; ++x)
		asset->pixel_data[x] = GLubyte(255);
	asset->size = vec2(1);
	Asset_Manager::AddWorkOrder(new Texture_WorkOrder(asset, ""), true);
}

namespace Asset_Loader {
	void load_asset(Shared_Asset_Texture & user, const string & filename, const bool & mipmap, const bool & anis, const bool & threaded)
	{
		// Check if a copy already exists
		if (Asset_Manager::QueryExistingAsset<Asset_Texture>(user, filename))
			return;
		
		// Check if the file/directory exists on disk
		const string &fullDirectory = DIRECTORY_TEXTURE + filename;
		if (!FileReader::FileExistsOnDisk(fullDirectory)) {
			MSG::Error(FILE_MISSING, fullDirectory);
			fetchDefaultAsset(user);
			return;
		}

		// Create the asset
		Asset_Manager::CreateNewAsset<Asset_Texture, Texture_WorkOrder>(user, threaded, fullDirectory, filename, GL_TEXTURE_2D, mipmap, anis);
	}
}

void Texture_WorkOrder::Initialize_Order()
{
	const char * file = m_filename.c_str();
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(file, 0);

	if (format == -1) {
		MSG::Error(FILE_MISSING, m_filename);
		fetchDefaultAsset(m_asset);
		return;
	}

	if (format == FIF_UNKNOWN) {
		format = FreeImage_GetFIFFromFilename(file);
		if (!FreeImage_FIFSupportsReading(format)) { // Attempt to resolve texture file format
			MSG::Error(FILE_CORRUPT, m_filename, "Using default texture.");
			fetchDefaultAsset(m_asset);
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
	m_asset->pixel_data = new GLubyte[4 * (int)size.x*(int)size.y];
	GLubyte *textureData = m_asset->pixel_data;
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
		m_asset->type = GL_TEXTURE_1D;
}

void Texture_WorkOrder::Finalize_Order()
{
	if (!m_asset->ExistsYet()) {
		unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
		auto &gl_tex_ID = m_asset->gl_tex_ID;
		auto &type = m_asset->type;
		auto &size = m_asset->size;
		auto &pixel_data = m_asset->pixel_data;
		auto &anis = m_asset->anis;
		auto &mipmap = m_asset->mipmap;
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

		m_asset->Finalize();
	}
}