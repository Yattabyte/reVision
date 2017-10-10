/*
	Asset_Shader

	- Encapsulates an OpenGL shader program
	- Shader program uses multiple shader files with same name
		- Example: "World_Shader.vertex", "World_Shader.fragment", "World_Shader.geometry"
	- Shader uniform values set by binding the appropriate shader, and then modifying the correct location by using the static "setLocation" functions
*/

#pragma once
#ifndef	ASSET_SHADER
#define	ASSET_SHADER
#ifdef	ASSET_SHADER_EXPORT
#define	ASSET_SHADER_API __declspec(dllexport)
#else
#define	ASSET_SHADER_API __declspec(dllimport)
#endif

#include "Modules\Asset_manager.h"
#include "GLM\common.hpp"
#include "GL\glew.h"
#include <string>

using namespace glm;
using namespace std;

class Asset_Shader : public Asset
{
public:	
	/*************
	----Common----
	*************/

	~Asset_Shader(); 
	Asset_Shader(); 
	ASSET_SHADER_API static int GetAssetType();
	ASSET_SHADER_API virtual void Finalize();

	/****************
	----Variables----
	****************/

	GLuint gl_program_ID, gl_shader_vertex_ID, gl_shader_fragment_ID, gl_shader_geometry_ID; // OpenGL ID's
	string filename, vertex_text, fragment_text, geometry_text; // Text Data

	/***********************
	----Shader Functions----
	***********************/

	// Destroy's this shader program's opengl shader state
	ASSET_SHADER_API void Destroy();
	// Creates and compiles all available shader types for this program
	ASSET_SHADER_API void Compile();
	// Creates and compiles a single shader file, given the text as <source> and the specified shader type as <type>.
	ASSET_SHADER_API void Compile_Single_Shader(GLuint & ID, const char * source, const GLenum & type);
	// Generates an OpenGL shader program ID for this class, and attempts to attach any available shaders
	ASSET_SHADER_API void GenerateProgram();
	// Attempts to link and validate the shader program
	ASSET_SHADER_API void LinkProgram();
	// Make this shader program active
	ASSET_SHADER_API void Bind();
	// Inactivate any currently bound shader program
	ASSET_SHADER_API static void Release();

	/****************************************************************************************************
 	----Convenience functions for setting uniform values at a given location, while a shader is bound----
	****************************************************************************************************/

	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const bool &b);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const int &o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const double &o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const float &o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const vec2 &o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const vec3 &o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const vec4 &o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const ivec2 &o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const ivec3 &o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const ivec4 &o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const mat3 &o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const mat4 &o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const int *o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const double *o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const float *o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const vec2 *o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const vec3 *o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const vec4 *o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const mat3 *o);
	ASSET_SHADER_API static void setLocationValue(const GLuint &i, const mat4 *o);
	ASSET_SHADER_API static void setLocationValueArray(const GLuint &i, const int &o, const int &size);
	ASSET_SHADER_API static void setLocationValueArray(const GLuint &i, const double &o, const int &size);
	ASSET_SHADER_API static void setLocationValueArray(const GLuint &i, const float &o, const int &size);
	ASSET_SHADER_API static void setLocationValueArray(const GLuint &i, const vec2 &o, const int &size);
	ASSET_SHADER_API static void setLocationValueArray(const GLuint &i, const vec3 &o, const int &size);
	ASSET_SHADER_API static void setLocationValueArray(const GLuint &i, const vec4 &o, const int &size);
	ASSET_SHADER_API static void setLocationValueArray(const GLuint &i, const mat4 &o, const int &size);
	ASSET_SHADER_API static void setLocationValueArray(const GLuint &i, const int *o, const int &size);
	ASSET_SHADER_API static void setLocationValueArray(const GLuint &i, const double *o, const int &size);
	ASSET_SHADER_API static void setLocationValueArray(const GLuint &i, const float *o, const int &size);
	ASSET_SHADER_API static void setLocationValueArray(const GLuint &i, const vec2 *o, const int &size);
	ASSET_SHADER_API static void setLocationValueArray(const GLuint &i, const vec3 *o, const int &size);
	ASSET_SHADER_API static void setLocationValueArray(const GLuint &i, const vec4 *o, const int &size);
	ASSET_SHADER_API static void setLocationValueArray(const GLuint &i, const mat4 *o, const int &size);
	ASSET_SHADER_API static void setLocationMatArray(const GLuint &i, const float * o, const int &size, const GLboolean &transpose);		
};

typedef shared_ptr<Asset_Shader> Shared_Asset_Shader;
namespace Asset_Manager {
	// Attempts to create an asset from disk or share one if it already exists
	ASSET_SHADER_API void load_asset(Shared_Asset_Shader &user, const string &filename, const bool &regenerate = false, const bool &threaded = true);
}
#endif // ASSET_SHADER
