#include "Assets\Asset_Config.h"
#include "Managers\Message_Manager.h"
#include <fstream>

/* -----ASSET TYPE----- */
#define ASSET_TYPE 1


Asset_Config::~Asset_Config()
{
}

Asset_Config::Asset_Config(const string & filename, const vector<string> & strings) : Asset(filename), m_strings(strings)
{
}

int Asset_Config::Get_Asset_Type()
{
	return ASSET_TYPE;
}

void Asset_Config::setValue(const unsigned int & cfg_key, const float & cfg_value)
{
	// Try inserting the value by key in case the key doesn't exist.
	configuration.insert(pair<int, float>(cfg_key, cfg_value));
	configuration[cfg_key] = cfg_value;
}

float Asset_Config::getValue(const unsigned int & cfg_key)
{
	if (cfg_key >= 0 && configuration.find(cfg_key) != configuration.end())
		return configuration[cfg_key];
	return UNDEFINED_CVAL;
}

void Asset_Config::saveConfig()
{
	string output;

	for each (const auto &value in configuration) 
		output += "\"" + m_strings[value.first] + "\" \"" + to_string(value.second) + "\"\n";

	string directory = ABS_DIRECTORY_CONFIG(getFileName());
	ofstream out(directory);
	out << output.c_str();
}

/** Attempts to retrieve a string between quotation marks "<string>" 
 * @return	the string between quotation marks*/
string get_between_quotes(string & s)
{
	string output = s;
	int spot1 = s.find_first_of("\"");
	if (spot1 >= 0) {
		output = output.substr(spot1+1, output.length() - spot1 - 1);
		int spot2 = output.find_first_of("\"");
		if (spot2 >= 0) {
			output = output.substr(0, spot2);

			s = s.substr(spot2+2, s.length() - spot2 - 1);
		}
	}
	return output;
}

/** Checks if the supplied value is a parameter in the CFG_STRING list. 
 * @param	s	the string to check for in the list
 * @param	m_strings	the list of strings to check for an occurrence of our value within
 * @return	the index of the value in the list if found, otherwise -1. */
int find_CFG_Property(const string & s, const vector<string> & m_strings)
{
	string UPPER_STRING;
	for each (const auto &c in s) 
		UPPER_STRING += toupper(c);
	bool success = false;
	for (auto value = begin(m_strings); value != end(m_strings); value++)
		if ((*value) == UPPER_STRING)
			return std::distance(m_strings.begin(), value);
	return -1;
}

/** Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
 * @brief Uses hard-coded values
 * @param	asset	a shared pointer to fill with the default asset */
void fetch_default_asset(Shared_Asset_Config & asset)
{	
	// Check if a copy already exists
	if (Asset_Manager::Query_Existing_Asset<Asset_Config>(asset, "defaultConfig"))
		return;

	// Create hard-coded alternative
	Asset_Manager::Create_New_Asset<Asset_Config>(asset, "defaultConfig", vector<string>());
	Asset_Manager::Add_Work_Order(new Config_WorkOrder(asset, ""), true);
}

namespace Asset_Loader {
	void load_asset(Shared_Asset_Config & user, const string & filename, const vector<string> & cfg_strings, const bool & threaded)
	{
		// Check if a copy already exists
		if (Asset_Manager::Query_Existing_Asset<Asset_Config>(user, filename))
			return;

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = ABS_DIRECTORY_CONFIG(filename);
		if (!File_Reader::FileExistsOnDisk(fullDirectory)) {
			MSG::Error(FILE_MISSING, fullDirectory);
			fetch_default_asset(user);
			return;
		}

		// Create the asset
		Asset_Manager::Submit_New_Asset<Asset_Config, Config_WorkOrder>(user, threaded, fullDirectory, filename, cfg_strings);
	}
}

void Config_WorkOrder::initializeOrder()
{
	ifstream file_stream(m_filename);
	unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
	for (std::string line; std::getline(file_stream, line); ) {
		if (line.length()) {
			const string cfg_property = get_between_quotes(line);
			int spot = find_CFG_Property(cfg_property, m_asset->m_strings);
			if (spot >= 0) {
				string cfg_value = get_between_quotes(line);
				m_asset->setValue(spot, atof(cfg_value.c_str()));
			}
		}
	}
}

void Config_WorkOrder::finalizeOrder()
{
	if (!m_asset->existsYet()) 
		m_asset->finalize();	
}