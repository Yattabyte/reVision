#include "Assets\Asset_Shader_Pkg.h"
#include "Utilities\IO\Text_IO.h"
#include "Engine.h"
#include "GLM\gtc\type_ptr.hpp"


constexpr char* EXT_PACKAGE = ".pkg";
constexpr char* DIRECTORY_SHADER_PKG = "\\Shaders\\";

/** Parse the shader snippet, looking for any directives that require us to modify the document.
@param	engine			the engine being used
@param	userAsset		the asset we are loading from */
inline void parse(Engine * engine, Asset_Shader_Pkg & userAsset)
{
	std::string input;
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
		std::string left = input.substr(0, spot);
		std::string right = input.substr(spot + 1 + qspot2);
		input = left + package->getPackageText() + right;
		spot = input.find("#package");
	}	
	userAsset.m_packageText = input;
}

Asset_Shader_Pkg::Asset_Shader_Pkg(const std::string & filename) : Asset(filename) {}

Shared_Asset_Shader_Pkg Asset_Shader_Pkg::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Shader_Pkg>(filename, threaded);
	if (!userAsset) {
		userAsset = std::make_shared<Asset_Shader_Pkg>(filename);
		assetManager.addShareableAsset(userAsset);
		
		// Submit the work order
		const std::string relativePath(DIRECTORY_SHADER_PKG + filename);	
		assetManager.submitNewWorkOrder(std::move(std::bind(&initialize, userAsset.get(), engine, relativePath)), threaded);
	}
	return userAsset;
}

void Asset_Shader_Pkg::initialize(Engine * engine, const std::string & relativePath)
{
	const bool found = Text_IO::Import_Text(engine, relativePath + EXT_PACKAGE, m_packageText);
	
	if (!found)
		engine->reportError(MessageManager::FILE_MISSING, getFileName() + EXT_PACKAGE);

	// parse
	parse(engine, *this);

	Asset::finalize(engine);
}

std::string Asset_Shader_Pkg::getPackageText() const
{
	return m_packageText;
}