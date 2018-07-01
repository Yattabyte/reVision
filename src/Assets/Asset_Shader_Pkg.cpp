#include "Assets\Asset_Shader_Pkg.h"
#include "Managers\Message_Manager.h"
#include "GLM\gtc\type_ptr.hpp"
#include <fstream>

/** Parse the shader snippet, looking for any directives that require us to modify the document.
 * @param	assetManager	the asset manager to use
 * @param	userAsset		the asset we are loading from */
inline void parse(AssetManager & assetManager, Shared_Asset_Shader_Pkg & userAsset)
{
	string input = userAsset->m_packageText;
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
		assetManager.create(package, directory, false);
		string left = input.substr(0, spot);
		string right = input.substr(spot + 1 + qspot2);
		input = left + package->getPackageText() + right;
		spot = input.find("#package");
	}
	unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
	userAsset->m_packageText = input;
}

/** Read a file from disk.
 * @param	returnFile		the destination to load the text into
 * @param	fileDirectory	the file directory to load from
 * @return					returns true if file read successfull, false otherwise */
inline bool fetch_file_from_disk(string & returnFile, const string & fileDirectory)
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

Asset_Shader_Pkg::~Asset_Shader_Pkg()
{
}

Asset_Shader_Pkg::Asset_Shader_Pkg(const string & filename) : Asset(filename)
{	
	m_packageText = "";
}

void Asset_Shader_Pkg::CreateDefault(AssetManager & assetManager, Shared_Asset_Shader_Pkg & userAsset)
{
	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, ""))
		return;

	// Create hard-coded alternative
	assetManager.createNewAsset(userAsset, "");

	// Create the asset
	assetManager.submitNewWorkOrder(userAsset, true,
		/* Initialization. */
		[]() {},
		/* Finalization. */
		[&assetManager, &userAsset]() { Finalize(assetManager, userAsset); }
	);
}

void Asset_Shader_Pkg::Create(AssetManager & assetManager, Shared_Asset_Shader_Pkg & userAsset, const string & filename, const bool & threaded)
{
	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const std::string &fullDirectory = DIRECTORY_SHADER_PKG + filename;
	bool found = File_Reader::FileExistsOnDisk(fullDirectory + EXT_PACKAGE);
	if (!found) {
		MSG_Manager::Error(MSG_Manager::FILE_MISSING, fullDirectory + EXT_PACKAGE);
		CreateDefault(assetManager, userAsset);
		return;
	}

	// Create the asset
	assetManager.submitNewAsset(userAsset, threaded,
		/* Initialization. */
		[&assetManager, &userAsset, fullDirectory]() mutable { Initialize(assetManager, userAsset, fullDirectory); },
		/* Finalization. */
		[&assetManager, &userAsset]() mutable { Finalize(assetManager, userAsset); },
		/* Constructor Arguments. */
		filename
	);
}

void Asset_Shader_Pkg::Initialize(AssetManager & assetManager, Shared_Asset_Shader_Pkg & userAsset, const string & fullDirectory)
{
	unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
	bool found = fetch_file_from_disk(userAsset->m_packageText, fullDirectory + EXT_PACKAGE);
	write_guard.unlock();
	write_guard.release();

	if (!found)
		MSG_Manager::Error(MSG_Manager::FILE_MISSING, userAsset->getFileName() + EXT_PACKAGE);

	// parse
	parse(assetManager, userAsset);
}

void Asset_Shader_Pkg::Finalize(AssetManager & assetManager, Shared_Asset_Shader_Pkg & userAsset)
{
	unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
	userAsset->m_finalized = true;
	write_guard.unlock();
	write_guard.release();
	shared_lock<shared_mutex> read_guard(userAsset->m_mutex);
	for each (auto qwe in userAsset->m_callbacks)
		assetManager.submitNotifyee(qwe.second);
	/* To Do: Finalize call here*/
}

string Asset_Shader_Pkg::getPackageText() const
{
	return m_packageText;
}