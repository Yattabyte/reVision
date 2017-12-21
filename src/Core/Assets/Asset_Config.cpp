#include "Assets\Asset_Config.h"
#include "Systems\Message_Manager.h"
#include <fstream>

/* -----ASSET TYPE----- */
#define ASSET_TYPE 1

using namespace Asset_Loader;

Asset_Config::~Asset_Config()
{
}

Asset_Config::Asset_Config(const string & filename) : Asset(filename)
{
}

int Asset_Config::GetAssetType()
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

	string directory = ABS_DIRECTORY_CONFIG(GetFileName());
	ofstream out(directory);
	out << output.c_str();
}

// Attempts to retrieve a string between quotation marks "<string>"
string getBetweenQuotes(string & s)
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

// Checks if the value @s is a parameter in the CFG_STRING list. 
// If true, returns the spot in the list @s is. 
// If false, returns -1.
int findCFGProperty(const string & s, const vector<string> & m_strings)
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

// Retrieves (or initializes) a default asset from the Asset_Manager.
// Can hard code default configuration parameters here.
// Saves to disk afterwards.
Shared_Asset_Config fetchDefaultAsset()
{
	shared_lock<shared_mutex> guard(Asset_Manager::GetMutex_Assets());
	std::map<int, Shared_Asset> &fallback_assets = Asset_Manager::GetFallbackAssets_Map();
	fallback_assets.insert(std::pair<int, Shared_Asset>(Asset_Config::GetAssetType(), Shared_Asset()));
	auto &default_asset = fallback_assets[Asset_Config::GetAssetType()];
	guard.unlock();
	guard.release();
	if (default_asset.get() == nullptr) { // Check if we already created the default asset
		default_asset = shared_ptr<Asset_Config>(new Asset_Config("defaultConfig"));
		Shared_Asset_Config cast_asset = dynamic_pointer_cast<Asset_Config>(default_asset);
		return cast_asset;
	}
	return dynamic_pointer_cast<Asset_Config>(default_asset);
}

namespace Asset_Loader {
	void load_asset(Shared_Asset_Config & user, const string & filename, const vector<string> & cfg_strings, const bool & threaded)
	{
		// Check if a copy already exists
		shared_mutex &mutex_IO_assets = Asset_Manager::GetMutex_Assets();
		auto &assets_configs = (Asset_Manager::GetAssets_List(Asset_Config::GetAssetType()));
		{
			shared_lock<shared_mutex> guard(mutex_IO_assets);
			for each (auto &asset in assets_configs) {
				shared_lock<shared_mutex> asset_guard(asset->m_mutex);
				const Shared_Asset_Config derived_asset = dynamic_pointer_cast<Asset_Config>(asset);
				if (derived_asset) { // Check that pointer isn't null after dynamic pointer casting
					if (derived_asset->GetFileName() == filename) { // Filenames match, use this asset
						asset_guard.unlock();
						asset_guard.release();
						user = derived_asset;
						// Can't guarantee that the asset isn't already being worked on, so no finalization here if threaded
						return;
					}
				}
			}
		}

		// Attempt to create the asset
		const std::string &fulldirectory = ABS_DIRECTORY_CONFIG(filename);
		if (!FileReader::FileExistsOnDisk(fulldirectory)) {
			MSG::Error(FILE_MISSING, fulldirectory);
			user = fetchDefaultAsset();
			return;
		}
		else {
			unique_lock<shared_mutex> guard(mutex_IO_assets);
			user = Shared_Asset_Config(new Asset_Config(filename));
			user->m_strings = cfg_strings;
			assets_configs.push_back(user);
		}

		// Either continue processing on a new thread or stay on the current one
		bool *complete = new bool(false);
		if (threaded) 
			Asset_Manager::AddWorkOrder(new Config_WorkOrder(user, fulldirectory));	
		else {
			Config_WorkOrder work_order(user, fulldirectory);
			work_order.Initialize_Order();
			work_order.Finalize_Order();
		}
	}
}

void Config_WorkOrder::Initialize_Order()
{
	ifstream file_stream(m_filename);
	unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
	for (std::string line; std::getline(file_stream, line); ) {
		if (line.length()) {
			const string cfg_property = getBetweenQuotes(line);
			int spot = findCFGProperty(cfg_property, m_asset->m_strings);
			if (spot >= 0) {
				string cfg_value = getBetweenQuotes(line);
				m_asset->setValue(spot, atof(cfg_value.c_str()));
			}
		}
	}
}

void Config_WorkOrder::Finalize_Order()
{
	shared_lock<shared_mutex> read_guard(m_asset->m_mutex);
	if (!m_asset->ExistsYet()) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
		m_asset->Finalize();
	}
}