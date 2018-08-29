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

/** An encapsulation of a vertex/fragment OpenGL shader program.
Also provides support for explicitly setting uniform values for a given attribute location. */
class Asset_Shader : public Asset
{
public:	
	/** Destroy the Shader. */
	~Asset_Shader();


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

	void setUniform(const GLuint & i, const bool & o) { glProgramUniform1i(m_glProgramID, i, o); }
	void setUniform(const GLuint & i, const int & o) { glProgramUniform1i(m_glProgramID, i, o); }
	void setUniform(const GLuint & i, const double & o) { glProgramUniform1d(m_glProgramID, i,o ); }
	void setUniform(const GLuint & i, const float & o) { glProgramUniform1f(m_glProgramID, i, o); }
	void setUniform(const GLuint & i, const GLuint64 & o) { glProgramUniformHandleui64ARB(m_glProgramID, i, o); }
	void setUniform(const GLuint & i, const glm::vec2 & o) { glProgramUniform2f(m_glProgramID, i, o.x, o.y); }
	void setUniform(const GLuint & i, const glm::vec3 & o) { glProgramUniform3f(m_glProgramID, i, o.x, o.y, o.z); }
	void setUniform(const GLuint & i, const glm::vec4 & o) { glProgramUniform4f(m_glProgramID, i, o.x, o.y, o.z, o.w); }
	void setUniform(const GLuint & i, const glm::ivec2 & o) { glProgramUniform2i(m_glProgramID, i, o.x, o.y); }
	void setUniform(const GLuint & i, const glm::ivec3 & o) { glProgramUniform3i(m_glProgramID, i, o.x, o.y, o.z); }
	void setUniform(const GLuint & i, const glm::ivec4 & o) { glProgramUniform4i(m_glProgramID, i, o.x, o.y, o.z, o.w); }
	void setUniform(const GLuint & i, const glm::mat3 & o) { glProgramUniformMatrix3fv(m_glProgramID, i, 1, GL_FALSE, &o[0][0]); }
	void setUniform(const GLuint & i, const glm::mat4 & o) { glProgramUniformMatrix4fv(m_glProgramID, i, 1, GL_FALSE, &o[0][0]); }
	void setUniform(const GLuint & i, const int * o) { glProgramUniform1iv(m_glProgramID, i, 1, o); }
	void setUniform(const GLuint & i, const double * o) { glProgramUniform1dv(m_glProgramID, i, 1, o); }
	void setUniform(const GLuint & i, const float * o) { glProgramUniform1fv(m_glProgramID, i, 1, o); }
	void setUniform(const GLuint & i, const GLuint64 * o) { glProgramUniformHandleui64ARB(m_glProgramID, i, *o); }
	void setUniform(const GLuint & i, const glm::vec2 * o) { glProgramUniform2fv(m_glProgramID, i, 1, glm::value_ptr(*o)); }
	void setUniform(const GLuint & i, const glm::vec3 * o) { glProgramUniform3fv(m_glProgramID, i, 1, glm::value_ptr(*o)); }
	void setUniform(const GLuint & i, const glm::vec4 * o) { glProgramUniform4fv(m_glProgramID, i, 1, glm::value_ptr(*o)); }
	void setUniform(const GLuint & i, const glm::mat3 * o) { glProgramUniformMatrix3fv(m_glProgramID, i, 1, GL_FALSE, glm::value_ptr(*o)); }
	void setUniform(const GLuint & i, const glm::mat4 * o) { glProgramUniformMatrix4fv(m_glProgramID, i, 1, GL_FALSE, glm::value_ptr(*o)); }
	void setUniformArray(const GLuint & i, const int & o, const int & size) { glProgramUniform1iv(m_glProgramID, i, size, &o); }
	void setUniformArray(const GLuint & i, const double & o, const int & size) { glProgramUniform1dv(m_glProgramID, i, size, &o); }
	void setUniformArray(const GLuint & i, const float & o, const int & size) { glProgramUniform1fv(m_glProgramID, i, size, &o); }
	void setUniformArray(const GLuint & i, const GLuint64 & o, const int & size) { glProgramUniformHandleui64vARB(m_glProgramID, i, size, &o); }
	void setUniformArray(const GLuint & i, const glm::vec2 & o, const int & size) { glProgramUniform2fv(m_glProgramID, i, size, glm::value_ptr(o)); }
	void setUniformArray(const GLuint & i, const glm::vec3 & o, const int & size) { glProgramUniform3fv(m_glProgramID, i, size, glm::value_ptr(o)); }
	void setUniformArray(const GLuint & i, const glm::vec4 & o, const int & size) { glProgramUniform4fv(m_glProgramID, i, size, glm::value_ptr(o)); }
	void setUniformArray(const GLuint & i, const glm::mat4 & o, const int & size) { glProgramUniformMatrix4fv(m_glProgramID, i, size, GL_FALSE, glm::value_ptr(o)); }
	void setUniformArray(const GLuint & i, const int * o, const int & size) { glProgramUniform1iv(m_glProgramID, i, size, o); }
	void setUniformArray(const GLuint & i, const double * o, const int & size) { glProgramUniform1dv(m_glProgramID, i, size, o); }
	void setUniformArray(const GLuint & i, const float * o, const int & size) { glProgramUniform1fv(m_glProgramID, i, size, o); }
	void setUniformArray(const GLuint & i, const GLuint64 * o, const int & size) { glProgramUniformHandleui64vARB(m_glProgramID, i, size, o); }
	void setUniformArray(const GLuint & i, const glm::vec2 * o, const int & size) { glProgramUniform2fv(m_glProgramID, i, size, glm::value_ptr(*o)); }
	void setUniformArray(const GLuint & i, const glm::vec3 * o, const int & size) { glProgramUniform3fv(m_glProgramID, i, size, glm::value_ptr(*o)); }
	void setUniformArray(const GLuint & i, const glm::vec4 * o, const int & size) { glProgramUniform4fv(m_glProgramID, i, size, glm::value_ptr(*o)); }
	void setUniformArray(const GLuint & i, const glm::mat4 * o, const int & size) { glProgramUniformMatrix4fv(m_glProgramID, i, size, GL_FALSE, glm::value_ptr(*o)); }
	void setUniformMatrixArray(const GLuint & i, const float * o, const int & size, const GLboolean & transpose) { glProgramUniformMatrix4fv(m_glProgramID, i, size, transpose, o); }
	

	// Public Attributes
	GLuint m_glProgramID = 0, m_glVertexID = 0, m_glFragmentID = 0; // OpenGL ID's
	std::string m_vertexText = "", m_fragmentText = ""; // Text Data


protected:
	// Protected Constructors
	/** Construct the Shader. */
	Asset_Shader(const std::string & filename);


private:
	// Private Methods
	// Interface Implementation
	virtual void initializeDefault(Engine * engine) override;
	virtual void initialize(Engine * engine, const std::string & fullDirectory) override;
	virtual void finalize(Engine * engine) override;


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_SHADER_H
