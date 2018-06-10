#pragma once
#ifndef MESSAGE_MANAGER_H
#define MESSAGE_MANAGER_H

#include <string>
#include <shared_mutex>
#include <deque>

using namespace std;


/**
* Provides some message reporting functionality for the engine.
* Holds a log of text in case they need to be access by any external UI
**/
class MSG_Manager
{
public:
	// Public Methods
	/** Singleton GET method.
	 * @return	static Material_Manager instance */
	static MSG_Manager &Get() {
		static MSG_Manager instance;
		return instance;
	}
	/** Prints a raw string into the message log.
	 * @param	input	string message to print */
	static void Statement(const string & input);

	/** Prints a formatted message based around the supplied error type.
	 * @param	error_number		the error type to format this error message around
	 * @param	input				the error message to be formatted
	 * @param	additional_input	an optional additional message to supplement the original */
	static void Error(const int & error_number, const string & input, const string & additional_input = "");


	// Public Attributes
	// Enumerations used for reporting error types
	static const enum Error_Enum
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
	// String-ified versions of the previous error enumerations
	const string Error_String[ERROR_COUNT] =
	{
		"Error (" + to_string(FILE_MISSING) + "): The file % does not exist! ",
		"Error (" + to_string(DIRECTORY_MISSING) + "): The directory % does not exist! ",
		"Error (" + to_string(FILE_CORRUPT) + "): The file % is corrupt! ",

		"Error (" + to_string(FBO_INCOMPLETE) + "): A Framebuffer in the % is incomplete. ",
		"Error (" + to_string(SHADER_INCOMPLETE) + "): The Shader file % could not compile. ",
		"Error (" + to_string(PROGRAM_INCOMPLETE) + "): The Shader program % could not compile. ",

		"Error (" + to_string(GLFW_ERROR) + "): GLFW Error: % ",
		"Error (" + to_string(OPENGL_ERROR) + "): OpenGL Error: % ",
		"Error (" + to_string(OTHER_ERROR) + "): % ",
	};

private:
	// Private Methods
	/** A helper function that writes the message to a log and to the console. */
	void text_output(const string & output);
	
	// Private Attributes
	shared_mutex message_log_mutex;
	deque<string> message_log;
};




#endif // MESSAGE_MANAGER_H