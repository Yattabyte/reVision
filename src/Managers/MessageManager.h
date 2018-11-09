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
	~MessageManager() = default;
	/** Construct the message manager*/
	MessageManager() = default;


	// Public Methods
	/** Prints a raw std::string into the message log.
	@param	input				std::string message to print */
	void statement(const std::string & input);
	/** Prints a formatted message based around the supplied error type.
	@param	input				the error message to be displayed. */
	void error(const std::string & input);


private:
	// Private Methods
	/** A helper function that writes the message to a log and to the console. */
	void textOutput(const std::string & output);


	// Private Attributes
	std::shared_mutex m_mutex;
	std::deque<std::string> m_messageLog;
};

#endif // MESSAGEMANAGER_H