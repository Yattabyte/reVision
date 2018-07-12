#pragma once
#ifndef	ASSET_SHADER_H
#define	ASSET_SHADER_H

#include "Assets\Asset.h"
#include "glm\glm.hpp"
#include "GL\glew.h"
#include "GLM\gtc\type_ptr.hpp"
#include <string>

using namespace glm;
using namespace std;
class Engine;
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
	/** Destroy the Shader. */
	~Asset_Shader();


	// Public Methods
	/** Creates a default asset.
	 * @param	engine			the engine being used
	 * @param	userAsset		the desired asset container */
	static void CreateDefault(Engine * engine, Shared_Asset_Shader & userAsset);
	/** Begins the creation process for this asset.
	 * @param	engine			the engine being used
	 * @param	userAsset		the desired asset container
	 * @param	filename		the filename to use
	 * @param	threaded		create in a separate thread */
	static void Create(Engine * engine, Shared_Asset_Shader & userAsset, const string & filename, const bool & threaded = true);
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
	GLuint m_glProgramID, m_glVertexID, m_glFragmentID; // OpenGL ID's
	string m_vertexText, m_fragmentText; // Text Data


protected:
	// Protected Constructors
	/** Construct the Shader. */
	Asset_Shader(const string & filename);


private:
	// Private Methods
	/** Initializes the asset. */
	static void Initialize(Engine * engine, Shared_Asset_Shader & userAsset, const string & fullDirectory);
	/** Finalizes the asset. */
	static void Finalize(Engine * engine, Shared_Asset_Shader & userAsset);


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_SHADER_H
