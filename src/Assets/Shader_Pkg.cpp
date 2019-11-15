#include "Assets/Shader_Pkg.h"
#include "Utilities/IO/Text_IO.h"
#include "Engine.h"
#include "glm/gtc/type_ptr.hpp"


constexpr const char* EXT_PACKAGE = ".pkg";
constexpr const char* DIRECTORY_SHADER_PKG = "\\Shaders\\";

/** Parse the shader snippet, looking for any directives that require us to modify the document.
@param	engine			the engine being used
@param	userAsset		the asset we are loading from */
inline static void parse(Engine* engine, Shader_Pkg& userAsset) noexcept
{
	std::string input;
	input = userAsset.m_packageText;
	if (input.empty()) return;
	// Find Package to include
	size_t spot = input.find("#package");
	while (spot != std::string::npos) {
		std::string directory = input.substr(spot);

		size_t qspot1 = directory.find('\"');
		size_t qspot2 = directory.find('\"', qspot1 + 1);
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

Shared_Shader_Pkg::Shared_Shader_Pkg(Engine* engine, const std::string& filename, const bool& threaded) noexcept
{
	(*(std::shared_ptr<Shader_Pkg>*)(this)) = std::dynamic_pointer_cast<Shader_Pkg>(
		engine->getManager_Assets().shareAsset(
			typeid(Shader_Pkg).name(),
			filename,
			[engine, filename]() { return std::make_shared<Shader_Pkg>(engine, filename); },
			threaded
		));
}

Shader_Pkg::Shader_Pkg(Engine* engine, const std::string& filename) noexcept : Asset(engine, filename) {}

void Shader_Pkg::initialize() noexcept
{
	const bool found = Text_IO::Import_Text(m_engine, DIRECTORY_SHADER_PKG + getFileName() + EXT_PACKAGE, m_packageText);

	if (!found)
		m_engine->getManager_Messages().error("Shader_Pkg \"" + m_filename + "\" file does not exist");

	// parse
	parse(m_engine, *this);

	Asset::finalize();
}