/*
	FileReader

	- A utility for reading certain things from disk
	- Mostly used for reading my our own format stuff	
*/

#pragma once
#ifndef	FILEREADER
#define	FILEREADER
#ifdef	FILEREADER_EXPORT
#define	FILEREADER_API __declspec(dllexport)
#else
#define	FILEREADER_API __declspec(dllimport)
#endif

#include "GLM\common.hpp"
#include "GLM\gtc\quaternion.hpp"
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;
using namespace glm;

namespace FileReader {

	// A utility for interacting with text driven documents
	namespace DocParser {
		// A terrible solution
		struct Property {
			string s;
			int i_1;
			ivec2 i_2;
			ivec3 i_3;
			ivec4 i_4;
			float f_1;
			vec2 f_2;
			vec3 f_3;
			vec4 f_4;

			vector<string> v_s;
			vector<float> v_f_1;
			vector<vec2> v_f_2;
			vector<vec3> v_f_3;
			vector<vec4> v_f_4;
			vector<int> v_i_1;
			vector<ivec2> v_i_2;
			vector<ivec3> v_i_3;
			vector<ivec4> v_i_4;
		};
		
		// Updates the @property token using the @input string. Returns true if it got any data from it
		FILEREADER_API bool getProperty(istringstream &string_stream, Property &property, string &input = string(""));

		/*********************************************************************
		*--------------------------Getter Functions--------------------------*			
		*----Converts @string_stream into an appropriate value in @target----*
		*********************************************************************/

		static void getValue(istringstream &string_stream, string &target);
		static void getValue(istringstream &string_stream, int &target);
		static void getValue(istringstream &string_stream, ivec2 &target);
		static void getValue(istringstream &string_stream, ivec3 &target);
		static void getValue(istringstream &string_stream, ivec4 &target);
		static void getValue(istringstream &string_stream, float &target);
		static void getValue(istringstream &string_stream, vec2 &target);
		static void getValue(istringstream &string_stream, vec3 &target);
		static void getValue(istringstream &string_stream, vec4 &target);
		static void getValue(istringstream &string_stream, vector<string> &target);
		static void getValue(istringstream &string_stream, vector<int> &target);
		static void getValue(istringstream &string_stream, vector<ivec2> &target);
		static void getValue(istringstream &string_stream, vector<ivec3> &target);
		static void getValue(istringstream &string_stream, vector<ivec4> &target);
		static void getValue(istringstream &string_stream, vector<float> &target);
		static void getValue(istringstream &string_stream, vector<vec2> &target);
		static void getValue(istringstream &string_stream, vector<vec3> &target);
		static void getValue(istringstream &string_stream, vector<vec4> &target);

		/*********************************************************************
		*--------------------------Setter Functions--------------------------*
		*----Converts @target into an appropriate string, then returns it----*
		*********************************************************************/

		static string setValue(const string &target);
		static string setValue(const int &target);
		static string setValue(const ivec2 &target);
		static string setValue(const ivec3 &target);
		static string setValue(const ivec4 &target);
		static string setValue(const float &target);
		static string setValue(const vec2 &target);
		static string setValue(const vec3 &target);
		static string setValue(const vec4 &target);
		static string setValue(const quat &target);
		static string setValue(const vector<ivec2> &target);
		static string setValue(const vector<vec2> &target);
		static string setValue(const vector<vec3> &target);
	}
};

#endif // FILEREADER