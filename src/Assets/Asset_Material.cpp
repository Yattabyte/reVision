#include "Assets\Asset_Material.h"
#include "Engine.h"
#include <math.h>
#include <fstream>
#include <sstream>

#define EXT_MATERIAL ".mat"
#define ABS_DIRECTORY_MATERIAL(filename) Engine::Get_Current_Dir() + "\\Materials\\" + filename + EXT_MATERIAL
#define ABS_DIRECTORY_MAT_TEX(filename) Engine::Get_Current_Dir() + "\\Textures\\Environment\\" + filename


Asset_Material::~Asset_Material()
{
	if (existsYet())
		glDeleteTextures(1, &m_glArrayID);
	if (m_materialData)
		delete m_materialData;
}

Asset_Material::Asset_Material(const std::string & filename, const std::vector<std::string> &textures) : Asset(filename), m_textures(textures) {}

Shared_Asset_Material Asset_Material::Create(Engine * engine, const std::string & filename, const std::vector<std::string> &textures, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();
	MaterialManager & materialManager = engine->getMaterialManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Material>(filename);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Material>(filename, textures);
		auto & assetRef = *userAsset.get();
		assetRef.m_matSpot = materialManager.generateID();

		std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, filename);
		std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);

		/*
		if (!Engine::File_Exists(filename) || (filename == "") || (filename == " ")) {
			engine->reportError(MessageManager::FILE_MISSING, fullDirectory);
			initFunc = std::bind(&initializeDefault, &assetRef, engine);
		}*/

		// Submit the work order
		assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);
	}
	return userAsset;
}

void Asset_Material::initializeDefault(Engine * engine)
{
	m_materialData = new GLubyte[192]{
		// Albedo with full alpha
		GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255),
		GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255),
		GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255),
		GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255),
		GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255),
		GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255),
		GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255),
		GLubyte(0), GLubyte(0), GLubyte(0), GLubyte(255), GLubyte(255), GLubyte(0), GLubyte(255), GLubyte(255),

		// Straight pointing normal with empty fourth channel
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),
		GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000), GLubyte(128), GLubyte(128), GLubyte(255), GLubyte(000),

		// Quarter metalness (mostly dielectric), half roughness, no height, and full ambience (no occlusion)
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),
		GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255),	GLubyte(063), GLubyte(127), GLubyte(000), GLubyte(255)
	};
	m_size = glm::ivec2(4);
}

void Asset_Material::initialize(Engine * engine, const std::string & fullDirectory)
{
	// Check if we're loading extra material data from a .mat file
	if (Engine::File_Exists(fullDirectory)) {
		// Fetch a list of textures as defined in the file
		auto textures = Asset_Material::Get_Material_Textures(fullDirectory);
		// Recover the material folder directory from the filename
		const size_t slash1Index = fullDirectory.find_last_of('/'), slash2Index = fullDirectory.find_last_of('\\');
		const size_t furthestFolderIndex = std::max(slash1Index != std::string::npos ? slash1Index : 0, slash2Index != std::string::npos ? slash2Index : 0);
		const std::string modelDirectory = fullDirectory.substr(0, furthestFolderIndex + 1);
		// Apply these texture directories to the material whenever not null
		m_textures.resize(textures.size());
		for (size_t x = 0, size = m_textures.size(); x < size; ++x)
			if (textures[x] != "")
				m_textures[x] = modelDirectory + textures[x];
	}

	// Some definitions for later
	const size_t textureCount = m_textures.size();
	const size_t materialCount = textureCount / MAX_PHYSICAL_IMAGES;

	// Load all images	
	m_images.resize(textureCount);
	constexpr GLenum fillPolicies[MAX_PHYSICAL_IMAGES] = {
		Asset_Image::Fill_Policy::Checkered,
		Asset_Image::Fill_Policy::Solid,
		Asset_Image::Fill_Policy::Checkered,
		Asset_Image::Fill_Policy::Checkered,
		Asset_Image::Fill_Policy::Checkered,
		Asset_Image::Fill_Policy::Solid
	};
	for (size_t x = 0; x < textureCount; ++x)
		m_images[x] = Asset_Image::Create(engine, m_textures[x], false, fillPolicies[x]);

	// Find the largest dimensions	
	for each (const auto & image in m_images) {
		if (m_size.x < image->m_size.x)
			m_size.x = image->m_size.x;
		if (m_size.y < image->m_size.y)
			m_size.y = image->m_size.y;
	}

	// Force all images to be the same size	
	for each (auto image in m_images)
		if (image->m_size != m_size)
			image->resize(m_size);
	

	// Merge data into single array
	const size_t pixelsPerImage = m_size.x * m_size.y * 4;
	m_materialData = new GLubyte[(pixelsPerImage) * MAX_DIGITAL_IMAGES * materialCount]();
	size_t arrayIndex = 0;
	for (size_t tx = 0; tx < textureCount; tx += MAX_PHYSICAL_IMAGES) {
		for (size_t x = 0; x < pixelsPerImage; ++x, ++arrayIndex)
			m_materialData[arrayIndex] = m_images[tx + 0]->m_pixelData[x]; // ALBEDO	
		for (size_t x = 0; x < pixelsPerImage; ++x, ++arrayIndex)
			m_materialData[arrayIndex] = m_images[tx + 1]->m_pixelData[x]; // NORMAL
		for (size_t x = 0; x < pixelsPerImage; x += 4, arrayIndex += 4) {
			m_materialData[arrayIndex + 0] = m_images[tx + 2]->m_pixelData[x]; // METALNESS
			m_materialData[arrayIndex + 1] = m_images[tx + 3]->m_pixelData[x]; // ROUGHNESS
			m_materialData[arrayIndex + 2] = m_images[tx + 4]->m_pixelData[x]; // HEIGHT
			m_materialData[arrayIndex + 3] = m_images[tx + 5]->m_pixelData[x]; // AO
		}
	}
}

void Asset_Material::finalize(Engine * engine)
{
	MaterialManager & materialManager = engine->getMaterialManager();

	// Create Material	
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_glArrayID);
	
	// Load material		
	float anisotropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);
	// The equation beneath calculates the nubmer of mip levels needed, to mip down to a size of 1
	// Uses the smallest dimension of the image
	GLsizei m_imageCount = GLsizei((m_textures.size() / MAX_PHYSICAL_IMAGES) * MAX_DIGITAL_IMAGES);
	glTextureStorage3D(m_glArrayID, GLsizei(floor(log2f(float(std::min(m_size.x, m_size.y))) + 1.0f)), GL_RGBA16F, m_size.x, m_size.y, m_imageCount);
	glTextureSubImage3D(m_glArrayID, 0, 0, 0, 0, m_size.x, m_size.y, m_imageCount, GL_RGBA, GL_UNSIGNED_BYTE, m_materialData);
	glTextureParameteri(m_glArrayID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_glArrayID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameterf(m_glArrayID, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
	glGenerateTextureMipmap(m_glArrayID);

	// Synchronize because sometimes driver hasn't completed generating mipmap's before the handle is created 
	// That IS a problem, because once the handle is issued, the texture object CAN NOT and MUST NOT be changed!!!
	GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	auto state = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
	while (state != GL_SIGNALED && state != GL_ALREADY_SIGNALED && state == GL_CONDITION_SATISFIED)
		state = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
	glDeleteSync(fence);
	materialManager.generateHandle(m_matSpot, m_glArrayID);
	if (!glIsTexture(m_glArrayID))
		engine->reportError(MessageManager::MATERIAL_INCOMPLETE, m_filename, m_textures[0] + ", " + m_textures[1] + ", " + m_textures[2] + ", " + m_textures[3] + ", " + m_textures[4] + ", " + m_textures[5]);
	
	// Finalize
	Asset::finalize(engine);
}

/** Fetch the directory of a material texture from its definition file. */
bool getString(std::istringstream & string_stream, std::string & target, std::string & input = std::string(""))
{
	string_stream >> input;
	if (input == "std::string") {
		int size = 0; string_stream >> size;
		target.reserve(size);
		for (int x = 0; x < size; ++x) {
			std::string v;
			string_stream >> v;
			target += v;
		}
	}
	else return false;
	return true;
}
/** Attempts to retrieve a std::string between quotation marks "<std::string>"
@return	the std::string between quotation marks */
std::string const get_between_quotes(std::string & s)
{
	std::string output = s;
	size_t spot1 = s.find_first_of("\"");
	if (spot1 != std::string::npos) {
		output = output.substr(spot1 + 1, output.length() - spot1 - 1);
		size_t spot2 = output.find_first_of("\"");
		if (spot2 != std::string::npos) {
			output = output.substr(0, spot2);

			s = s.substr(spot2 + 2, s.length() - spot2 - 1);
		}
	}
	return output;
}
/** Parse a given line between parantheses and convert it to a string.
@param	in	the string to convert
@return		a string */
std::string const getType_String(std::string & in) {
	return get_between_quotes(in);
}
/** Search a given string and return whether or not it contains the desired string.
@param		s1	the string to search within
@param		s2	the target string to find
@return		true if the second string is found in the first, else otherwise. */
bool const find(const std::string & s1, const std::string & s2) {
	return (s1.find(s2) != std::string::npos);
}
std::vector<std::string> parse_pbr(std::ifstream & file_stream)
{
	std::vector<std::string> textures(MAX_PHYSICAL_IMAGES);
	int bracketCount = 0;
	for (std::string line; std::getline(file_stream, line); ) {
		if (line.length() && line != "" && line != " ") {
			if (find(line, "{")) {
				bracketCount++;
				continue;
			}
			else if (find(line, "}")) {
				bracketCount--;
				if (bracketCount <= 0)
					break;
				continue;
			}
			else {
				if (find(line, "albedo"))
					textures[0] = getType_String(line);
				else if (find(line, "normal"))
					textures[1] = getType_String(line);
				else if (find(line, "metalness"))
					textures[2] = getType_String(line);
				else if (find(line, "roughness"))
					textures[3] = getType_String(line);
				else if (find(line, "height"))
					textures[4] = getType_String(line);
				else if (find(line, "occlusion"))
					textures[5] = getType_String(line);
			}
		}
	}
	return textures;
}
std::vector<std::string> Asset_Material::Get_Material_Textures(const std::string & filename)
{
	std::vector<std::string> textures;
	std::ifstream file_stream(filename);
	int bracketCount = 0;
	for (std::string line; std::getline(file_stream, line); ) {
		if (find(line, "{")) {
			bracketCount++;
			continue;
		}
		else if (find(line, "}")) {
			bracketCount--;
			if (bracketCount <= 0)
				break;
			continue;
		}
		else if (find(line, "PBR"))	
			for each (const auto & texture in parse_pbr(file_stream)) 
				textures.push_back(texture);		
	}
	return textures;
}