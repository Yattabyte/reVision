#include "Assets\Asset_Shader_Pkg.h"
#include "Utilities\IO\Text_IO.h"
#include "Engine.h"
#include "GLM\gtc\type_ptr.hpp"
#define EXT_PACKAGE ".pkg"
#define DIRECTORY_SHADER_PKG Engine::Get_Current_Dir() + "\\Shaders\\"


/** Parse the shader snippet, looking for any directives that require us to modify the document.
 * @param	engine			the engine being used
 * @param	userAsset		the asset we are loading from */
inline void parse(Engine * engine, Shared_Asset_Shader_Pkg & userAsset)
{
	std::string input = userAsset->m_packageText;
	if (input == "") return;
	// Find Package to include
	int spot = input.find("#package");
	while (spot != std::string::npos) {
		std::string directory = input.substr(spot);

		unsigned int qspot1 = directory.find("\"");
		unsigned int qspot2 = directory.find("\"", qspot1 + 1);
		// find std::string quotes and remove them
		directory = directory.substr(qspot1 + 1, qspot2 - 1 - qspot1);

		Shared_Asset_Shader_Pkg package;
		engine->createAsset(package, directory, false);
		std::string left = input.substr(0, spot);
		std::string right = input.substr(spot + 1 + qspot2);
		input = left + package->getPackageText() + right;
		spot = input.find("#package");
	}
	std::unique_lock<std::shared_mutex> write_guard(userAsset->m_mutex);
	userAsset->m_packageText = input;
}

Asset_Shader_Pkg::~Asset_Shader_Pkg()
{
}

Asset_Shader_Pkg::Asset_Shader_Pkg(const std::string & filename) : Asset(filename)
{	
	m_packageText = "";
}

void Asset_Shader_Pkg::CreateDefault(Engine * engine, Shared_Asset_Shader_Pkg & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();

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
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); }
	);
}

void Asset_Shader_Pkg::Create(Engine * engine, Shared_Asset_Shader_Pkg & userAsset, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const std::string &fullDirectory = DIRECTORY_SHADER_PKG + filename;
	const bool found = Engine::File_Exists(fullDirectory + EXT_PACKAGE);
	if (!found) {
		engine->reportError(MessageManager::FILE_MISSING, fullDirectory + EXT_PACKAGE);
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
		filename
	);
}

void Asset_Shader_Pkg::Initialize(Engine * engine, Shared_Asset_Shader_Pkg & userAsset, const std::string & fullDirectory)
{
	std::unique_lock<std::shared_mutex> write_guard(userAsset->m_mutex);
	const bool found = Text_IO::Import_Text(engine, fullDirectory + EXT_PACKAGE, userAsset->m_packageText);
	write_guard.unlock();
	write_guard.release();

	if (!found)
		engine->reportError(MessageManager::FILE_MISSING, userAsset->getFileName() + EXT_PACKAGE);

	// parse
	parse(engine, userAsset);
}

void Asset_Shader_Pkg::Finalize(Engine * engine, Shared_Asset_Shader_Pkg & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();
	userAsset->finalize();

	// Notify Completion
	std::shared_lock<std::shared_mutex> read_guard(userAsset->m_mutex);
	for each (auto qwe in userAsset->m_callbacks)
		assetManager.submitNotifyee(qwe.second);
}

std::string Asset_Shader_Pkg::getPackageText() const
{
	return m_packageText;
}