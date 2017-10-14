#include "Assets\Asset_Shader.h"
#include "Managers\Message_Manager.h"
#include "GLM\gtc\type_ptr.hpp"
#include <fstream>

/* -----ASSET TYPE----- */
#define ASSET_TYPE 2

using namespace Asset_Manager;

Asset_Shader::~Asset_Shader()
{
	Destroy();
}

Asset_Shader::Asset_Shader()
{
	gl_program_ID = 0;
	gl_shader_vertex_ID = 0;
	gl_shader_fragment_ID = 0;
	gl_shader_geometry_ID = 0;
	filename = "";
	vertex_text = "";
	fragment_text = "";
	geometry_text = "";
}

int Asset_Shader::GetAssetType() 
{ 
	return ASSET_TYPE;
}

void Asset_Shader::Finalize()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	if (!finalized) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_mutex);
		Compile();
		GenerateProgram();
		LinkProgram();
		finalized = true;
	}
}

void Asset_Shader::Destroy()
{
	if (finalized) {
		glDeleteProgram(gl_program_ID);
		finalized = false;
	}
}

void Asset_Shader::Compile()
{
	Compile_Single_Shader(gl_shader_vertex_ID, vertex_text.c_str(), GL_VERTEX_SHADER);
	Compile_Single_Shader(gl_shader_fragment_ID, fragment_text.c_str(), GL_FRAGMENT_SHADER);
	Compile_Single_Shader(gl_shader_geometry_ID, geometry_text.c_str(), GL_GEOMETRY_SHADER);
}

void Asset_Shader::Compile_Single_Shader(GLuint &ID, const char *source, const GLenum &type)
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
			MSG::Error(SHADER_INCOMPLETE, filename, string(InfoLog, 1024));
		}
	}
}

void Asset_Shader::GenerateProgram()
{
	gl_program_ID = glCreateProgram();

	if (gl_shader_vertex_ID != 0)
		glAttachShader(gl_program_ID, gl_shader_vertex_ID);
	if (gl_shader_fragment_ID != 0)
		glAttachShader(gl_program_ID, gl_shader_fragment_ID);
	if (gl_shader_geometry_ID != 0)
		glAttachShader(gl_program_ID, gl_shader_geometry_ID);
}

void Asset_Shader::LinkProgram()
{
	// Link and validate, retrieve any errors
	glLinkProgram(gl_program_ID);
	GLint success;
	glGetProgramiv(gl_program_ID, GL_LINK_STATUS, &success);
	if (success == 0) {
		GLchar ErrorLog[1024];
		glGetProgramInfoLog(gl_program_ID, sizeof(ErrorLog), NULL, ErrorLog);
		MSG::Error(PROGRAM_INCOMPLETE, filename, string(ErrorLog, 1024));
	}
	glValidateProgram(gl_program_ID);

	// Delete shader objects, they are already compiled and attached
	if (gl_shader_vertex_ID != 0)
		glDeleteShader(gl_shader_vertex_ID);
	if (gl_shader_fragment_ID != 0)
		glDeleteShader(gl_shader_fragment_ID);
	if (gl_shader_geometry_ID != 0)
		glDeleteShader(gl_shader_geometry_ID);
}

void Asset_Shader::Bind()
{
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
bool FetchFileFromDisk(string &returnFile, const string &fileDirectory)
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

// Start loading the individual shaders from disk.
// This is the extent to what we can do for this in a separate thread.
void initialize_Shader(Shared_Asset_Shader & user, const string & filename, bool * complete)
{
	unique_lock<shared_mutex> write_guard(user->m_mutex);

	bool found_vertex = FetchFileFromDisk(user->vertex_text, filename + ".vsh");
	bool found_fragement = FetchFileFromDisk(user->fragment_text, filename + ".fsh");
	bool found_geometry = FetchFileFromDisk(user->geometry_text, filename + ".gsh");

	if (!found_vertex)
		MSG::Error(FILE_MISSING, user->filename + ".vsh");
	if (!found_fragement)
		MSG::Error(FILE_MISSING, user->filename + ".fsh");
	else if (!(found_vertex + found_fragement + found_geometry))
		MSG::Error(FILE_MISSING, user->filename);

	submitWorkorder(user);
	*complete = true;
}

namespace Asset_Manager {
	void load_asset(Shared_Asset_Shader & user, const string &filename, const bool &regenerate, const bool & threaded)
	{
		// Check if a copy already exists
		shared_mutex &mutex_IO_assets = getMutexIOAssets();
		auto &assets_shaders = (fetchAssetList(Asset_Shader::GetAssetType()));
		{
			shared_lock<shared_mutex> guard(mutex_IO_assets);
			for each (auto &asset in assets_shaders) {
				shared_lock<shared_mutex> asset_guard(asset->m_mutex);
				const Shared_Asset_Shader derived_asset = dynamic_pointer_cast<Asset_Shader>(asset);
				if (derived_asset) { // Check that pointer isn't null after dynamic pointer casting
					if (derived_asset->filename == filename) { // Filenames match, use this asset
						asset_guard.unlock();
						asset_guard.release();
						user = derived_asset;
						if (regenerate) { 
							user->Destroy();
							if (!threaded)
								break;
						}
						// If we don't want multithreading, try to create the asset now.
						// It is OK if the first time this asset was requested was Multithreaded but this one isn't!
						// Because finalize can be called multiple times safely, as it checks to see if the content was already created.
						if (!threaded) {
							user->Finalize();
							return;
						}
					}
				}
			}
		}

		// Attempt to create the asset
		const std::string &fulldirectory = getCurrentDir() + "\\Shaders\\" + filename;
		{
			unique_lock<shared_mutex> guard(mutex_IO_assets);
			user = Shared_Asset_Shader(new Asset_Shader());
			user->filename = filename;
			assets_shaders.push_back(user);
		}

		// Either continue processing on a new thread or stay on the current one
		bool *complete = new bool(false);
		if (threaded) {
			thread *import_thread = new thread(initialize_Shader, user, fulldirectory, complete);
			import_thread->detach();
			submitWorkthread(std::pair<thread*, bool*>(import_thread, complete));
		}
		else {
			initialize_Shader(user, fulldirectory, complete);
			user->Finalize();
		}
	}
}