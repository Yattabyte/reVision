#pragma once
#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include <string>
#include <shared_mutex>
#include <deque>


/** Provides some message reporting functionality for the engine..
Holds a log of text in case they need to be accessed by any external UI */
class MessageManager {
public:
	// (de)Constructors
	/** Destroy the message manager. */
	~MessageManager();
	/** Construct the message manager*/
	MessageManager();


	// Public Methods
	/** Prints a raw std::string into the message log.
	@param	input				std::string message to print */
	void statement(const std::string & input);
	/** Prints a formatted message based around the supplied error type.
	@param	error_number		the error type to format this error message around
	@param	input				the error message to be formatted
	@param	additional_input	an optional additional message to supplement the original */
	void error(const int & error_number, const std::string & input, const std::string & additional_input = "");


	// Public Attributes
	// Enumerations used for reporting error types
	const enum Error_Enum
	{
		FILE_MISSING,
		FILE_CORRUPT,

		ASSET_FAILED,

		FBO_INCOMPLETE,
		SHADER_INCOMPLETE,
		PROGRAM_INCOMPLETE,
		TEXTURE_INCOMPLETE,
		MATERIAL_INCOMPLETE,

		GLFW_ERROR,
		OPENGL_ERROR,
		MANUAL_ERROR,
		ERROR_COUNT,
	};
	// String-ified versions of the previous error enumerations
	const std::string Error_String[ERROR_COUNT] =
	{
		"The file \"%\" does not exist! ",
		"The file \"%\" is corrupt! ",
				
		"\"%\" initialization failure, attempting to use fallback asset...",

		"A Framebuffer in the \"%\" is incomplete. ",
		"The Shader file \"%\" could not compile. ",
		"The Shader program \"%\" could not compile. ",
		"The Texture object \"%\" is incomplete. ",
		"The Material object \"%\" is incomplete. ",
		
		"GLFW Error: % ",
		"OpenGL Error: % ",
		"% "
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