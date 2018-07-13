#pragma once
#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include <string>
#include <shared_mutex>
#include <deque>


/**
 * Provides some message reporting functionality for the engine.
 * Holds a log of text in case they need to be accessed by any external UI
 **/
class MessageManager
{
public:
	// (de)Constructors
	/** Destroy the message manager. */
	~MessageManager();
	/** Construct the message manager*/
	MessageManager();


	// Public Methods
	/** Prints a raw std::string into the message log.
	 * @param	input	std::string message to print */
	void statement(const std::string & input);

	/** Prints a formatted message based around the supplied error type.
	 * @param	error_number		the error type to format this error message around
	 * @param	input				the error message to be formatted
	 * @param	additional_input	an optional additional message to supplement the original */
	void error(const int & error_number, const std::string & input, const std::string & additional_input = "");


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
	const std::string Error_String[ERROR_COUNT] =
	{
		"Error (" + std::to_string(FILE_MISSING) + "): The file % does not exist! ",
		"Error (" + std::to_string(DIRECTORY_MISSING) + "): The directory % does not exist! ",
		"Error (" + std::to_string(FILE_CORRUPT) + "): The file % is corrupt! ",

		"Error (" + std::to_string(FBO_INCOMPLETE) + "): A Framebuffer in the % is incomplete. ",
		"Error (" + std::to_string(SHADER_INCOMPLETE) + "): The Shader file % could not compile. ",
		"Error (" + std::to_string(PROGRAM_INCOMPLETE) + "): The Shader program % could not compile. ",

		"Error (" + std::to_string(GLFW_ERROR) + "): GLFW Error: % ",
		"Error (" + std::to_string(OPENGL_ERROR) + "): OpenGL Error: % ",
		"Error (" + std::to_string(OTHER_ERROR) + "): % ",
	};


private:
	// Private Methods
	/** A helper function that writes the message to a log and to the console. */
	void textOutput(const std::string & output);


	// Private Attributes
	std::shared_mutex message_log_mutex;
	std::deque<std::string> message_log;
};

#endif // MESSAGEMANAGER_H