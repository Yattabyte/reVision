#pragma once
#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include <deque>
#include <shared_mutex>
#include <string>


/** Provides some message reporting functionality for the engine.
Holds a log of text in case they need to be accessed by any external UI */
class MessageManager {
public:

	// Public Methods
	/** Prints a general statement into the console.
	@param	input	std::string message to print. */
	void statement(const std::string& input);
	/** Prints a warning message into the console.
	@param	input	std::string message to print. */
	void warning(const std::string& input);
	/** Prints an error message into the console.
	@param	input	the error message to be displayed. */
	void error(const std::string& input);


private:
	// Private Methods
	/** A helper function that writes the message to a log and to the console. 
	@param	output	the text to write to the log. */
	void textOutput(const std::string& output);


	// Private Attributes
	std::shared_mutex m_mutex;
	std::deque<std::string> m_messageLog;
};

#endif // MESSAGEMANAGER_H