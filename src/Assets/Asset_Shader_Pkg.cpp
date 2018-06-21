#include "Assets\Asset_Shader_Pkg.h"
#include "Managers\Message_Manager.h"
#include "GLM\gtc\type_ptr.hpp"
#include <fstream>


/** Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
 * @brief Uses hard-coded values
 * @param	asset	a shared pointer to fill with the default asset */
void fetch_default_asset(Shared_Asset_Shader_Pkg & userAsset)
{
	// Check if a copy already exists
	if (Asset_Manager::Query_Existing_Asset<Asset_Shader_Pkg>(userAsset, ""))
		return;

	// Create hard-coded alternative
	Asset_Manager::Create_New_Asset<Asset_Shader_Pkg>(userAsset, "");
	Asset_Manager::Add_Work_Order(new Shader_Pkg_WorkOrder(userAsset, ""), true);
}

Asset_Shader_Pkg::~Asset_Shader_Pkg()
{
}

Asset_Shader_Pkg::Asset_Shader_Pkg(const string & filename) : Asset(filename)
{	
	m_packageText = "";
}

void Asset_Shader_Pkg::Create(Shared_Asset_Shader_Pkg & userAsset, const string & filename, const bool & threaded)
{
	// Check if a copy already exists
	if (Asset_Manager::Query_Existing_Asset<Asset_Shader_Pkg>(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const std::string &fullDirectory = DIRECTORY_SHADER_PKG + filename;
	bool found = File_Reader::FileExistsOnDisk(fullDirectory + EXT_PACKAGE);
	if (!found) {
		MSG_Manager::Error(MSG_Manager::FILE_MISSING, fullDirectory + EXT_PACKAGE);
		fetch_default_asset(userAsset);
		return;
	}

	// Create the asset
	Asset_Manager::Submit_New_Asset<Asset_Shader_Pkg, Shader_Pkg_WorkOrder>(userAsset, threaded, fullDirectory, filename);
}

string Asset_Shader_Pkg::getPackageText() const
{
	return m_packageText;
}

void Shader_Pkg_WorkOrder::initializeOrder()
{
	unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
	bool found = fetchFileFromDisk(m_asset->m_packageText, m_filename + EXT_PACKAGE);
	write_guard.unlock();
	write_guard.release();

	if (!found)
		MSG_Manager::Error(MSG_Manager::FILE_MISSING, m_asset->getFileName() + EXT_PACKAGE);

	// Parse
	parse();
}

void Shader_Pkg_WorkOrder::parse()
{
	string input = m_asset->m_packageText;
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
		Asset_Shader_Pkg::Create(package, directory, false);
		string left = input.substr(0, spot);
		string right = input.substr(spot + 1 + qspot2);
		input = left + package->getPackageText() + right;
		spot = input.find("#package");
	}
	unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
	m_asset->m_packageText = input;	
}


void Shader_Pkg_WorkOrder::finalizeOrder()
{
	if (!m_asset->existsYet()) 
		m_asset->finalize();	
}

bool Shader_Pkg_WorkOrder::fetchFileFromDisk(string & returnFile, const string & fileDirectory)
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