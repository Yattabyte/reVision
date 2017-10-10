/*
	Asset_Shader
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
	
	~Asset_Shader();
	Asset_Shader();
	Asset_Shader(const string &_filename);
	static int GetAssetType() { return 0; }

	GLuint gl_program_ID, gl_shader_vertex_ID, gl_shader_fragment_ID, gl_shader_geometry_ID;
	string filename, vertex_text, fragment_text, geometry_text;

	void Bind();
	static ASSET_SHADER_API void Release();

	void Destroy();
	void Finalize();
	void Compile();
	void Compile_Single_Shader(GLuint & ID, const char * source, const GLenum & type);
	void GenerateProgram();
	void LinkProgram();

	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const bool &b);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const int &o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const double &o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const float &o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const vec2 &o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const vec3 &o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const vec4 &o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const ivec2 &o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const ivec3 &o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const ivec4 &o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const mat3 &o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const mat4 &o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const int *o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const double *o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const float *o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const vec2 *o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const vec3 *o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const vec4 *o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const mat3 *o);
	static ASSET_SHADER_API void setLocationValue(const GLuint &i, const mat4 *o);
	static ASSET_SHADER_API void setLocationValueArray(const GLuint &i, const int &o, const int &size);
	static ASSET_SHADER_API void setLocationValueArray(const GLuint &i, const double &o, const int &size);
	static ASSET_SHADER_API void setLocationValueArray(const GLuint &i, const float &o, const int &size);
	static ASSET_SHADER_API void setLocationValueArray(const GLuint &i, const vec2 &o, const int &size);
	static ASSET_SHADER_API void setLocationValueArray(const GLuint &i, const vec3 &o, const int &size);
	static ASSET_SHADER_API void setLocationValueArray(const GLuint &i, const vec4 &o, const int &size);
	static ASSET_SHADER_API void setLocationValueArray(const GLuint &i, const mat4 &o, const int &size);
	static ASSET_SHADER_API void setLocationValueArray(const GLuint &i, const int *o, const int &size);
	static ASSET_SHADER_API void setLocationValueArray(const GLuint &i, const double *o, const int &size);
	static ASSET_SHADER_API void setLocationValueArray(const GLuint &i, const float *o, const int &size);
	static ASSET_SHADER_API void setLocationValueArray(const GLuint &i, const vec2 *o, const int &size);
	static ASSET_SHADER_API void setLocationValueArray(const GLuint &i, const vec3 *o, const int &size);
	static ASSET_SHADER_API void setLocationValueArray(const GLuint &i, const vec4 *o, const int &size);
	static ASSET_SHADER_API void setLocationValueArray(const GLuint &i, const mat4 *o, const int &size);
	static ASSET_SHADER_API void setLocationMatArray(const GLuint &i, const float * o, const int &size, const GLboolean &transpose);
};

typedef shared_ptr<Asset_Shader> Shared_Asset_Shader;
namespace Asset_Manager {
	void ASSET_SHADER_API load_asset(Shared_Asset_Shader &user, const string &filename, const bool &regenerate = false, const bool &threaded = true);
}
#endif // ASSET_SHADER
