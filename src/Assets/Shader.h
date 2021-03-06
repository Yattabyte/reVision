#pragma once
#ifndef	SHADER_H
#define	SHADER_H

#include "Assets/Asset.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"


// Forward Declarations
class Shader;

/** Shared version of a Shader asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Shader final : public std::shared_ptr<Shader> {
public:
	// Public (De)Constructors
	/** Constructs an empty asset. */
	inline Shared_Shader() = default;
	/** Begins the creation process for this asset.
	@param	engine			reference to the engine to use.
	@param	filename		the filename to use.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	Shared_Shader(Engine& engine, const std::string& filename, const bool& threaded = true);
};

/** A single shader object, like a fragment shader or a vertex shader, not a whole shader program. */
struct ShaderObj {
	// (De)Constructors
	/** Destroy the shader object. */
	~ShaderObj();
	/** Construct a shader object. */
	explicit ShaderObj(const GLenum& type);
	/** Move a shader object. */
	ShaderObj(ShaderObj&&) noexcept;
	/** Copy a shader object. */
	ShaderObj(const ShaderObj&);
	/** Disallow shader object move assignment. */
	inline ShaderObj& operator =(ShaderObj&&) = delete;
	/** Disallow shader object copy assignment. */
	inline ShaderObj& operator =(const ShaderObj&) = delete;


	// Functions
	/** Retrieve a shader parameter by the name specified.
	@param	parameterName		the program parameter name.
	@return						the parameter value matching the name specified. */
	GLint getShaderiv(const GLenum& parameterName) const noexcept;
	/** Load a shader document from the file path specified.
	@param	engine				the active engine to use.
	@param	filePath			the relative path to the file to read.
	@return						true on success, false otherwise. */
	bool loadDocument(Engine& engine, const std::string& filePath);
	/** Create an OpenGL shader object from this class, using the document string loaded.
	@param	engine				the active engine to use.
	@param	filename			the shader file name (for reporting purposes).
	@return						true on success, false otherwise. */
	bool createGLShader(Engine& engine, const std::string& filename);


	// Attributes
	GLuint m_shaderID = 0;
	std::string m_shaderText = "";
	GLenum m_type = GL_VERTEX_SHADER;
};

/** An entire OpenGL vertex/fragment shader program.
An encapsulation of an OpenGL vertex & fragment shader program.
Responsible for loading the files associated with this program from disk, and forming the program.
Also provides support for explicitly setting uniform values for a given attribute location.
Supports binary representation. */
class Shader : public Asset {
public:
	// Public (De)Constructors
	/** Destroy the Shader. */
	~Shader();
	/** Construct the Shader.
	@param	engine				reference to the engine to use.
	@param	filename			the asset file name (relative to engine directory). */
	Shader(Engine& engine, const std::string& filename);


public:
	// Public Methods
	/** Make this shader program active */
	void bind() noexcept;
	/** Inactivate any currently bound shader program. */
	static void Release() noexcept;


	/**********************************************************************
	----Convenient DSA functions for changing program uniform variables----
	***********************************************************************/

	inline void setUniform(const GLuint& i, const bool& o) noexcept { glProgramUniform1i(m_glProgramID, i, o); }
	inline void setUniform(const GLuint& i, const int& o) noexcept { glProgramUniform1i(m_glProgramID, i, o); }
	inline void setUniform(const GLuint& i, const unsigned int& o) noexcept { glProgramUniform1ui(m_glProgramID, i, o); }
	inline void setUniform(const GLuint& i, const double& o) noexcept { glProgramUniform1d(m_glProgramID, i, o); }
	inline void setUniform(const GLuint& i, const float& o) noexcept { glProgramUniform1f(m_glProgramID, i, o); }
	inline void setUniform(const GLuint& i, const GLuint64& o) noexcept { glProgramUniformHandleui64ARB(m_glProgramID, i, o); }
	inline void setUniform(const GLuint& i, const glm::vec2& o) noexcept { glProgramUniform2f(m_glProgramID, i, o.x, o.y); }
	inline void setUniform(const GLuint& i, const glm::vec3& o) noexcept { glProgramUniform3f(m_glProgramID, i, o.x, o.y, o.z); }
	inline void setUniform(const GLuint& i, const glm::vec4& o) noexcept { glProgramUniform4f(m_glProgramID, i, o.x, o.y, o.z, o.w); }
	inline void setUniform(const GLuint& i, const glm::ivec2& o) noexcept { glProgramUniform2i(m_glProgramID, i, o.x, o.y); }
	inline void setUniform(const GLuint& i, const glm::ivec3& o) noexcept { glProgramUniform3i(m_glProgramID, i, o.x, o.y, o.z); }
	inline void setUniform(const GLuint& i, const glm::ivec4& o) noexcept { glProgramUniform4i(m_glProgramID, i, o.x, o.y, o.z, o.w); }
	inline void setUniform(const GLuint& i, const glm::mat3& o) noexcept { glProgramUniformMatrix3fv(m_glProgramID, i, 1, GL_FALSE, &o[0][0]); }
	inline void setUniform(const GLuint& i, const glm::mat4& o) noexcept { glProgramUniformMatrix4fv(m_glProgramID, i, 1, GL_FALSE, &o[0][0]); }
	inline void setUniform(const GLuint& i, const int* o) noexcept { glProgramUniform1iv(m_glProgramID, i, 1, o); }
	inline void setUniform(const GLuint& i, const unsigned int* o) noexcept { glProgramUniform1uiv(m_glProgramID, i, 1, o); }
	inline void setUniform(const GLuint& i, const double* o) noexcept { glProgramUniform1dv(m_glProgramID, i, 1, o); }
	inline void setUniform(const GLuint& i, const float* o) noexcept { glProgramUniform1fv(m_glProgramID, i, 1, o); }
	inline void setUniform(const GLuint& i, const GLuint64* o) noexcept { glProgramUniformHandleui64ARB(m_glProgramID, i, *o); }
	inline void setUniform(const GLuint& i, const glm::vec2* o) { glProgramUniform2fv(m_glProgramID, i, 1, glm::value_ptr(*o)); }
	inline void setUniform(const GLuint& i, const glm::vec3* o) { glProgramUniform3fv(m_glProgramID, i, 1, glm::value_ptr(*o)); }
	inline void setUniform(const GLuint& i, const glm::vec4* o) { glProgramUniform4fv(m_glProgramID, i, 1, glm::value_ptr(*o)); }
	inline void setUniform(const GLuint& i, const glm::mat3* o) { glProgramUniformMatrix3fv(m_glProgramID, i, 1, GL_FALSE, glm::value_ptr(*o)); }
	inline void setUniform(const GLuint& i, const glm::mat4* o) { glProgramUniformMatrix4fv(m_glProgramID, i, 1, GL_FALSE, glm::value_ptr(*o)); }
	inline void setUniformArray(const GLuint& i, const int& o, const int& count) noexcept { glProgramUniform1iv(m_glProgramID, i, count, &o); }
	inline void setUniformArray(const GLuint& i, const unsigned int& o, const int& count) noexcept { glProgramUniform1uiv(m_glProgramID, i, count, &o); }
	inline void setUniformArray(const GLuint& i, const double& o, const int& count) noexcept { glProgramUniform1dv(m_glProgramID, i, count, &o); }
	inline void setUniformArray(const GLuint& i, const float& o, const int& count) noexcept { glProgramUniform1fv(m_glProgramID, i, count, &o); }
	inline void setUniformArray(const GLuint& i, const GLuint64& o, const int& count) noexcept { glProgramUniformHandleui64vARB(m_glProgramID, i, count, &o); }
	inline void setUniformArray(const GLuint& i, const glm::vec2& o, const int& count) { glProgramUniform2fv(m_glProgramID, i, count, glm::value_ptr(o)); }
	inline void setUniformArray(const GLuint& i, const glm::vec3& o, const int& count) { glProgramUniform3fv(m_glProgramID, i, count, glm::value_ptr(o)); }
	inline void setUniformArray(const GLuint& i, const glm::vec4& o, const int& count) { glProgramUniform4fv(m_glProgramID, i, count, glm::value_ptr(o)); }
	inline void setUniformArray(const GLuint& i, const glm::mat4& o, const int& count) { glProgramUniformMatrix4fv(m_glProgramID, i, count, GL_FALSE, glm::value_ptr(o)); }
	inline void setUniformArray(const GLuint& i, const int* o, const int& count) noexcept { glProgramUniform1iv(m_glProgramID, i, count, o); }
	inline void setUniformArray(const GLuint& i, const unsigned int* o, const int& count) noexcept { glProgramUniform1uiv(m_glProgramID, i, count, o); }
	inline void setUniformArray(const GLuint& i, const double* o, const int& count) noexcept { glProgramUniform1dv(m_glProgramID, i, count, o); }
	inline void setUniformArray(const GLuint& i, const float* o, const int& count) noexcept { glProgramUniform1fv(m_glProgramID, i, count, o); }
	inline void setUniformArray(const GLuint& i, const GLuint64* o, const int& count) noexcept { glProgramUniformHandleui64vARB(m_glProgramID, i, count, o); }
	inline void setUniformArray(const GLuint& i, const glm::vec2* o, const int& count) { glProgramUniform2fv(m_glProgramID, i, count, glm::value_ptr(*o)); }
	inline void setUniformArray(const GLuint& i, const glm::vec3* o, const int& count) { glProgramUniform3fv(m_glProgramID, i, count, glm::value_ptr(*o)); }
	inline void setUniformArray(const GLuint& i, const glm::vec4* o, const int& count) { glProgramUniform4fv(m_glProgramID, i, count, glm::value_ptr(*o)); }
	inline void setUniformArray(const GLuint& i, const glm::mat4* o, const int& count) { glProgramUniformMatrix4fv(m_glProgramID, i, count, GL_FALSE, glm::value_ptr(*o)); }
	inline void setUniformMatrixArray(const GLuint& i, const float* o, const int& count, const GLboolean& transpose) noexcept { glProgramUniformMatrix4fv(m_glProgramID, i, count, transpose, o); }


	// Public Attributes
	GLuint m_glProgramID = 0;
	ShaderObj m_vertexShader = ShaderObj(GL_VERTEX_SHADER);
	ShaderObj m_fragmentShader = ShaderObj(GL_FRAGMENT_SHADER);


protected:
	// Protected Methods
	/** Retrieve a program parameter by the name specified.
	@param	parameterName		the program parameter name.
	@return						the parameter value matching the name specified. */
	GLint getProgramiv(const GLenum& parameterName) const noexcept;
	/** Retrieve an error log corresponding to this shader program.
	@return						an error log for this shader program. */
	[[nodiscard]] std::vector<GLchar> getErrorLog() const;
	/** Attempt to load a shader program from a cached binary file.
	@param	relativePath		the relative path of the binary file.
	@return						true on success, false otherwise. */
	bool loadCachedBinary(const std::string& relativePath);
	/** Attempt to save a shader program to a cached binary file.
	@param	relativePath		the relative path of the binary file.
	@return						true on success, false otherwise. */
	bool saveCachedBinary(const std::string& relativePath);
	/** Attempt to delete a shader program's cached binary file.
	@param	relativePath		the relative path of the binary file.
	@return						true on success, false otherwise. */
	static bool deleteCachedBinary(const std::string& relativePath);
	/** Attempt to load a shader program from separate shader files.
	@param	relativePath		the relative path of the shader files.
	@return						true on success, false otherwise. */
	virtual bool initShaders(const std::string& relativePath);
	/** Use to validate this shader program after linking.
	@return						true on success, false otherwise. */
	bool validateProgram() noexcept;


private:
	// Private but deleted
	/** Disallow asset move constructor. */
	inline Shader(Shader&&) noexcept = delete;
	/** Disallow asset copy constructor. */
	inline Shader(const Shader&) noexcept = delete;
	/** Disallow asset move assignment. */
	inline Shader& operator =(Shader&&) noexcept = delete;
	/** Disallow asset copy assignment. */
	inline Shader& operator =(const Shader&) noexcept = delete;


	// Private Interface Implementation
	void initialize() override;


	// Private Attributes
	friend class Shared_Shader;
};

#endif // SHADER_H