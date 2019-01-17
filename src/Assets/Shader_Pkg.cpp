#include "Assets/Shader_Pkg.h"
#include "Utilities/IO/Text_IO.h"
#include "Engine.h"
#include "glm/gtc/type_ptr.hpp"


constexpr char* EXT_PACKAGE = ".pkg";
constexpr char* DIRECTORY_SHADER_PKG = "\\Shaders\\";

/** Parse the shader snippet, looking for any directives that require us to modify the document.
@param	engine			the engine being used
@param	userAsset		the asset we are loading from */
inline void parse(Engine * engine, Shader_Pkg & userAsset)
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

		Shared_Shader_Pkg package = Shared_Shader_Pkg(engine, directory, false);
		std::string left = input.substr(0, spot);
		std::string right = input.substr(spot + 1 + qspot2);
		input = left + package->getPackageText() + right;
		spot = input.find("#package");
	}	
	userAsset.m_packageText = input;
}

Shared_Shader_Pkg::Shared_Shader_Pkg(Engine * engine, const std::string & filename, const bool & threaded)
	: std::shared_ptr<Shader_Pkg>(std::dynamic_pointer_cast<Shader_Pkg>(engine->getManager_Assets().shareAsset(typeid(Shader_Pkg).name(), filename)))
{
	// Find out if the asset needs to be created
	if (!get()) {
		// Create new asset on shared_ptr portion of this class 
		(*(std::shared_ptr<Shader_Pkg>*)(this)) = std::make_shared<Shader_Pkg>(filename);
		// Submit data to asset manager
		engine->getManager_Assets().submitNewAsset(typeid(Shader_Pkg).name(), (*(std::shared_ptr<Asset>*)(this)), std::move(std::bind(&Shader_Pkg::initialize, get(), engine, (DIRECTORY_SHADER_PKG + filename))), threaded);
	}
	// Check if we need to wait for initialization
	else
		if (!threaded)
			// Stay here until asset finalizes
			while (!get()->existsYet())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

Shader_Pkg::Shader_Pkg(const std::string & filename) : Asset(filename) {}

void Shader_Pkg::initialize(Engine * engine, const std::string & relativePath)
{
	const bool found = Text_IO::Import_Text(engine, relativePath + EXT_PACKAGE, m_packageText);
	
	if (!found)
		engine->getManager_Messages().error("Shader_Pkg \"" + m_filename + "\" file does not exist");

	// parse
	parse(engine, *this);

	Asset::finalize(engine);
}