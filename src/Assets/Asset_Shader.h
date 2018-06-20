#pragma once
#ifndef	ASSET_SHADER_H
#define	ASSET_SHADER_H
#define EXT_SHADER_VERTEX ".vsh"
#define EXT_SHADER_FRAGMENT ".fsh"
#define EXT_SHADER_GEOMETRY ".gsh"
#define DIRECTORY_SHADER File_Reader::GetCurrentDir() + "\\Shaders\\"

#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include "Utilities\File_Reader.h"
#include "glm\glm.hpp"
#include "GL\glew.h"
#include "GLM\gtc\type_ptr.hpp"
#include <string>

using namespace glm;
using namespace std;
class Asset_Shader;
typedef shared_ptr<Asset_Shader> Shared_Asset_Shader;


/**
 * An encapsulation of an OpenGL shader program.\n
 * Supports vertex, fragment, and geometry shaders.\n
 * Also provides support for explicitly setting uniform values for a given attribute location.
 **/
class Asset_Shader : public Asset
{
public:	
	// (de)Constructors
	/** Destroy the Shader. */
	~Asset_Shader();
	/** Construct the Shader. */
	Asset_Shader(const string & filename);


	// Public Methods
	/** Make this shader program active */
	void bind();
	/** Inactivate any currently bound shader program. */
	static void Release();	


	/**********************************************************************
	----Convenient DSA functions for changing program uniform variables----
	***********************************************************************/

	void Set_Uniform(const GLuint & i, const bool & o) { glProgramUniform1i(m_glProgramID, i, o); }
	void Set_Uniform(const GLuint & i, const int & o) { glProgramUniform1i(m_glProgramID, i, o); }
	void Set_Uniform(const GLuint & i, const double & o) { glProgramUniform1d(m_glProgramID, i,o ); }
	void Set_Uniform(const GLuint & i, const float & o) { glProgramUniform1f(m_glProgramID, i, o); }
	void Set_Uniform(const GLuint & i, const vec2 & o) { glProgramUniform2f(m_glProgramID, i, o.x, o.y); }
	void Set_Uniform(const GLuint & i, const vec3 & o) { glProgramUniform3f(m_glProgramID, i, o.x, o.y, o.z); }
	void Set_Uniform(const GLuint & i, const vec4 & o) { glProgramUniform4f(m_glProgramID, i, o.x, o.y, o.z, o.w); }
	void Set_Uniform(const GLuint & i, const ivec2 & o) { glProgramUniform2i(m_glProgramID, i, o.x, o.y); }
	void Set_Uniform(const GLuint & i, const ivec3 & o) { glProgramUniform3i(m_glProgramID, i, o.x, o.y, o.z); }
	void Set_Uniform(const GLuint & i, const ivec4 & o) { glProgramUniform4i(m_glProgramID, i, o.x, o.y, o.z, o.w); }
	void Set_Uniform(const GLuint & i, const mat3 & o) { glProgramUniformMatrix3fv(m_glProgramID, i, 1, GL_FALSE, &o[0][0]); }
	void Set_Uniform(const GLuint & i, const mat4 & o) { glProgramUniformMatrix4fv(m_glProgramID, i, 1, GL_FALSE, &o[0][0]); }
	void Set_Uniform(const GLuint & i, const int * o) { glProgramUniform1iv(m_glProgramID, i, 1, o); }
	void Set_Uniform(const GLuint & i, const double * o) { glProgramUniform1dv(m_glProgramID, i, 1, o); }
	void Set_Uniform(const GLuint & i, const float * o) { glProgramUniform1fv(m_glProgramID, i, 1, o); }
	void Set_Uniform(const GLuint & i, const vec2 * o) { glProgramUniform2fv(m_glProgramID, i, 1, glm::value_ptr(*o)); }
	void Set_Uniform(const GLuint & i, const vec3 * o) { glProgramUniform3fv(m_glProgramID, i, 1, glm::value_ptr(*o)); }
	void Set_Uniform(const GLuint & i, const vec4 * o) { glProgramUniform4fv(m_glProgramID, i, 1, glm::value_ptr(*o)); }
	void Set_Uniform(const GLuint & i, const mat3 * o) { glProgramUniformMatrix3fv(m_glProgramID, i, 1, GL_FALSE, glm::value_ptr(*o)); }
	void Set_Uniform(const GLuint & i, const mat4 * o) { glProgramUniformMatrix4fv(m_glProgramID, i, 1, GL_FALSE, glm::value_ptr(*o)); }
	void Set_Uniform_Array(const GLuint & i, const int & o, const int & size) { glProgramUniform1iv(m_glProgramID, i, size, &o); }
	void Set_Uniform_Array(const GLuint & i, const double & o, const int & size) { glProgramUniform1dv(m_glProgramID, i, size, &o); }
	void Set_Uniform_Array(const GLuint & i, const float & o, const int & size) { glProgramUniform1fv(m_glProgramID, i, size, &o); }
	void Set_Uniform_Array(const GLuint & i, const vec2 & o, const int & size) { glProgramUniform2fv(m_glProgramID, i, size, glm::value_ptr(o)); }
	void Set_Uniform_Array(const GLuint & i, const vec3 & o, const int & size) { glProgramUniform3fv(m_glProgramID, i, size, glm::value_ptr(o)); }
	void Set_Uniform_Array(const GLuint & i, const vec4 & o, const int & size) { glProgramUniform4fv(m_glProgramID, i, size, glm::value_ptr(o)); }
	void Set_Uniform_Array(const GLuint & i, const mat4 & o, const int & size) { glProgramUniformMatrix4fv(m_glProgramID, i, size, GL_FALSE, glm::value_ptr(o)); }
	void Set_Uniform_Array(const GLuint & i, const int * o, const int & size) { glProgramUniform1iv(m_glProgramID, i, size, o); }
	void Set_Uniform_Array(const GLuint & i, const double * o, const int & size) { glProgramUniform1dv(m_glProgramID, i, size, o); }
	void Set_Uniform_Array(const GLuint & i, const float * o, const int & size) { glProgramUniform1fv(m_glProgramID, i, size, o); }
	void Set_Uniform_Array(const GLuint & i, const vec2 * o, const int & size) { glProgramUniform2fv(m_glProgramID, i, size, glm::value_ptr(*o)); }
	void Set_Uniform_Array(const GLuint & i, const vec3 * o, const int & size) { glProgramUniform3fv(m_glProgramID, i, size, glm::value_ptr(*o)); }
	void Set_Uniform_Array(const GLuint & i, const vec4 * o, const int & size) { glProgramUniform4fv(m_glProgramID, i, size, glm::value_ptr(*o)); }
	void Set_Uniform_Array(const GLuint & i, const mat4 * o, const int & size) { glProgramUniformMatrix4fv(m_glProgramID, i, size, GL_FALSE, glm::value_ptr(*o)); }
	void Set_Uniform_Mat_Array(const GLuint & i, const float * o, const int & size, const GLboolean & transpose) { glProgramUniformMatrix4fv(m_glProgramID, i, size, transpose, o); }
	

	/***********************************************************************************************************
 	----Convenient static functions for setting uniform values at a given location, while a shader is bound----
	************************************************************************************************************/

	static void Change_Uniform(const GLuint & i, const bool & o) { glUniform1i(i, o); }
	static void Change_Uniform(const GLuint & i, const int & o) { glUniform1i(i, o); }
	static void Change_Uniform(const GLuint & i, const double & o) { glUniform1d(i, o); }
	static void Change_Uniform(const GLuint & i, const float & o) { glUniform1f(i, o); }
	static void Change_Uniform(const GLuint & i, const vec2 & o) { glUniform2f(i, o.x, o.y); }
	static void Change_Uniform(const GLuint & i, const vec3 & o) { glUniform3f(i, o.x, o.y, o.z); }
	static void Change_Uniform(const GLuint & i, const vec4 & o) { glUniform4f(i, o.x, o.y, o.z, o.w); }
	static void Change_Uniform(const GLuint & i, const ivec2 & o) { glUniform2i(i, o.x, o.y); }
	static void Change_Uniform(const GLuint & i, const ivec3 & o) { glUniform3i(i, o.x, o.y, o.z); }
	static void Change_Uniform(const GLuint & i, const ivec4 & o) { glUniform4i(i, o.x, o.y, o.z, o.w); }
	static void Change_Uniform(const GLuint & i, const mat3 & o) { glUniformMatrix3fv(i, 1, GL_FALSE, &o[0][0]); }
	static void Change_Uniform(const GLuint & i, const mat4 & o) { glUniformMatrix4fv(i, 1, GL_FALSE, &o[0][0]); }
	static void Change_Uniform(const GLuint & i, const int * o) { glUniform1iv(i, 1, o); }
	static void Change_Uniform(const GLuint & i, const double * o) { glUniform1dv(i, 1, o); }
	static void Change_Uniform(const GLuint & i, const float * o) { glUniform1fv(i, 1, o); }
	static void Change_Uniform(const GLuint & i, const vec2 * o) { glUniform2fv(i, 1, glm::value_ptr(*o)); }
	static void Change_Uniform(const GLuint & i, const vec3 * o) { glUniform3fv(i, 1, glm::value_ptr(*o)); }
	static void Change_Uniform(const GLuint & i, const vec4 * o) { glUniform4fv(i, 1, glm::value_ptr(*o)); }
	static void Change_Uniform(const GLuint & i, const mat3 * o) { glUniformMatrix3fv(i, 1, GL_FALSE, glm::value_ptr(*o)); }
	static void Change_Uniform(const GLuint & i, const mat4 * o) { glUniformMatrix4fv(i, 1, GL_FALSE, glm::value_ptr(*o)); }
	static void Change_Uniform_Array(const GLuint & i, const int & o, const int & size) { glUniform1iv(i, size, &o); }
	static void Change_Uniform_Array(const GLuint & i, const double & o, const int & size) { glUniform1dv(i, size, &o); }
	static void Change_Uniform_Array(const GLuint & i, const float & o, const int & size) { glUniform1fv(i, size, &o); }
	static void Change_Uniform_Array(const GLuint & i, const vec2 & o, const int & size) { glUniform2fv(i, size, glm::value_ptr(o)); }
	static void Change_Uniform_Array(const GLuint & i, const vec3 & o, const int & size) { glUniform3fv(i, size, glm::value_ptr(o)); }
	static void Change_Uniform_Array(const GLuint & i, const vec4 & o, const int & size) { glUniform4fv(i, size, glm::value_ptr(o)); }
	static void Change_Uniform_Array(const GLuint & i, const mat4 & o, const int & size) { glUniformMatrix4fv(i, size, GL_FALSE, glm::value_ptr(o)); }
	static void Change_Uniform_Array(const GLuint & i, const int * o, const int & size) { glUniform1iv(i, size, o); }
	static void Change_Uniform_Array(const GLuint & i, const double * o, const int & size) { glUniform1dv(i, size, o); }
	static void Change_Uniform_Array(const GLuint & i, const float * o, const int & size) { glUniform1fv(i, size, o); }
	static void Change_Uniform_Array(const GLuint & i, const vec2 * o, const int & size) { glUniform2fv(i, size, glm::value_ptr(*o)); }
	static void Change_Uniform_Array(const GLuint & i, const vec3 * o, const int & size) { glUniform3fv(i, size, glm::value_ptr(*o)); }
	static void Change_Uniform_Array(const GLuint & i, const vec4 * o, const int & size) { glUniform4fv(i, size, glm::value_ptr(*o)); }
	static void Change_Uniform_Array(const GLuint & i, const mat4 * o, const int & size) { glUniformMatrix4fv(i, size, GL_FALSE, glm::value_ptr(*o)); }
	static void Change_Uniform_Mat_Array(const GLuint & i, const float * o, const int & size, const GLboolean & transpose) { glUniformMatrix4fv(i, size, transpose, o); }

	
	// Public Attributes
	GLuint m_glProgramID, m_glVertexID, m_glFragmentID, m_glGeometryID; // OpenGL ID's
	string m_vertexText, m_fragmentText, m_geometryText; // Text Data
};


/**
 * Namespace that provides functionality for loading assets.
 **/
namespace Asset_Loader {
	/** Attempts to create an asset from disk or share one if it already exists */
	 void load_asset(Shared_Asset_Shader & user, const string & filename, const bool & threaded = true);
};

/**
 * Implements a work order for Shader Assets.
 **/
class Shader_WorkOrder : public Work_Order {
public:
	/** Constructs an Asset_Shader work order */
	Shader_WorkOrder(Shared_Asset_Shader & asset, const std::string & filename) : m_asset(asset), m_filename(filename) {};
	~Shader_WorkOrder() {};
	virtual void initializeOrder();
	virtual void finalizeOrder();


private:
	// Private Methods
	/** Parse for shader package inclusions. */
	void Parse();
	/** Creates and compiles all available shader types for this program. */
	void Compile();
	/* Creates and compiles a single shader file.
	 * @param	ID	the ID of the shader to compile
	 * @param	source	the text source to use
	 * @param	type	the shader type for this to become
	 * @note	self reports errors should they occur.	*/
	void Compile_Single_Shader(GLuint & ID, const char * source, const GLenum & type);
	/** Generates an OpenGL shader program ID for this class, and attempts to attach any available shaders. */
	void GenerateProgram();
	/** Attempts to link and validate the shader program. */
	void LinkProgram();
	/** Reads in a text file from disk.
	 * @param	returnFile	reference string to return the text file to
	 * @param	fileDirectory	absolute path to the file to read from
	 * @return	true if the file exists, false otherwise */
	bool FetchFileFromDisk(string & returnFile, const string & fileDirectory);
	
	
	// Private Attributes
	string m_filename;
	Shared_Asset_Shader m_asset;
};

#endif // ASSET_SHADER_H
