#include "Assets/Material.h"
#include "Engine.h"
#include <cmath>
#include <fstream>
#include <sstream>


constexpr const char* MATERIAL_EXTENSION = ".mat";

Shared_Material::Shared_Material(Engine* engine, const std::string& filename, const std::vector<std::string>& textures, const bool& threaded) noexcept
{
	(*(std::shared_ptr<Material>*)(this)) = std::dynamic_pointer_cast<Material>(
		engine->getManager_Assets().shareAsset(
			typeid(Material).name(),
			filename,
			[engine, filename, textures]() { return std::make_shared<Material>(engine, filename, textures); },
			threaded
		));
}

Material::~Material() noexcept
{
	if (m_materialData != nullptr)
		delete m_materialData;
}

Material::Material(Engine* engine, const std::string& filename, const std::vector<std::string>& textures) noexcept
	: Asset(engine, filename), m_textures(textures)
{
	// We need to reserve a region of GPU memory for all the textures
	// So we need to preemptively figure out the maximum number of textures we may need (can't delay until later)
	// Thus, we will process any extra files ahead of time, like ".mat"

	// Check if we're loading extra material data from a .mat file
	const std::string relativePath = filename + MATERIAL_EXTENSION;
	if (Engine::File_Exists(relativePath)) {
		// Fetch a list of textures as defined in the file
		auto tx = Material::Get_Material_Textures(relativePath);
		// Recover the material folder directory from the filename
		const size_t slash1Index = relativePath.find_last_of('/');
		const size_t slash2Index = relativePath.find_last_of('\\');
		const size_t furthestFolderIndex = std::max(slash1Index != std::string::npos ? slash1Index : 0, slash2Index != std::string::npos ? slash2Index : 0);
		const std::string modelDirectory = relativePath.substr(0, furthestFolderIndex + 1);
		// Apply these texture directories to the material whenever not null
		for (size_t x = 0, size = m_textures.size(); x < size; ++x) {
			// In case we made the original texture set larger, copy the original texture naming pattern
			if (m_textures[x].empty())
				m_textures[x] = m_textures[x % MAX_PHYSICAL_IMAGES];
			if (!tx[x].empty())
				m_textures[x] = modelDirectory + tx[x];
		}
	}
}

void Material::initialize() noexcept
{
	// Some definitions for later
	const size_t remainder = m_textures.size() % size_t(6U);
	const size_t textureCount = remainder != 0U
		? m_textures.size() + size_t(6U) - remainder // if remainder != 0, round up to nearest multiple of 6
		: std::max(size_t(6U), m_textures.size()); // else remainder == 0, enforce minimum size of 6
	const size_t materialCount = textureCount / MAX_PHYSICAL_IMAGES;
	m_textures.resize(textureCount);

	// Load all images
	float materialSize = 512.0F;
	m_engine->getPreferenceState().getOrSetValue(PreferenceState::Preference::C_MATERIAL_SIZE, materialSize);
	m_images.resize(textureCount);
	m_size = glm::ivec2((int)materialSize);
	constexpr Fill_Policy fillPolicies[MAX_PHYSICAL_IMAGES] = {
		Fill_Policy::CHECKERED,
		Fill_Policy::SOLID,
		Fill_Policy::SOLID,
		Fill_Policy::SOLID,
		Fill_Policy::SOLID,
		Fill_Policy::SOLID
	};
	for (size_t x = 0; x < textureCount; ++x)
		m_images[x] = Shared_Image(m_engine, m_textures[x], m_size, false, fillPolicies[x]);

	// Merge data into single array
	const size_t pixelsPerImage = size_t(m_size.x) * size_t(m_size.y) * 4ULL;
	m_materialData = new GLubyte[(pixelsPerImage)*MAX_DIGITAL_IMAGES * materialCount]();
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

	// Finalize
	Asset::finalize();
}

/** Attempts to retrieve a std::string between quotation marks "<std::string>"
@return	the std::string between quotation marks */
[[nodiscard]] static std::string get_between_quotes(std::string& s) noexcept
{
	std::string output = s;
	size_t spot1 = s.find_first_of('\"');
	if (spot1 != std::string::npos) {
		output = output.substr(spot1 + 1, output.length() - spot1 - 1);
		size_t spot2 = output.find_first_of('\"');
		if (spot2 != std::string::npos) {
			output = output.substr(0, spot2);

			s = s.substr(spot2 + 2, s.length() - spot2 - 1);
		}
	}
	return output;
}

/** Parse a given line between parentheses and convert it to a string.
@param	in	the string to convert
@return		a string */
[[nodiscard]] static std::string getType_String(std::string& in) noexcept
{
	return get_between_quotes(in);
}

/** Search a given string and return whether or not it contains the desired string.
@param		s1	the string to search within
@param		s2	the target string to find
@return		true if the second string is found in the first, else otherwise. */
[[nodiscard]] static bool find(const std::string& s1, const std::string& s2) noexcept
{
	return (s1.find(s2) != std::string::npos);
}

/** Parse a PBR material document. */
[[nodiscard]] static std::vector<std::string> parse_pbr(std::ifstream& file_stream) noexcept
{
	std::vector<std::string> textures(MAX_PHYSICAL_IMAGES);
	int bracketCount = 0;
	for (std::string line; std::getline(file_stream, line); ) {
		if ((line.length() != 0U) && !line.empty() && line != " ") {
			if (find(line, "{")) {
				bracketCount++;
				continue;
			}
			if (find(line, "}")) {
				bracketCount--;
				if (bracketCount <= 0)
					break;
				continue;
			}
			
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
	return textures;
}

[[nodiscard]] std::vector<std::string> Material::Get_Material_Textures(const std::string& filename) noexcept
{
	std::vector<std::string> textures;
	std::ifstream file_stream(Engine::Get_Current_Dir() + filename);
	int bracketCount = 0;
	for (std::string line; std::getline(file_stream, line); ) {
		if (find(line, "{")) {
			bracketCount++;
			continue;
		}
		if (find(line, "}")) {
			bracketCount--;
			if (bracketCount <= 0)
				break;
			continue;
		}
		if (find(line, "PBR"))
			for each (const auto & texture in parse_pbr(file_stream))
				textures.push_back(texture);
	}
	return textures;
}