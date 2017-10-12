#include "Assets\Asset_Config.h"
#include <fstream>

using namespace Asset_Manager;

Asset_Config::~Asset_Config()
{
}

Asset_Config::Asset_Config()
{
	filename = "";
}

int Asset_Config::GetAssetType()
{
	return 1;
}

void Asset_Config::Finalize()
{
	return void();
}

void Asset_Config::setValue(const int & cfg_key, const float & cfg_value)
{
	// Try inserting the value by key in case the key doesn't exist.
	configuration.insert(pair<int, float>(cfg_key, cfg_value));
	configuration[cfg_key] = cfg_value;
}

float Asset_Config::getValue(const int & cfg_key)
{
	if (cfg_key >= 0 && configuration.find(cfg_key) != configuration.end())
		return configuration[cfg_key];
	return UNDEFINED_CVAL;
}

void Asset_Config::saveConfig()
{
	string output;

	for each (const auto &value in configuration) 
		output += "\"" + CFG_STRING[value.first] + "\" \"" + to_string(value.second) + "\"\n";	

	string directory = getCurrentDir() + "\\Config\\" + filename;
	ofstream out(directory);
	out << output.c_str();
}

// Retrieves (or initializes) a default asset from the Asset_Manager.
// Can hard code default configuration parameters here.
// Saves to disk afterwards.
Shared_Asset_Config fetchDefaultAsset()
{
	shared_lock<shared_mutex> guard(getMutexIOAssets());
	std::map<int, Shared_Asset> &fallback_assets = getFallbackAssets();
	fallback_assets.insert(std::pair<int, Shared_Asset>(Asset_Config::GetAssetType(), Shared_Asset()));
	auto &default_asset = fallback_assets[Asset_Config::GetAssetType()];
	if (default_asset.get() == nullptr) {
		default_asset = shared_ptr<Asset_Config>(new Asset_Config());
		Shared_Asset_Config cast_asset = dynamic_pointer_cast<Asset_Config>(default_asset);
		
		/* HARD CODE DEFAULT VALUES HERE */
		cast_asset->filename = "config.cfg";
		cast_asset->setValue(CFG_ENUM::C_WINDOW_WIDTH, 512);
		cast_asset->setValue(CFG_ENUM::C_WINDOW_HEIGHT, 512);
		cast_asset->saveConfig();
		return cast_asset; // already casted, might as well return it rather than recasting it
	}
	return dynamic_pointer_cast<Asset_Config>(default_asset);
}

// Attempts to retrieve a string between quotation marks "<string>"
string getBetweenQuotes(string &s)
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
int findCFGProperty(const string &s)
{
	string UPPER_STRING;
	for each (const auto &c in s) 
		UPPER_STRING += toupper(c);
	bool success = false;
	for (auto value = begin(CFG_STRING); value != end(CFG_STRING); value++)
		if ((*value) == UPPER_STRING)
			return distance(CFG_STRING.begin(), value);
	return -1;
}

// Reads in the configuration file from disk.
void initialize_Config(Shared_Asset_Config & user, const string & filename, bool * complete)
{
	ifstream file_stream(filename);
	unique_lock<shared_mutex> write_guard(user->m_mutex);
	for (std::string line; std::getline(file_stream, line); ) {
		if (file_stream.good()) {
			const string cfg_property = getBetweenQuotes(line);
			int spot = findCFGProperty(cfg_property);
			if (spot >= 0) {
				string cfg_value = getBetweenQuotes(line);
				user->setValue(spot, atof(cfg_value.c_str()));
			}
		}
	}

	submitWorkorder(user);
	*complete = true;
}

namespace Asset_Manager {
	void load_asset(Shared_Asset_Config & user, const string & filename, const bool & threaded)
	{
		// Check if a copy already exists
		shared_mutex &mutex_IO_assets = getMutexIOAssets();
		auto &assets_shaders = (fetchAssetList(Asset_Config::GetAssetType()));
		{
			shared_lock<shared_mutex> guard(mutex_IO_assets);
			for each (auto &asset in assets_shaders) {
				shared_lock<shared_mutex> asset_guard(asset->m_mutex);
				const Shared_Asset_Config derived_asset = dynamic_pointer_cast<Asset_Config>(asset);
				if (derived_asset) { // Check that pointer isn't null after dynamic pointer casting
					if (derived_asset->filename == filename) { // Filenames match, use this asset
						asset_guard.unlock();
						asset_guard.release();
						user = derived_asset;
						// If we don't want multithreading, try to create the asset now.
						// It is OK if the first time this asset was requested was Multithreaded but this one isn't!
						// Because finalize can be called multiple times safely, it checks to see if the content was already created.
						if (!threaded) {
							user->Finalize();
							return;
						}
					}
				}
			}
		}

		// Attempt to create the asset
		const std::string &fulldirectory = getCurrentDir() + "\\Config\\" + filename;
		if (!fileOnDisk(fulldirectory)) {
			user = fetchDefaultAsset();
			return;
		}
		else {
			unique_lock<shared_mutex> guard(mutex_IO_assets);
			user = Shared_Asset_Config(new Asset_Config());
			user->filename = filename;
			assets_shaders.push_back(user);
		}

		// Either continue processing on a new thread or stay on the current one
		bool *complete = new bool(false);
		if (threaded) {
			thread *import_thread = new thread(initialize_Config, user, fulldirectory, complete);
			import_thread->detach();
			submitWorkthread(std::pair<thread*, bool*>(import_thread, complete));
		}
		else {
			initialize_Config(user, fulldirectory, complete);
			user->Finalize();
		}
	}
}