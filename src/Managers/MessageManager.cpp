#include "Managers\MessageManager.h"
#include <iostream>


MessageManager::~MessageManager()
{
}

MessageManager::MessageManager()
{
}

void MessageManager::statement(const string & input)
{
	textOutput(input);
}

void MessageManager::error(const int & error_number, const string & input, const string & additional_input)
{
	string error_message = Error_String[error_number];
	const int & spot = error_message.find('%');
	error_message.erase(error_message.begin() + spot, error_message.begin() + spot + 1);
	error_message.insert(spot, input);
	textOutput(error_message + additional_input);
}

void MessageManager::textOutput(const string & output)
{
	unique_lock<shared_mutex> write_guard(message_log_mutex);
	message_log.push_back(output);
	cout << output + "\n";
}
