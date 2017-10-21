/*
	Message Manager

	- Manages engine related error message and statement outputs
	- Has handy enum and string lists with definitions
	- Can print raw text or a specific error
	- By default, prints via std::cout
	- Holds a log of text in case they need to be access by any external UI
*/

#pragma once
#ifndef MESSAGE_MANAGER
#define MESSAGE_MANAGER
#ifdef	CORE_EXPORT
#define MESSAGE_MANAGER_API __declspec(dllexport)
#else
#define	MESSAGE_MANAGER_API __declspec(dllimport)
#endif

#include <string>

using namespace std;

namespace MSG {
	// Prints a raw string @input to the message log
	MESSAGE_MANAGER_API void Statement(const string &input);
	// Prints a formatted message using the error @error_number
	MESSAGE_MANAGER_API void Error(const int &error_number, const string &input, const std::string &additional_input = "");
}

enum Error_Enum
{
	FILE_MISSING,
	DIRECTORY_MISSING,
	FILE_CORRUPT,

	FBO_INCOMPLETE,
	SHADER_INCOMPLETE,
	PROGRAM_INCOMPLETE,

	GLFW_ERROR,
	ERROR_COUNT
};

static std::string Error_String[ERROR_COUNT] =
{
	"Error ("+to_string(FILE_MISSING)+"): The file % does not exist! ",
	"Error ("+to_string(DIRECTORY_MISSING)+"): The directory % does not exist! ",
	"Error ("+to_string(FILE_CORRUPT)+"): The file % is corrupt! ",

	"Error ("+to_string(FBO_INCOMPLETE)+"): A Framebuffer in the % is incomplete. ",
	"Error ("+to_string(SHADER_INCOMPLETE)+"): The Shader file % could not compile. ",
	"Error ("+to_string(PROGRAM_INCOMPLETE)+"): The Shader program % could not compile. ",

	"Error ("+to_string(GLFW_ERROR)+"): GLFW Error % "
};

#endif // MESSAGE_MANAGER