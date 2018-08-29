#include "Assets\Asset_Shader_Pkg.h"
#include "Utilities\IO\Text_IO.h"
#include "Engine.h"
#include "GLM\gtc\type_ptr.hpp"

#define EXT_PACKAGE ".pkg"
#define DIRECTORY_SHADER_PKG Engine::Get_Current_Dir() + "\\Shaders\\"


/** Parse the shader snippet, looking for any directives that require us to modify the document.
@param	engine			the engine being used
@param	userAsset		the asset we are loading from */
inline void parse(Engine * engine, Asset_Shader_Pkg & userAsset)
{
	std::string input;
	{
		std::shared_lock<std::shared_mutex> read_guard(userAsset.m_mutex);
		input = userAsset.m_packageText;
		if (input == "") return;
		// Find Package to include
		size_t spot = input.find("#package");
		while (spot != std::string::npos) {
			std::string directory = input.substr(spot);

			size_t qspot1 = directory.find("\"");
			size_t qspot2 = directory.find("\"", qspot1 + 1);
			// find std::string quotes and remove them
			directory = directory.substr(qspot1 + 1, qspot2 - 1 - qspot1);

			Shared_Asset_Shader_Pkg package = Asset_Shader_Pkg::Create(engine, directory, false);
			std::shared_lock<std::shared_mutex> read_guardPKG(package->m_mutex);
			std::string left = input.substr(0, spot);
			std::string right = input.substr(spot + 1 + qspot2);
			input = left + package->getPackageText() + right;
			spot = input.find("#package");
		}
	}
	std::unique_lock<std::shared_mutex> write_guard(userAsset.m_mutex);
	userAsset.m_packageText = input;
}

Asset_Shader_Pkg::~Asset_Shader_Pkg()
{
}

Asset_Shader_Pkg::Asset_Shader_Pkg(const std::string & filename) : Asset(filename)
{	
	m_packageText = "";
}

Shared_Asset_Shader_Pkg Asset_Shader_Pkg::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Shader_Pkg>(filename);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Shader_Pkg>(filename);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = DIRECTORY_SHADER_PKG + filename;
		std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, fullDirectory);
		std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);
		const bool found = Engine::File_Exists(fullDirectory + EXT_PACKAGE);
		if (!found) {
			engine->reportError(MessageManager::FILE_MISSING, fullDirectory + EXT_PACKAGE);
			initFunc = std::bind(&initializeDefault, &assetRef, engine);
		}

		// Submit the work order
		assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);
	}
	return userAsset;
}

void Asset_Shader_Pkg::initializeDefault(Engine * engine)
{
	// Create hard-coded alternative
}

void Asset_Shader_Pkg::initialize(Engine * engine, const std::string & fullDirectory)
{
	std::unique_lock<std::shared_mutex> write_guard(m_mutex);
	const bool found = Text_IO::Import_Text(engine, fullDirectory + EXT_PACKAGE, m_packageText);
	write_guard.unlock();
	write_guard.release();

	if (!found)
		engine->reportError(MessageManager::FILE_MISSING, getFileName() + EXT_PACKAGE);

	// parse
	parse(engine, *this);
}

void Asset_Shader_Pkg::finalize(Engine * engine)
{
	Asset::finalize(engine);
}

std::string Asset_Shader_Pkg::getPackageText() const
{
	return m_packageText;
}