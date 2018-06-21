#include "Assets\Asset_Shader.h"
#include "Managers\Message_Manager.h"
#include <fstream>


/** Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
 * @brief Uses hard-coded values
 * @param	asset	a shared pointer to fill with the default asset */
void fetch_default_asset(Shared_Asset_Shader & userAsset)
{
	// Check if a copy already exists
	if (Asset_Manager::Query_Existing_Asset<Asset_Shader>(userAsset, "defaultShader"))
		return;

	// Create hard-coded alternative
	Asset_Manager::Create_New_Asset<Asset_Shader>(userAsset, "defaultShader");
	userAsset->m_vertexText = "#version 430\n\nlayout(location = 0) in vec3 vertex;\n\nvoid main()\n{\n\tgl_Position = vec4(vertex, 1.0);\n}";
	userAsset->m_fragmentText = "#version 430\n\nlayout (location = 0) out vec4 fragColor;\n\nvoid main()\n{\n\tfragColor = vec4(1.0f);\n}";
	Asset_Manager::Add_Work_Order(new Shader_WorkOrder(userAsset, ""), true);
}

Asset_Shader::~Asset_Shader()
{
	if (existsYet()) 
		glDeleteProgram(m_glProgramID);
}

Asset_Shader::Asset_Shader(const string & filename) : Asset(filename)
{
	m_glProgramID = 0;
	m_glVertexID = 0;
	m_glFragmentID = 0;
	m_glGeometryID = 0;
	m_vertexText = "";
	m_fragmentText = "";
	m_geometryText = "";
}

void Asset_Shader::Create(Shared_Asset_Shader & userAsset, const string & filename, const bool & threaded)
{
	// Check if a copy already exists
	if (Asset_Manager::Query_Existing_Asset<Asset_Shader>(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const std::string &fullDirectory = DIRECTORY_SHADER + filename;
	bool found_vertex = File_Reader::FileExistsOnDisk(fullDirectory + EXT_SHADER_VERTEX);
	bool found_fragement = File_Reader::FileExistsOnDisk(fullDirectory + EXT_SHADER_FRAGMENT);
	if (!found_vertex)
		MSG_Manager::Error(MSG_Manager::FILE_MISSING, fullDirectory + EXT_SHADER_VERTEX);
	if (!found_fragement)
		MSG_Manager::Error(MSG_Manager::FILE_MISSING, fullDirectory + EXT_SHADER_FRAGMENT);
	if (!(found_vertex && found_fragement)) {
		fetch_default_asset(userAsset);
		return;
	}

	// Create the asset
	Asset_Manager::Submit_New_Asset<Asset_Shader, Shader_WorkOrder>(userAsset, threaded, fullDirectory, filename);
}

void Asset_Shader::bind()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	glUseProgram(m_glProgramID);
}

void Asset_Shader::Release()
{
	glUseProgram(0);
}

void Shader_WorkOrder::initializeOrder()
{
	unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
	bool found_vertex = FetchFileFromDisk(m_asset->m_vertexText, m_filename + EXT_SHADER_VERTEX);
	bool found_fragement = FetchFileFromDisk(m_asset->m_fragmentText, m_filename + EXT_SHADER_FRAGMENT);
	bool found_geometry = FetchFileFromDisk(m_asset->m_geometryText, m_filename + EXT_SHADER_GEOMETRY);
	write_guard.unlock();
	write_guard.release();
	
	if (!found_vertex)
		MSG_Manager::Error(MSG_Manager::FILE_MISSING, m_asset->getFileName() + EXT_SHADER_VERTEX);
	if (!found_fragement)
		MSG_Manager::Error(MSG_Manager::FILE_MISSING, m_asset->getFileName() + EXT_SHADER_FRAGMENT);
	if (!(found_vertex + found_fragement + found_geometry)) {
		// handle this?
	}

	// Parse
	Parse();
}

#include "Assets\Asset_Shader_Pkg.h"
void Shader_WorkOrder::Parse()
{
	string *text[3] = { &m_asset->m_vertexText, &m_asset->m_fragmentText, &m_asset->m_geometryText };
	for (int x = 0; x < 3; ++x) {		
		if (*text[x] == "") continue;
		string input = *text[x];
		// Find Package to include
		int spot = input.find("#package");
		while (spot != string::npos) {
			string directory = input.substr(spot);

			unsigned int qspot1 = directory.find("\"");
			unsigned int qspot2 = directory.find("\"", qspot1+1);
			// find string quotes and remove them
			directory = directory.substr(qspot1+1, qspot2-1 - qspot1);

			Shared_Asset_Shader_Pkg package;
			Asset_Shader_Pkg::Create(package, directory, false);
			string left = input.substr(0, spot);
			string right = input.substr(spot+1 + qspot2);
			input = left + package->getPackageText() + right;
			spot = input.find("#package");
		}
		unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
		*text[x] = input;
	}
}

void Shader_WorkOrder::finalizeOrder()
{
	if (!m_asset->existsYet()) {
		unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
		Compile();
		GenerateProgram();
		LinkProgram();
		glFlush();

		write_guard.unlock();
		write_guard.release();
		m_asset->finalize();
	}
}

void Shader_WorkOrder::Compile_Single_Shader(GLuint & ID, const char * source, const GLenum & type)
{
	if (strlen(source) > 0) {
		ID = glCreateShader(type);
		glShaderSource(ID, 1, &source, NULL);
		glCompileShader(ID);

		GLint success;
		glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLchar InfoLog[1024];
			glGetShaderInfoLog(ID, sizeof(InfoLog), NULL, InfoLog);
			MSG_Manager::Error(MSG_Manager::SHADER_INCOMPLETE, m_filename, string(InfoLog, 1024));
		}
	}
}

void Shader_WorkOrder::Compile()
{
	Compile_Single_Shader(m_asset->m_glVertexID, m_asset->m_vertexText.c_str(), GL_VERTEX_SHADER);
	Compile_Single_Shader(m_asset->m_glFragmentID, m_asset->m_fragmentText.c_str(), GL_FRAGMENT_SHADER);
	Compile_Single_Shader(m_asset->m_glGeometryID, m_asset->m_geometryText.c_str(), GL_GEOMETRY_SHADER);
}

void Shader_WorkOrder::GenerateProgram()
{
	m_asset->m_glProgramID = glCreateProgram();

	if (m_asset->m_glVertexID != 0)
		glAttachShader(m_asset->m_glProgramID, m_asset->m_glVertexID);
	if (m_asset->m_glFragmentID != 0)
		glAttachShader(m_asset->m_glProgramID, m_asset->m_glFragmentID);
	if (m_asset->m_glGeometryID != 0)
		glAttachShader(m_asset->m_glProgramID, m_asset->m_glGeometryID);
}

void Shader_WorkOrder::LinkProgram()
{
	// Link and validate, retrieve any errors
	glLinkProgram(m_asset->m_glProgramID);
	GLint success;
	glGetProgramiv(m_asset->m_glProgramID, GL_LINK_STATUS, &success);
	if (success == 0) {
		GLchar ErrorLog[1024];
		glGetProgramInfoLog(m_asset->m_glProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		MSG_Manager::Error(MSG_Manager::PROGRAM_INCOMPLETE, m_filename, string(ErrorLog, 1024));
	}
	glValidateProgram(m_asset->m_glProgramID);

	// Delete shader objects, they are already compiled and attached
	if (m_asset->m_glVertexID != 0)
		glDeleteShader(m_asset->m_glVertexID);
	if (m_asset->m_glFragmentID != 0)
		glDeleteShader(m_asset->m_glFragmentID);
	if (m_asset->m_glGeometryID != 0)
		glDeleteShader(m_asset->m_glGeometryID);
}

bool Shader_WorkOrder::FetchFileFromDisk(string & returnFile, const string & fileDirectory)
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