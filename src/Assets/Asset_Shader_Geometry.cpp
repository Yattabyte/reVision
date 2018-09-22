#include "Assets\Asset_Shader_Geometry.h"
#include "Assets\Asset_Shader_Pkg.h"
#include "Utilities\IO\Text_IO.h"
#include "Engine.h"


constexpr char* EXT_SHADER_VERTEX = ".vsh";
constexpr char* EXT_SHADER_FRAGMENT = ".fsh";
constexpr char* EXT_SHADER_GEOMETRY = ".gsh";
constexpr char* DIRECTORY_SHADER = "\\Shaders\\";

/** Parse the shader, looking for any directives that require us to modify the document.
@param	engine			the engine being used
@param	userAsset		the asset we are loading from */
inline void parse(Engine * engine, Asset_Shader_Geometry & userAsset)
{
	std::string *text[3] = { &userAsset.m_vertexText, &userAsset.m_fragmentText, &userAsset.m_geometryText };
	for (int x = 0; x < 3; ++x) {
		std::string input;
		if (*text[x] == "") continue;
		input = *text[x];
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
		*text[x] = input;
	}
}

/** Compile a single shader object.
@param	engine		the engine to be used
@param	filename	the shader filename
@param	ID			the shader ID to update
@param	source		the char array representing the document
@param	type		the shader type */
inline void compile_single_shader(Engine * engine, const std::string & filename, GLuint & ID, const char * source, const GLenum & type)
{
	if (strlen(source) > 0) {
		ID = glCreateShader(type);
		glShaderSource(ID, 1, &source, NULL);
		glCompileShader(ID);

		GLint success;
		glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLint infoLogLength;
			glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &infoLogLength);
			std::vector<GLchar> infoLog(infoLogLength);
			glGetShaderInfoLog(ID, infoLog.size(), NULL, &infoLog[0]);
			engine->reportError(MessageManager::SHADER_INCOMPLETE, filename, std::string(infoLog.data(), infoLog.size()));
		}
	}
}

/** Compile all the shaders representing a shader program.
@param	engine		the engine to be used
@param	userAsset	the shader asset to compile */
inline void compile(Engine * engine, Asset_Shader_Geometry & userAsset)
{
	compile_single_shader(engine, userAsset.getFileName(), userAsset.m_glVertexID, userAsset.m_vertexText.c_str(), GL_VERTEX_SHADER);
	compile_single_shader(engine, userAsset.getFileName(), userAsset.m_glFragmentID, userAsset.m_fragmentText.c_str(), GL_FRAGMENT_SHADER);
	compile_single_shader(engine, userAsset.getFileName(), userAsset.m_glGeometryID, userAsset.m_geometryText.c_str(), GL_GEOMETRY_SHADER);
}

/** Generate the shader program.
@param	userAsset	the shader asset to generate for */
void generate_program(Asset_Shader_Geometry & userAsset)
{
	userAsset.m_glProgramID = glCreateProgram();

	if (userAsset.m_glVertexID != 0)
		glAttachShader(userAsset.m_glProgramID, userAsset.m_glVertexID);
	if (userAsset.m_glFragmentID != 0)
		glAttachShader(userAsset.m_glProgramID, userAsset.m_glFragmentID);
	if (userAsset.m_glGeometryID != 0)
		glAttachShader(userAsset.m_glProgramID, userAsset.m_glGeometryID);
}

/** Link the shader program.
@param	engine		the engine to be used
@param	userAsset	the shader asset to link for */
inline void link_program(Engine * engine, Asset_Shader_Geometry & userAsset)
{
	// Link and validate, retrieve any errors
	glLinkProgram(userAsset.m_glProgramID);
	GLint success;
	glGetProgramiv(userAsset.m_glProgramID, GL_LINK_STATUS, &success);
	if (success == 0) {
		GLint infoLogLength;
		glGetProgramiv(userAsset.m_glProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<GLchar> infoLog(infoLogLength);
		glGetProgramInfoLog(userAsset.m_glProgramID, infoLog.size(), NULL, &infoLog[0]);
		engine->reportError(MessageManager::PROGRAM_INCOMPLETE, userAsset.getFileName(), std::string(infoLog.data(), infoLog.size()));
	}
	glValidateProgram(userAsset.m_glProgramID);

	// Delete shader objects, they are already compiled and attached
	if (userAsset.m_glVertexID != 0)
		glDeleteShader(userAsset.m_glVertexID);
	if (userAsset.m_glFragmentID != 0)
		glDeleteShader(userAsset.m_glFragmentID);
	if (userAsset.m_glGeometryID != 0)
		glDeleteShader(userAsset.m_glGeometryID);
}

Asset_Shader_Geometry::~Asset_Shader_Geometry()
{
	if (existsYet()) 
		glDeleteProgram(m_glProgramID);
}

Asset_Shader_Geometry::Asset_Shader_Geometry(const std::string & filename) : Asset_Shader(filename) {}

Shared_Asset_Shader_Geometry Asset_Shader_Geometry::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();
	
	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Shader_Geometry>(filename, threaded);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Shader_Geometry>(filename);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string &relativePath = "\\Shaders\\" + filename;
		std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, relativePath);
		std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);
		bool found_vertex = Engine::File_Exists(relativePath + EXT_SHADER_VERTEX);
		bool found_fragement = Engine::File_Exists(relativePath + EXT_SHADER_FRAGMENT);
		if (!found_vertex)
			engine->reportError(MessageManager::FILE_MISSING, relativePath + EXT_SHADER_VERTEX);
		if (!found_fragement)
			engine->reportError(MessageManager::FILE_MISSING, relativePath + EXT_SHADER_FRAGMENT);
		if (!(found_vertex && found_fragement))
			initFunc = std::bind(&initializeDefault, &assetRef, engine);

		// Submit the work order
		assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);
	}
	return userAsset;
}

void Asset_Shader_Geometry::initializeDefault(Engine * engine)
{	
	// Create hard-coded alternative	
	m_vertexText = "#version 430\n\nlayout(location = 0) in glm::vec3 vertex;\n\nvoid main()\n{\n\tgl_Position = glm::vec4(vertex, 1.0);\n}";
	m_fragmentText = "#version 430\n\nlayout (location = 0) out glm::vec4 fragColor;\n\nvoid main()\n{\n\tfragColor = glm::vec4(1.0f);\n}";	
}

void Asset_Shader_Geometry::initialize(Engine * engine, const std::string & relativePath)
{
	const bool found_vertex = Text_IO::Import_Text(engine, relativePath + EXT_SHADER_VERTEX, m_vertexText);
	const bool found_fragement = Text_IO::Import_Text(engine, relativePath + EXT_SHADER_FRAGMENT, m_fragmentText);
	const bool found_geometry = Text_IO::Import_Text(engine, relativePath + EXT_SHADER_GEOMETRY, m_geometryText);

	if (!(found_vertex && found_fragement)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Shader_Geometry");
		initializeDefault(engine);
		return;
	}

	// parse
	parse(engine, *this);
}

void Asset_Shader_Geometry::finalize(Engine * engine)
{
	// Create Shader Program	
	compile(engine, *this);
	generate_program(*this);
	link_program(engine, *this);

	// Finalize
	Asset::finalize(engine);
}