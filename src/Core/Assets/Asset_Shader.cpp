#include "Assets\Asset_Shader.h"
#include "Systems\Message_Manager.h"
#include "GLM\gtc\type_ptr.hpp"
#include <fstream>

/* -----ASSET TYPE----- */
#define ASSET_TYPE 6

using namespace Asset_Loader;

Asset_Shader::~Asset_Shader()
{
	if (ExistsYet()) 
		glDeleteProgram(gl_program_ID);
	if (m_fence != nullptr)
		glDeleteSync(m_fence);
}

Asset_Shader::Asset_Shader(const string & filename) : Asset(filename)
{
	gl_program_ID = 0;
	gl_shader_vertex_ID = 0;
	gl_shader_fragment_ID = 0;
	gl_shader_geometry_ID = 0;
	vertex_text = "";
	fragment_text = "";
	geometry_text = "";
	m_fence = nullptr;
}

int Asset_Shader::GetAssetType() 
{ 
	return ASSET_TYPE;
}

bool Asset_Shader::ExistsYet()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	if (Asset::ExistsYet() && m_fence != nullptr) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_mutex);
		const auto state = glClientWaitSync(m_fence, 0, 0);
		if (((state == GL_ALREADY_SIGNALED) || (state == GL_CONDITION_SATISFIED))
			&& (state != GL_WAIT_FAILED))
			return true;
	}
	return false;
}

void Asset_Shader::Bind()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	glUseProgram(gl_program_ID);
}

void Asset_Shader::Release()
{
	glUseProgram(0);
}

void Asset_Shader::setLocationValue(const GLuint & i, const bool & b) { glUniform1i(i, b); }

void Asset_Shader::setLocationValue(const GLuint & i, const int & o) { glUniform1i(i, o); }

void Asset_Shader::setLocationValue(const GLuint & i, const double & o) { glUniform1d(i, o); }

void Asset_Shader::setLocationValue(const GLuint & i, const float & o) { glUniform1f(i, o); }

void Asset_Shader::setLocationValue(const GLuint & i, const vec2 & o) { glUniform2f(i, o.x, o.y); }

void Asset_Shader::setLocationValue(const GLuint & i, const vec3 & o) { glUniform3f(i, o.x, o.y, o.z); }

void Asset_Shader::setLocationValue(const GLuint & i, const vec4 & o) { glUniform4f(i, o.x, o.y, o.z, o.w); }

void Asset_Shader::setLocationValue(const GLuint & i, const ivec2 & o) { glUniform2i(i, o.x, o.y); }

void Asset_Shader::setLocationValue(const GLuint & i, const ivec3 & o) { glUniform3i(i, o.x, o.y, o.z); }

void Asset_Shader::setLocationValue(const GLuint & i, const ivec4 & o) { glUniform4i(i, o.x, o.y, o.z, o.w); }

void Asset_Shader::setLocationValue(const GLuint & i, const mat3 & o) { glUniformMatrix3fv(i, 1, GL_FALSE, &o[0][0]); }

void Asset_Shader::setLocationValue(const GLuint & i, const mat4 & o) { glUniformMatrix4fv(i, 1, GL_FALSE, &o[0][0]); }

void Asset_Shader::setLocationValue(const GLuint & i, const int * o) { glUniform1iv(i, 1, o); }

void Asset_Shader::setLocationValue(const GLuint & i, const double * o) { glUniform1dv(i, 1, o); }

void Asset_Shader::setLocationValue(const GLuint & i, const float * o) { glUniform1fv(i, 1, o); }

void Asset_Shader::setLocationValue(const GLuint & i, const vec2 * o) { glUniform2fv(i, 1, glm::value_ptr(*o)); }

void Asset_Shader::setLocationValue(const GLuint & i, const vec3 * o) { glUniform3fv(i, 1, glm::value_ptr(*o)); }

void Asset_Shader::setLocationValue(const GLuint & i, const vec4 * o) { glUniform4fv(i, 1, glm::value_ptr(*o)); }

void Asset_Shader::setLocationValue(const GLuint & i, const mat3 * o) { glUniformMatrix3fv(i, 1, GL_FALSE, glm::value_ptr(*o)); }

void Asset_Shader::setLocationValue(const GLuint & i, const mat4 * o) { glUniformMatrix4fv(i, 1, GL_FALSE, glm::value_ptr(*o)); }

void Asset_Shader::setLocationValueArray(const GLuint & i, const int & o, const int & size) { glUniform1iv(i, size, &o); }

void Asset_Shader::setLocationValueArray(const GLuint & i, const double & o, const int & size) { glUniform1dv(i, size, &o); }

void Asset_Shader::setLocationValueArray(const GLuint & i, const float & o, const int & size) { glUniform1fv(i, size, &o); }

void Asset_Shader::setLocationValueArray(const GLuint & i, const vec2 & o, const int & size) { glUniform2fv(i, size, glm::value_ptr(o)); }

void Asset_Shader::setLocationValueArray(const GLuint & i, const vec3 & o, const int & size) { glUniform3fv(i, size, glm::value_ptr(o)); }

void Asset_Shader::setLocationValueArray(const GLuint & i, const vec4 & o, const int & size) { glUniform4fv(i, size, glm::value_ptr(o)); }

void Asset_Shader::setLocationValueArray(const GLuint & i, const mat4 & o, const int & size) { glUniformMatrix4fv(i, size, GL_FALSE, glm::value_ptr(o)); }

void Asset_Shader::setLocationValueArray(const GLuint & i, const int * o, const int & size) { glUniform1iv(i, size, o); }

void Asset_Shader::setLocationValueArray(const GLuint & i, const double * o, const int & size) { glUniform1dv(i, size, o); }

void Asset_Shader::setLocationValueArray(const GLuint & i, const float * o, const int & size) { glUniform1fv(i, size, o); }

void Asset_Shader::setLocationValueArray(const GLuint & i, const vec2 * o, const int & size) { glUniform2fv(i, size, glm::value_ptr(*o)); }

void Asset_Shader::setLocationValueArray(const GLuint & i, const vec3 * o, const int & size) { glUniform3fv(i, size, glm::value_ptr(*o)); }

void Asset_Shader::setLocationValueArray(const GLuint & i, const vec4 * o, const int & size) { glUniform4fv(i, size, glm::value_ptr(*o)); }

void Asset_Shader::setLocationValueArray(const GLuint & i, const mat4 * o, const int & size) { glUniformMatrix4fv(i, size, GL_FALSE, glm::value_ptr(*o)); }

void Asset_Shader::setLocationMatArray(const GLuint & i, const float * o, const int & size, const GLboolean & transpose) { glUniformMatrix4fv(i, size, transpose, o); }

// Reads in a text file from disk, given a file directory, and appends it to the returnFile param
// Returns true if succeeded, false if file doesn't exist
bool FetchFileFromDisk(string & returnFile, const string & fileDirectory)
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

// Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
// Will resort to building one of its own if it can't find one from disk
void fetchDefaultAsset(Shared_Asset_Shader & asset)
{	
	// Check if a copy already exists
	if (Asset_Manager::RetrieveDefaultAsset<Asset_Shader>(asset, "defaultShader"))
		return;

	// Check if the file/directory exists on disk
	const string fullDirectory = DIRECTORY_SHADER + "defaultShader";
	Shader_WorkOrder work_order(asset, fullDirectory);
	bool found_vertex = FileReader::FileExistsOnDisk(fullDirectory + EXT_SHADER_VERTEX);
	bool found_fragement = FileReader::FileExistsOnDisk(fullDirectory + EXT_SHADER_FRAGMENT);
	if (!found_vertex)
		MSG::Error(FILE_MISSING, fullDirectory + EXT_SHADER_VERTEX);
	if (!found_fragement)
		MSG::Error(FILE_MISSING, fullDirectory + EXT_SHADER_FRAGMENT);
	if ((found_vertex && found_fragement)) { 
		work_order.Initialize_Order();
		work_order.Finalize_Order();
		if (asset->ExistsYet())
			return;
	}

	// We couldn't load the default file, generate a temporary one
	MSG::Error(FILE_MISSING, fullDirectory);
	/* HARD CODE DEFAULT VALUES HERE */
	asset->vertex_text = "#version 430\n\nlayout(location = 0) in vec3 vertex;\n\nvoid main()\n{\n\tgl_Position = vec4(vertex, 1.0);\n}";
	asset->fragment_text = "#version 430\n\nlayout (location = 0) out vec4 fragColor;\n\nvoid main()\n{\n\tfragColor = vec4(1.0f);\n}";
	work_order.Finalize_Order();
}

namespace Asset_Loader {
	void load_asset(Shared_Asset_Shader & user, const string & filename, const bool & threaded)
	{
		// Check if a copy already exists
		if (Asset_Manager::QueryExistingAsset<Asset_Shader>(user, filename))
			return;

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = DIRECTORY_SHADER + filename;
		bool found_vertex = FileReader::FileExistsOnDisk(fullDirectory + EXT_SHADER_VERTEX);
		bool found_fragement = FileReader::FileExistsOnDisk(fullDirectory + EXT_SHADER_FRAGMENT);
		if (!found_vertex)
			MSG::Error(FILE_MISSING, fullDirectory + EXT_SHADER_VERTEX);
		if (!found_fragement)
			MSG::Error(FILE_MISSING, fullDirectory + EXT_SHADER_FRAGMENT);
		if ( !(found_vertex && found_fragement) ) {
			fetchDefaultAsset(user);
			return;
		}

		// Create the asset
		Asset_Manager::CreateNewAsset<Asset_Shader, Shader_WorkOrder>(user, threaded, fullDirectory, filename);
	}
}

void Shader_WorkOrder::Initialize_Order()
{
	unique_lock<shared_mutex> write_guard(m_asset->m_mutex);

	bool found_vertex = FetchFileFromDisk(m_asset->vertex_text, m_filename + EXT_SHADER_VERTEX);
	bool found_fragement = FetchFileFromDisk(m_asset->fragment_text, m_filename + EXT_SHADER_FRAGMENT);
	bool found_geometry = FetchFileFromDisk(m_asset->geometry_text, m_filename + EXT_SHADER_GEOMETRY);

	if (!found_vertex)
		MSG::Error(FILE_MISSING, m_asset->GetFileName() + EXT_SHADER_VERTEX);
	if (!found_fragement)
		MSG::Error(FILE_MISSING, m_asset->GetFileName() + EXT_SHADER_FRAGMENT);
	if (!(found_vertex + found_fragement + found_geometry)) {
		// handle this?
	}
}

void Shader_WorkOrder::Finalize_Order()
{
	if (!m_asset->ExistsYet()) {
		unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
		Compile();
		GenerateProgram();
		LinkProgram();
		m_asset->m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glFlush();

		m_asset->Finalize();
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
			MSG::Error(SHADER_INCOMPLETE, m_filename, string(InfoLog, 1024));
		}
	}
}

void Shader_WorkOrder::Compile()
{
	Compile_Single_Shader(m_asset->gl_shader_vertex_ID, m_asset->vertex_text.c_str(), GL_VERTEX_SHADER);
	Compile_Single_Shader(m_asset->gl_shader_fragment_ID, m_asset->fragment_text.c_str(), GL_FRAGMENT_SHADER);
	Compile_Single_Shader(m_asset->gl_shader_geometry_ID, m_asset->geometry_text.c_str(), GL_GEOMETRY_SHADER);
}

void Shader_WorkOrder::GenerateProgram()
{
	m_asset->gl_program_ID = glCreateProgram();

	if (m_asset->gl_shader_vertex_ID != 0)
		glAttachShader(m_asset->gl_program_ID, m_asset->gl_shader_vertex_ID);
	if (m_asset->gl_shader_fragment_ID != 0)
		glAttachShader(m_asset->gl_program_ID, m_asset->gl_shader_fragment_ID);
	if (m_asset->gl_shader_geometry_ID != 0)
		glAttachShader(m_asset->gl_program_ID, m_asset->gl_shader_geometry_ID);
}

void Shader_WorkOrder::LinkProgram()
{
	// Link and validate, retrieve any errors
	glLinkProgram(m_asset->gl_program_ID);
	GLint success;
	glGetProgramiv(m_asset->gl_program_ID, GL_LINK_STATUS, &success);
	if (success == 0) {
		GLchar ErrorLog[1024];
		glGetProgramInfoLog(m_asset->gl_program_ID, sizeof(ErrorLog), NULL, ErrorLog);
		MSG::Error(PROGRAM_INCOMPLETE, m_filename, string(ErrorLog, 1024));
	}
	glValidateProgram(m_asset->gl_program_ID);

	// Delete shader objects, they are already compiled and attached
	if (m_asset->gl_shader_vertex_ID != 0)
		glDeleteShader(m_asset->gl_shader_vertex_ID);
	if (m_asset->gl_shader_fragment_ID != 0)
		glDeleteShader(m_asset->gl_shader_fragment_ID);
	if (m_asset->gl_shader_geometry_ID != 0)
		glDeleteShader(m_asset->gl_shader_geometry_ID);
}