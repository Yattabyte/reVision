#include "Assets\Asset_Config.h"
#include "Utilities\IO\Text_IO.h"
#include "Engine.h"
#include <fstream>
#define EXT_CONFIG ".cfg"
#define DIRECTORY_CONFIG Engine::Get_Current_Dir() + "\\Configs\\"
#define ABS_DIRECTORY_CONFIG(filename) DIRECTORY_CONFIG + filename + EXT_CONFIG


/** Attempts to retrieve a std::string between quotation marks "<std::string>"
 * @return	the std::string between quotation marks*/
inline std::string get_between_quotes(std::string & s)
{
	std::string output = s;
	int spot1 = s.find_first_of("\"");
	if (spot1 >= 0) {
		output = output.substr(spot1 + 1, output.length() - spot1 - 1);
		int spot2 = output.find_first_of("\"");
		if (spot2 >= 0) {
			output = output.substr(0, spot2);

			s = s.substr(spot2 + 2, s.length() - spot2 - 1);
		}
	}
	return output;
}

/** Checks if the supplied value is a parameter in the CFG_STRING list.
 * @param	s	the std::string to check for in the list
 * @param	m_strings	the list of strings to check for an occurrence of our value within
 * @return	the index of the value in the list if found, otherwise -1. */
inline int find_CFG_Property(const std::string & s, const std::vector<std::string> & m_strings)
{
	std::string UPPER_STRING;
	for each (const auto &c in s)
		UPPER_STRING += toupper(c);
	bool success = false;
	for (auto value = begin(m_strings); value != end(m_strings); value++)
		if ((*value) == UPPER_STRING)
			return std::distance(m_strings.begin(), value);
	return -1;
}

Asset_Config::~Asset_Config()
{
}

Asset_Config::Asset_Config(const std::string & filename, const std::vector<std::string> & strings) : Asset(filename), m_strings(strings)
{
}

/** Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
* @brief Uses hard-coded values
* @param	asset	a shared pointer to fill with the default asset */
void Asset_Config::CreateDefault(Engine * engine, Shared_Asset_Config & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, "defaultConfig"))
		return;

	// Create hard-coded alternative
	assetManager.createNewAsset(userAsset, "defaultConfig", std::vector<std::string>());

	// Create the asset
	assetManager.submitNewWorkOrder(userAsset, true,
		/* Initialization. */
		[]() {},
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); }
	);
}

void Asset_Config::Create(Engine * engine, Shared_Asset_Config & userAsset, const std::string & filename, const std::vector<std::string>& cfg_strings, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const std::string &fullDirectory = ABS_DIRECTORY_CONFIG(filename);
	if (!Engine::File_Exists(fullDirectory)) {
		engine->reportError(MessageManager::FILE_MISSING, fullDirectory);
		CreateDefault(engine, userAsset);
		return;
	}

	// Create the asset
	assetManager.submitNewAsset(userAsset, threaded,
		/* Initialization. */
		[engine, &userAsset, fullDirectory]() mutable { Initialize(engine, userAsset, fullDirectory); },
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); },
		/* Constructor Arguments. */
		filename, cfg_strings
	);
}

void Asset_Config::Initialize(Engine * engine, Shared_Asset_Config & userAsset, const std::string & fullDirectory)
{
	std::ifstream file_stream(fullDirectory);
	std::unique_lock<std::shared_mutex> write_guard(userAsset->m_mutex);
	for (std::string line; std::getline(file_stream, line); ) {
		if (line.length()) {
			const std::string cfg_property = get_between_quotes(line);
			int spot = find_CFG_Property(cfg_property, userAsset->m_strings);
			if (spot >= 0) {
				std::string cfg_value = get_between_quotes(line);
				userAsset->setValue(spot, atof(cfg_value.c_str()));
			}
		}
	}
}

void Asset_Config::Finalize(Engine * engine, Shared_Asset_Config & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();
	userAsset->finalize();

	// Notify completion
	std::shared_lock<std::shared_mutex> read_guard(userAsset->m_mutex);
	for each (auto qwe in userAsset->m_callbacks)
		assetManager.submitNotifyee(qwe.first, qwe.second);
}

void Asset_Config::setValue(const unsigned int & cfg_key, const float & cfg_value)
{
	// Try inserting the value by key in case the key doesn't exist.
	m_configuration.insert(std::pair<int, float>(cfg_key, cfg_value));
	m_configuration[cfg_key] = cfg_value;
}

float Asset_Config::getValue(const unsigned int & cfg_key)
{
	if (cfg_key >= 0 && m_configuration.find(cfg_key) != m_configuration.end())
		return m_configuration[cfg_key];
	return UNDEFINED_CVAL;
}

void Asset_Config::saveConfig()
{
	std::string output;
	for each (const auto &value in m_configuration) 
		output += "\"" + m_strings[value.first] + "\" \"" + std::to_string(value.second) + "\"\n";
	Text_IO::Export_Text(ABS_DIRECTORY_CONFIG(getFileName()), output);
}