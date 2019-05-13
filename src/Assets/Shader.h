#pragma once
#ifndef	SHADER_H
#define	SHADER_H

#include "Assets/Asset.h"
#include "glm/glm.hpp"
#include "GL/glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include <string>


class Engine;
class Shader;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Shader : public std::shared_ptr<Shader> {
public:
	Shared_Shader() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Shader(Engine * engine, const std::string & filename, const bool & threaded = true);
};

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
class Shader : public Asset
{
public:	
	/** Destroy the Shader. */
	~Shader();
	/** Construct the Shader. */
	Shader(Engine * engine, const std::string & filename);


public:
	// Public Methods
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
	inline void setUniformArray(const GLuint & i, const int & o, const int & count) { glProgramUniform1iv(m_glProgramID, i, count, &o); }
	inline void setUniformArray(const GLuint & i, const unsigned int & o, const int & count) { glProgramUniform1uiv(m_glProgramID, i, count, &o); }
	inline void setUniformArray(const GLuint & i, const double & o, const int & count) { glProgramUniform1dv(m_glProgramID, i, count, &o); }
	inline void setUniformArray(const GLuint & i, const float & o, const int & count) { glProgramUniform1fv(m_glProgramID, i, count, &o); }
	inline void setUniformArray(const GLuint & i, const GLuint64 & o, const int & count) { glProgramUniformHandleui64vARB(m_glProgramID, i, count, &o); }
	inline void setUniformArray(const GLuint & i, const glm::vec2 & o, const int & count) { glProgramUniform2fv(m_glProgramID, i, count, glm::value_ptr(o)); }
	inline void setUniformArray(const GLuint & i, const glm::vec3 & o, const int & count) { glProgramUniform3fv(m_glProgramID, i, count, glm::value_ptr(o)); }
	inline void setUniformArray(const GLuint & i, const glm::vec4 & o, const int & count) { glProgramUniform4fv(m_glProgramID, i, count, glm::value_ptr(o)); }
	inline void setUniformArray(const GLuint & i, const glm::mat4 & o, const int & count) { glProgramUniformMatrix4fv(m_glProgramID, i, count, GL_FALSE, glm::value_ptr(o)); }
	inline void setUniformArray(const GLuint & i, const int * o, const int & count) { glProgramUniform1iv(m_glProgramID, i, count, o); }
	inline void setUniformArray(const GLuint & i, const unsigned int * o, const int & count) { glProgramUniform1uiv(m_glProgramID, i, count, o); }
	inline void setUniformArray(const GLuint & i, const double * o, const int & count) { glProgramUniform1dv(m_glProgramID, i, count, o); }
	inline void setUniformArray(const GLuint & i, const float * o, const int & count) { glProgramUniform1fv(m_glProgramID, i, count, o); }
	inline void setUniformArray(const GLuint & i, const GLuint64 * o, const int & count) { glProgramUniformHandleui64vARB(m_glProgramID, i, count, o); }
	inline void setUniformArray(const GLuint & i, const glm::vec2 * o, const int & count) { glProgramUniform2fv(m_glProgramID, i, count, glm::value_ptr(*o)); }
	inline void setUniformArray(const GLuint & i, const glm::vec3 * o, const int & count) { glProgramUniform3fv(m_glProgramID, i, count, glm::value_ptr(*o)); }
	inline void setUniformArray(const GLuint & i, const glm::vec4 * o, const int & count) { glProgramUniform4fv(m_glProgramID, i, count, glm::value_ptr(*o)); }
	inline void setUniformArray(const GLuint & i, const glm::mat4 * o, const int & count) { glProgramUniformMatrix4fv(m_glProgramID, i, count, GL_FALSE, glm::value_ptr(*o)); }
	inline void setUniformMatrixArray(const GLuint & i, const float * o, const int & count, const GLboolean & transpose) { glProgramUniformMatrix4fv(m_glProgramID, i, count, transpose, o); }
	

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
	@param		relativePath	the relative path of the binary file
	@return						true on success, false otherwise. */
	const bool loadCachedBinary(const std::string & relativePath);
	/** Attempt to save a shader program to a cached binary file.
	@param		relativePath	the relative path of the binary file
	@return						true on success, false otherwise. */
	const bool saveCachedBinary(const std::string & relativePath);
	/** Attempt to load a shader program from separate shader files.
	@param		relativePath	the relative path of the shader files
	@return						true on success, false otherwise. */
	virtual const bool initShaders(const std::string & relativePath);
	/** Use to validate this shader program after linking.
	@return						true on success, false otherwise. */
	const bool validateProgram();


private:
	// Interface Implementation
	void initializeDefault();
	virtual void initialize() override;


	// Private Attributes
	friend class Shared_Shader;
};

#endif // SHADER_H
