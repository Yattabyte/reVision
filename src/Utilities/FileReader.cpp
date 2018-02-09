#include "Utilities\FileReader.h"
#include <fstream>
#include <direct.h>

namespace FileReader {
	namespace DocParser {
		void getValue(istringstream & string_stream, string & target)
		{
			string_stream >> target;
		}

		void getValue(istringstream & string_stream, int & target)
		{
			string_stream >> target; 
		}

		void getValue(istringstream & string_stream, ivec2 & target)
		{ 
			string_stream >> target.x >> target.y; 
		}

		void getValue(istringstream & string_stream, ivec3 & target)
		{ 
			string_stream >> target.x >> target.y >> target.z; 
		}

		void getValue(istringstream & string_stream, ivec4 & target)
		{ 
			string_stream >> target.x >> target.y >> target.z >> target.w; 
		}

		void getValue(istringstream & string_stream, float & target)
		{ 
			string_stream >> target; 
		}

		void getValue(istringstream & string_stream, vec2 & target)
		{ 
			string_stream >> target.x >> target.y; 
		}

		void getValue(istringstream & string_stream, vec3 & target)
		{ 
			string_stream >> target.x >> target.y >> target.z;
		}

		void getValue(istringstream & string_stream, vec4 & target)
		{
			string_stream >> target.x >> target.y >> target.z >> target.w; 
		}

		void getValue(istringstream & string_stream, vector<string>& target) {
			int size = 0; string_stream >> size;
			target.reserve(size);
			for (int x = 0; x < size; ++x) {
				string v;
				string_stream >> v;
				target.push_back(v);
			}
		}

		void getValue(istringstream & string_stream, vector<int>& target) {
			int size = 0; string_stream >> size;
			target.reserve(size);
			for (int x = 0; x < size; ++x) {
				int v;
				string_stream >> v;
				target.push_back(v);
			}
		}

		void getValue(istringstream & string_stream, vector<ivec2>& target) {
			int size = 0; string_stream >> size;
			target.reserve(size);
			for (int x = 0; x < size; ++x) {
				ivec2 v;
				string_stream >> v.x >> v.y;
				target.push_back(v);
			}
		}

		void getValue(istringstream & string_stream, vector<ivec3>& target) {
			int size = 0; string_stream >> size;
			target.reserve(size);
			for (int x = 0; x < size; ++x) {
				ivec3 v;
				string_stream >> v.x >> v.y >> v.z;
				target.push_back(v);
			}
		}

		void getValue(istringstream & string_stream, vector<ivec4>& target) {
			int size = 0; string_stream >> size;
			target.reserve(size);
			for (int x = 0; x < size; ++x) {
				ivec4 v;
				string_stream >> v.x >> v.y >> v.z >> v.w;
				target.push_back(v);
			}
		}

		void getValue(istringstream & string_stream, vector<float>& target) {
			int size = 0; string_stream >> size;
			target.reserve(size);
			for (int x = 0; x < size; ++x) {
				float v;
				string_stream >> v;
				target.push_back(v);
			}
		}

		void getValue(istringstream & string_stream, vector<vec2>& target) {
			int size = 0; string_stream >> size;
			target.reserve(size);
			for (int x = 0; x < size; ++x) {
				vec2 v;
				string_stream >> v.x >> v.y;
				target.push_back(v);
			}
		}

		void getValue(istringstream & string_stream, vector<vec3>& target) {
			int size = 0; string_stream >> size;
			target.reserve(size);
			for (int x = 0; x < size; ++x) {
				vec3 v;
				string_stream >> v.x >> v.y >> v.z;
				target.push_back(v);
			}
		}

		void getValue(istringstream & string_stream, vector<vec4>& target) {
			int size = 0; string_stream >> size;
			target.reserve(size);
			for (int x = 0; x < size; ++x) {
				vec4 v;
				string_stream >> v.x >> v.y >> v.z >> v.w;
				target.push_back(v);
			}
		}

		bool getProperty(istringstream & string_stream, Property & property, string & input)
		{
			string_stream >> input;
			if (input == "string") getValue(string_stream, property.s);
			else if (input == "int") getValue(string_stream, property.i_1);
			else if (input == "ivec2") getValue(string_stream, property.i_2);
			else if (input == "ivec3") getValue(string_stream, property.i_3);
			else if (input == "ivec4") getValue(string_stream, property.i_4);
			else if (input == "float") getValue(string_stream, property.f_1);
			else if (input == "vec2") getValue(string_stream, property.f_2);
			else if (input == "vec3") getValue(string_stream, property.f_3);
			else if (input == "vec4") getValue(string_stream, property.f_4);
			else if (input == "ivec2_array") getValue(string_stream, property.v_i_2);
			else if (input == "vec2_array") getValue(string_stream, property.v_f_2);
			else if (input == "vec3_array") getValue(string_stream, property.v_f_3);
			else return false;
			return true;
		}

		string setValue(const string & target)
		{ 
			return "string " + target; 
		}

		string setValue(const int & target)
		{ 
			return "int " + to_string(target); 
		}

		string setValue(const ivec2 & target)
		{ 
			return "ivec2 " + to_string(target.x) + " " + to_string(target.y); 
		}

		string setValue(const ivec3 & target)
		{
			return "ivec3 " + to_string(target.x) + " " + to_string(target.y) + " " + to_string(target.z);
		}

		string setValue(const ivec4 & target)
		{
			return "ivec4 " + to_string(target.x) + " " + to_string(target.y) + " " + to_string(target.z) + " " + to_string(target.w);
		}

		string setValue(const float & target)
		{
			return "float " + to_string(target);
		}

		string setValue(const vec2 & target)
		{ 
			return "vec2 " + to_string(target.x) + " " + to_string(target.y); 
		}

		string setValue(const vec3 & target)
		{ 
			return "vec3 " + to_string(target.x) + " " + to_string(target.y) + " " + to_string(target.z); 
		}

		string setValue(const vec4 & target)
		{ 
			return "vec4 " + to_string(target.x) + " " + to_string(target.y) + " " + to_string(target.z) + " " + to_string(target.w); 
		}

		string setValue(const quat & target)
		{ 
			return "vec4 " + to_string(target.x) + " " + to_string(target.y) + " " + to_string(target.z) + " " + to_string(target.w); 
		}

		string setValue(const vector<ivec2>& target)
		{
			string string = "ivec2_array " + to_string(target.size());
			for each (const ivec2 &n in target) {
				string += " " + to_string(n.x) + " " + to_string(n.y);
			}
			return string;
		}

		string setValue(const vector<vec2>& target)
		{
			string string = "vec2_array " + to_string(target.size());
			for each (const vec2 &n in target) {
				string += " " + to_string(n.x) + " " + to_string(n.y);
			}
			return string;
		}

		string setValue(const vector<vec3>& target)
		{
			string string = "vec3_array " + to_string(target.size());
			for each (const vec3 &n in target) {
				string += " " + to_string(n.x) + " " + to_string(n.y) + " " + to_string(n.z);
			}
			return string;
		}
	}

	bool ReadFileFromDisk(string & returnFile, const string & fileDirectory)
	{
		struct stat buffer;
		if (stat(fileDirectory.c_str(), &buffer))
			return false;

		ifstream file(fileDirectory);
		while (!file.eof()) {
			string temp;
			std::getline(file, temp);
			returnFile.append(temp + '\n');
		}

		return true;
	}
	bool FileExistsOnDisk(const string & name)
	{
		// Technique to return whether or not a given file or folder exists
		struct stat buffer;
		return (stat(name.c_str(), &buffer) == 0);		
	}
	string GetCurrentDir()
	{
		// Technique to return the running directory of the application
		char cCurrentPath[FILENAME_MAX];
		if (_getcwd(cCurrentPath, sizeof(cCurrentPath)))
			cCurrentPath[sizeof(cCurrentPath) - 1] = '/0';
		return string(cCurrentPath);
	}
}