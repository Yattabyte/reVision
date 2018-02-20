#include "Managers\Message_Manager.h"
#include <iostream>


void MSG_Manager::Statement(const string & input)
{
	Get().text_output(input);
}

void MSG_Manager::Error(const int & error_number, const string & input, const string & additional_input)
{
	string error_message = Get().Error_String[error_number];
	const int &spot = error_message.find('%');
	error_message.erase(error_message.begin() + spot, error_message.begin() + spot + 1);
	error_message.insert(spot, input);
	Get().text_output(error_message + additional_input);
}

void MSG_Manager::text_output(const string & output)
{
	unique_lock<shared_mutex> write_guard(message_log_mutex);
	message_log.push_back(output);
	cout << output + "\n";
}
