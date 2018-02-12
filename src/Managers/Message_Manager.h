#pragma once
#ifndef MESSAGE_MANAGER
#define MESSAGE_MANAGER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include <string>

using namespace std;


/** 
 * Provides some message reporting functionality for the engine. 
 * Is a static namespace class for now.
 * Holds a log of text in case they need to be access by any external UI
 * @todo Make into an object passed into the engine and stored in the package. Maybe a system even.
 * @todo can rework the function names later.
 **/
namespace MSG {
	/** Prints a raw string into the message log.
	 * @param	input	string message to print */
	DT_ENGINE_API void Statement(const string & input);

	/** Prints a formatted message based around the supplied error type.
	 * @param	error_number	the error type to format this error message around
	 * @param	input	the error message to be formatted
	 * @param	additional_input	an optional additional message to supplement the original */
	DT_ENGINE_API void Error(const int & error_number, const string & input, const std::string & additional_input = "");
}

// Enumerations used for reporting error types
enum Error_Enum
{
	FILE_MISSING,
	DIRECTORY_MISSING,
	FILE_CORRUPT,

	FBO_INCOMPLETE,
	SHADER_INCOMPLETE,
	PROGRAM_INCOMPLETE,

	GLFW_ERROR,
	OPENGL_ERROR,
	OTHER_ERROR,
	ERROR_COUNT,
};

// Stringified versions of the previous error enumerations
static std::string Error_String[ERROR_COUNT] =
{
	"Error ("+to_string(FILE_MISSING)+"): The file % does not exist! ",
	"Error ("+to_string(DIRECTORY_MISSING)+"): The directory % does not exist! ",
	"Error ("+to_string(FILE_CORRUPT)+"): The file % is corrupt! ",

	"Error ("+to_string(FBO_INCOMPLETE)+"): A Framebuffer in the % is incomplete. ",
	"Error ("+to_string(SHADER_INCOMPLETE)+"): The Shader file % could not compile. ",
	"Error ("+to_string(PROGRAM_INCOMPLETE)+"): The Shader program % could not compile. ",

	"Error ("+to_string(GLFW_ERROR)+"): GLFW Error: % ",
	"Error ("+to_string(OPENGL_ERROR)+"): OpenGL Error: % ",
	"Error ("+to_string(OTHER_ERROR)+"): % ",
};

#endif // MESSAGE_MANAGER