#pragma once
#ifndef	ASSET_SHADER_H
#define	ASSET_SHADER_H

#include "Assets\Asset.h"
#include "glm\glm.hpp"
#include "GL\glew.h"
#include "GLM\gtc\type_ptr.hpp"
#include <string>


class Engine;
class Asset_Shader;
using Shared_Asset_Shader = std::shared_ptr<Asset_Shader>;

struct ShaderObj 
{
	// (de)Constructors
	~ShaderObj();
	ShaderObj(const GLenum & type);


	// Functions
	/** Retrieve a shader parameter by the name specified.
	@param		pname			the program parameter name
	@return						the parameter value matching the name specified. */
	const GLint getShaderiv(const GLenum & pname) const;
	/** Load a shader document from the file path specified.
	@param		engine			the active engine to use
	@param		filePath		the relative path to the file to read
	@return						true on success, false otherwise. */
	const bool loadDocument(Engine * engine, const std::string & filePath);
	/** Create an OpenGL shader object from this class, using the document string loaded.
	@param		engine			the active engine to use
	@param		filename		the shader file name (for reporting purposes)
	@return						true on success, false otherwise. */
	const bool createGLShader(Engine * engine, const std::string & filename);

	
	// Attributes
	GLuint m_shaderID = 0;
	std::string m_shaderText = "";
	GLenum m_type = GL_VERTEX_SHADER;
};

/** An encapsulation of a vertex/fragment OpenGL shader program.
Also provides support for explicitly setting uniform values for a given attribute location. */
class Asset_Shader : public Asset
{
public:	
	/** Destroy the Shader. */
	~Asset_Shader();
	/** Construct the Shader. */
	Asset_Shader(const std::string & filename);


	// Public Methods
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	static Shared_Asset_Shader Create(Engine * engine, const std::string & filename, const bool & threaded = true);
	/** Make this shader program active */
	void bind();
	/** Inactivate any currently bound shader program. */
	static void Release();	


	/**********************************************************************
	----Convenient DSA functions for changing program uniform variables----
	***********************************************************************/

	inline void setUniform(const GLuint & i, const bool & o) { glProgramUniform1i(m_glProgramID, i, o); }
	inline void setUniform(const GLuint & i, const int & o) { glProgramUniform1i(m_glProgramID, i, o); }
	inline void setUniform(const GLuint & i, const unsigned int & o) { glProgramUniform1ui(m_glProgramID, i, o); }
	inline void setUniform(const GLuint & i, const double & o) { glProgramUniform1d(m_glProgramID, i,o ); }
	inline void setUniform(const GLuint & i, const float & o) { glProgramUniform1f(m_glProgramID, i, o); }
	inline void setUniform(const GLuint & i, const GLuint64 & o) { glProgramUniformHandleui64ARB(m_glProgramID, i, o); }
	inline void setUniform(const GLuint & i, const glm::vec2 & o) { glProgramUniform2f(m_glProgramID, i, o.x, o.y); }
	inline void setUniform(const GLuint & i, const glm::vec3 & o) { glProgramUniform3f(m_glProgramID, i, o.x, o.y, o.z); }
	inline void setUniform(const GLuint & i, const glm::vec4 & o) { glProgramUniform4f(m_glProgramID, i, o.x, o.y, o.z, o.w); }
	inline void setUniform(const GLuint & i, const glm::ivec2 & o) { glProgramUniform2i(m_glProgramID, i, o.x, o.y); }
	inline void setUniform(const GLuint & i, const glm::ivec3 & o) { glProgramUniform3i(m_glProgramID, i, o.x, o.y, o.z); }
	inline void setUniform(const GLuint & i, const glm::ivec4 & o) { glProgramUniform4i(m_glProgramID, i, o.x, o.y, o.z, o.w); }
	inline void setUniform(const GLuint & i, const glm::mat3 & o) { glProgramUniformMatrix3fv(m_glProgramID, i, 1, GL_FALSE, &o[0][0]); }
	inline void setUniform(const GLuint & i, const glm::mat4 & o) { glProgramUniformMatrix4fv(m_glProgramID, i, 1, GL_FALSE, &o[0][0]); }
	inline void setUniform(const GLuint & i, const int * o) { glProgramUniform1iv(m_glProgramID, i, 1, o); }
	inline void setUniform(const GLuint & i, const unsigned int * o) { glProgramUniform1uiv(m_glProgramID, i, 1, o); }
	inline void setUniform(const GLuint & i, const double * o) { glProgramUniform1dv(m_glProgramID, i, 1, o); }
	inline void setUniform(const GLuint & i, const float * o) { glProgramUniform1fv(m_glProgramID, i, 1, o); }
	inline void setUniform(const GLuint & i, const GLuint64 * o) { glProgramUniformHandleui64ARB(m_glProgramID, i, *o); }
	inline void setUniform(const GLuint & i, const glm::vec2 * o) { glProgramUniform2fv(m_glProgramID, i, 1, glm::value_ptr(*o)); }
	inline void setUniform(const GLuint & i, const glm::vec3 * o) { glProgramUniform3fv(m_glProgramID, i, 1, glm::value_ptr(*o)); }
	inline void setUniform(const GLuint & i, const glm::vec4 * o) { glProgramUniform4fv(m_glProgramID, i, 1, glm::value_ptr(*o)); }
	inline void setUniform(const GLuint & i, const glm::mat3 * o) { glProgramUniformMatrix3fv(m_glProgramID, i, 1, GL_FALSE, glm::value_ptr(*o)); }
	inline void setUniform(const GLuint & i, const glm::mat4 * o) { glProgramUniformMatrix4fv(m_glProgramID, i, 1, GL_FALSE, glm::value_ptr(*o)); }
	inline void setUniformArray(const GLuint & i, const int & o, const int & size) { glProgramUniform1iv(m_glProgramID, i, size, &o); }
	inline void setUniformArray(const GLuint & i, const unsigned int & o, const int & size) { glProgramUniform1uiv(m_glProgramID, i, size, &o); }
	inline void setUniformArray(const GLuint & i, const double & o, const int & size) { glProgramUniform1dv(m_glProgramID, i, size, &o); }
	inline void setUniformArray(const GLuint & i, const float & o, const int & size) { glProgramUniform1fv(m_glProgramID, i, size, &o); }
	inline void setUniformArray(const GLuint & i, const GLuint64 & o, const int & size) { glProgramUniformHandleui64vARB(m_glProgramID, i, size, &o); }
	inline void setUniformArray(const GLuint & i, const glm::vec2 & o, const int & size) { glProgramUniform2fv(m_glProgramID, i, size, glm::value_ptr(o)); }
	inline void setUniformArray(const GLuint & i, const glm::vec3 & o, const int & size) { glProgramUniform3fv(m_glProgramID, i, size, glm::value_ptr(o)); }
	inline void setUniformArray(const GLuint & i, const glm::vec4 & o, const int & size) { glProgramUniform4fv(m_glProgramID, i, size, glm::value_ptr(o)); }
	inline void setUniformArray(const GLuint & i, const glm::mat4 & o, const int & size) { glProgramUniformMatrix4fv(m_glProgramID, i, size, GL_FALSE, glm::value_ptr(o)); }
	inline void setUniformArray(const GLuint & i, const int * o, const int & size) { glProgramUniform1iv(m_glProgramID, i, size, o); }
	inline void setUniformArray(const GLuint & i, const unsigned int * o, const int & size) { glProgramUniform1uiv(m_glProgramID, i, size, o); }
	inline void setUniformArray(const GLuint & i, const double * o, const int & size) { glProgramUniform1dv(m_glProgramID, i, size, o); }
	inline void setUniformArray(const GLuint & i, const float * o, const int & size) { glProgramUniform1fv(m_glProgramID, i, size, o); }
	inline void setUniformArray(const GLuint & i, const GLuint64 * o, const int & size) { glProgramUniformHandleui64vARB(m_glProgramID, i, size, o); }
	inline void setUniformArray(const GLuint & i, const glm::vec2 * o, const int & size) { glProgramUniform2fv(m_glProgramID, i, size, glm::value_ptr(*o)); }
	inline void setUniformArray(const GLuint & i, const glm::vec3 * o, const int & size) { glProgramUniform3fv(m_glProgramID, i, size, glm::value_ptr(*o)); }
	inline void setUniformArray(const GLuint & i, const glm::vec4 * o, const int & size) { glProgramUniform4fv(m_glProgramID, i, size, glm::value_ptr(*o)); }
	inline void setUniformArray(const GLuint & i, const glm::mat4 * o, const int & size) { glProgramUniformMatrix4fv(m_glProgramID, i, size, GL_FALSE, glm::value_ptr(*o)); }
	inline void setUniformMatrixArray(const GLuint & i, const float * o, const int & size, const GLboolean & transpose) { glProgramUniformMatrix4fv(m_glProgramID, i, size, transpose, o); }
	

	// Public Attributes
	GLuint m_glProgramID = 0;
	ShaderObj m_vertexShader = ShaderObj(GL_VERTEX_SHADER);
	ShaderObj m_fragmentShader = ShaderObj(GL_FRAGMENT_SHADER);


protected:
	// Protected Methods
	/** Retrieve a program parameter by the name specified.
	@param		pname			the program parameter name
	@return						the parameter value matching the name specified. */
	const GLint getProgramiv(const GLenum & pname) const;
	/** Retrieve an error log corresponding to this shader program.
	@return						an error log for this shader program. */
	const std::vector<GLchar> getErrorLog() const;
	/** Attempt to load a shader program from a cached binary file.
	@param		engine			the active engine to use
	@param		relativePath	the relative path of the binary file
	@return						true on success, false otherwise. */
	const bool loadCachedBinary(Engine * engine, const std::string & relativePath);
	/** Attempt to save a shader program to a cached binary file.
	@param		engine			the active engine to use
	@param		relativePath	the relative path of the binary file
	@return						true on success, false otherwise. */
	const bool saveCachedBinary(Engine * engine, const std::string & relativePath);
	/** Attempt to load a shader program from separate shader files.
	@param		engine			the active engine to use
	@param		relativePath	the relative path of the shader files
	@return						true on success, false otherwise. */
	virtual const bool initShaders(Engine * engine, const std::string & relativePath);
	/** Use to validate this shader program after linking.
	@return						true on success, false otherwise. */
	const bool validateProgram();


private:
	// Interface Implementation
	void initializeDefault(Engine * engine);
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_SHADER_H
