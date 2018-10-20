#include "Managers\MessageManager.h"
#include <iostream>


void MessageManager::statement(const std::string & input)
{
	textOutput(input);
}

void MessageManager::error(const int & error_number, const std::string & input, const std::string & additional_input)
{
	const std::string error_prefix = "Error (" + std::to_string(error_number) + "): ";
	std::string error_message = Error_String[error_number];
	const size_t spot = error_message.find('%');
	error_message.erase(error_message.begin() + spot, error_message.begin() + spot + 1);
	error_message.insert(spot, input);
	textOutput(error_prefix + error_message + additional_input);
}

void MessageManager::textOutput(const std::string & output)
{
	std::unique_lock<std::shared_mutex> write_guard(m_mutex);
	m_messageLog.push_back(output);
	std::cout << output + "\n";
}
