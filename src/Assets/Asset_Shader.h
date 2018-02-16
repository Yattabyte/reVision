#pragma once
#ifndef	ASSET_SHADER
#define	ASSET_SHADER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define EXT_SHADER_VERTEX ".vsh"
#define EXT_SHADER_FRAGMENT ".fsh"
#define EXT_SHADER_GEOMETRY ".gsh"
#define DIRECTORY_SHADER File_Reader::GetCurrentDir() + "\\Shaders\\"

#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include "Utilities\File_Reader.h"
#include "glm\glm.hpp"
#include "GL\glew.h"
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
class DT_ENGINE_API Asset_Shader : public Asset
{
public:	
	// (de)Constructors
	/** Destroy the Shader. */
	~Asset_Shader();
	/** Construct the Shader. */
	Asset_Shader(const string & filename);


	// Methods
	/** @todo delete */
	static int Get_Asset_Type();	
	/** Make this shader program active */
	void bind();
	/** Inactivate any currently bound shader program. */
	static void Release();	
	
	/****************************************************************************************************
 	----Convenience functions for setting uniform values at a given location, while a shader is bound----
	****************************************************************************************************/

	static void Set_Uniform(const GLuint & i, const bool & o);
	static void Set_Uniform(const GLuint & i, const int & o);
	static void Set_Uniform(const GLuint & i, const double & o);
	static void Set_Uniform(const GLuint & i, const float & o);
	static void Set_Uniform(const GLuint & i, const vec2 & o);
	static void Set_Uniform(const GLuint & i, const vec3 & o);
	static void Set_Uniform(const GLuint & i, const vec4 & o);
	static void Set_Uniform(const GLuint & i, const ivec2 & o);
	static void Set_Uniform(const GLuint & i, const ivec3 & o);
	static void Set_Uniform(const GLuint & i, const ivec4 & o);
	static void Set_Uniform(const GLuint & i, const mat3 & o);
	static void Set_Uniform(const GLuint & i, const mat4 & o);
	static void Set_Uniform(const GLuint & i, const int * o);
	static void Set_Uniform(const GLuint & i, const double * o);
	static void Set_Uniform(const GLuint & i, const float * o);
	static void Set_Uniform(const GLuint & i, const vec2 * o);
	static void Set_Uniform(const GLuint & i, const vec3 * o);
	static void Set_Uniform(const GLuint & i, const vec4 * o);
	static void Set_Uniform(const GLuint & i, const mat3 * o);
	static void Set_Uniform(const GLuint & i, const mat4 * o);
	static void Set_Uniform_Array(const GLuint & i, const int & o, const int & size);
	static void Set_Uniform_Array(const GLuint & i, const double & o, const int & size);
	static void Set_Uniform_Array(const GLuint & i, const float & o, const int & size);
	static void Set_Uniform_Array(const GLuint & i, const vec2 & o, const int & size);
	static void Set_Uniform_Array(const GLuint & i, const vec3 & o, const int & size);
	static void Set_Uniform_Array(const GLuint & i, const vec4 & o, const int & size);
	static void Set_Uniform_Array(const GLuint & i, const mat4 & o, const int & size);
	static void Set_Uniform_Array(const GLuint & i, const int * o, const int & size);
	static void Set_Uniform_Array(const GLuint & i, const double * o, const int & size);
	static void Set_Uniform_Array(const GLuint & i, const float * o, const int & size);
	static void Set_Uniform_Array(const GLuint & i, const vec2 * o, const int & size);
	static void Set_Uniform_Array(const GLuint & i, const vec3 * o, const int & size);
	static void Set_Uniform_Array(const GLuint & i, const vec4 * o, const int & size);
	static void Set_Uniform_Array(const GLuint & i, const mat4 * o, const int & size);
	static void Set_Uniform_Mat_Array(const GLuint & i, const float * o, const int & size, const GLboolean & transpose);		

	
	// Interface Implementation
	/** Returns whether or not this asset has completed finalizing.
	 * @return	true if this asset has finished finalizing, false otherwise. */
	virtual bool existsYet();


	// Attributes
	GLuint gl_program_ID, gl_shader_vertex_ID, gl_shader_fragment_ID, gl_shader_geometry_ID; // OpenGL ID's
	string vertex_text, fragment_text, geometry_text; // Text Data
	GLsync m_fence;
};


/**
 * Namespace that provides functionality for loading assets.
 **/
namespace Asset_Loader {
	/** Attempts to create an asset from disk or share one if it already exists */
	DT_ENGINE_API void load_asset(Shared_Asset_Shader & user, const string & filename, const bool & threaded = true);
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
	// Methods
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
	
	
	// Attributes
	string m_filename;
	Shared_Asset_Shader m_asset;
};

#endif // ASSET_SHADER
