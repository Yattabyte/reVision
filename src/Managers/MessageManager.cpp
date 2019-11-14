#include "Managers/MessageManager.h"
#include <iostream>


void MessageManager::statement(const std::string& input) noexcept
{
	textOutput(input);
}

void MessageManager::warning(const std::string& input) noexcept
{
	textOutput("Warning: " + input);
}

void MessageManager::error(const std::string& input) noexcept
{
	textOutput("Error: " + input);
}

void MessageManager::textOutput(const std::string& output) noexcept
{
	std::unique_lock<std::shared_mutex> write_guard(m_mutex);
	m_messageLog.push_back(output);
	std::cout << output + "\n";
}