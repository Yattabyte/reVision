#include "Assets\Asset_Shader_Pkg.h"
#include "Managers\Message_Manager.h"
#include "GLM\gtc\type_ptr.hpp"
#include <fstream>

/* -----ASSET TYPE----- */
#define ASSET_TYPE 7

using namespace Asset_Loader;

Asset_Shader_Pkg::~Asset_Shader_Pkg()
{
}

Asset_Shader_Pkg::Asset_Shader_Pkg(const string & filename) : Asset(filename)
{	
	package_text = "";
}

int Asset_Shader_Pkg::GetAssetType() 
{ 
	return ASSET_TYPE;
}

bool Asset_Shader_Pkg::ExistsYet()
{
	return Asset::ExistsYet();
}

string Asset_Shader_Pkg::getPackageText() const
{
	return package_text;
}

// Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
// Uses hardcoded values
void fetchDefaultAsset(Shared_Asset_Shader_Pkg & asset)
{	
	// Check if a copy already exists
	if (Asset_Manager::QueryExistingAsset<Asset_Shader_Pkg>(asset, ""))
		return;

	// Create hardcoded alternative
	Asset_Manager::CreateNewAsset<Asset_Shader_Pkg>(asset, "");
	Asset_Manager::AddWorkOrder(new Shader_Pkg_WorkOrder(asset, ""), true);
}

namespace Asset_Loader {
	void load_asset(Shared_Asset_Shader_Pkg & user, const string & filename, const bool & threaded)
	{
		// Check if a copy already exists
		if (Asset_Manager::QueryExistingAsset<Asset_Shader_Pkg>(user, filename))
			return;

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = DIRECTORY_SHADER_PKG + filename;
		bool found = FileReader::FileExistsOnDisk(fullDirectory + EXT_PACKAGE);
		if (!found) {
			MSG::Error(FILE_MISSING, fullDirectory + EXT_PACKAGE);
			fetchDefaultAsset(user);
			return;
		}

		// Create the asset
		Asset_Manager::CreateNewAsset<Asset_Shader_Pkg, Shader_Pkg_WorkOrder>(user, threaded, fullDirectory, filename);
	}
}

void Shader_Pkg_WorkOrder::Initialize_Order()
{
	unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
	bool found = FetchFileFromDisk(m_asset->package_text, m_filename + EXT_PACKAGE);
	write_guard.unlock();
	write_guard.release();

	if (!found)
		MSG::Error(FILE_MISSING, m_asset->GetFileName() + EXT_PACKAGE);

	// Parse
	Parse();
}

void Shader_Pkg_WorkOrder::Parse()
{
	string input = m_asset->package_text;
	if (input == "") return;
	// Find Package to include
	int spot = input.find("#package");
	while (spot != string::npos) {
		string directory = input.substr(spot);

		unsigned int qspot1 = directory.find("\"");
		unsigned int qspot2 = directory.find("\"", qspot1 + 1);
		// find string quotes and remove them
		directory = directory.substr(qspot1 + 1, qspot2 - 1 - qspot1);

		Shared_Asset_Shader_Pkg package;
		Asset_Loader::load_asset(package, directory, false);
		string left = input.substr(0, spot);
		string right = input.substr(spot + 1 + qspot2);
		input = left + package->getPackageText() + right;
		spot = input.find("#package");
	}
	unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
	m_asset->package_text = input;	
}


void Shader_Pkg_WorkOrder::Finalize_Order()
{
	if (!m_asset->ExistsYet()) 
		m_asset->Finalize();	
}

// Reads in a text file from disk, given a file directory, and appends it to the returnFile param
// Returns true if succeeded, false if file doesn't exist
bool Shader_Pkg_WorkOrder::FetchFileFromDisk(string & returnFile, const string & fileDirectory)
{
	struct stat buffer;
	if (stat(fileDirectory.c_str(), &buffer))
		return false;

	ifstream file(fileDirectory);
	while (!file.eof()) {
		string temp;
		std::getline(file, temp);
		returnFile.append(temp + '\n');
	}

	return true;
}