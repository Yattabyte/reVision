#include "Modules/Graphics/Common/Graphics_Technique.h"


Graphics_Technique::Graphics_Technique(const Technique_Category& category) noexcept
	: m_category(category)
{
}

Graphics_Technique::Technique_Category Graphics_Technique::getCategory() const noexcept
{
	return m_category;
}

void Graphics_Technique::setEnabled(const bool& state) noexcept
{
	m_enabled = state;
}

void Graphics_Technique::clearCache(const float& /*unused*/)
{
}

void Graphics_Technique::updateCache(const float& /*unused*/, ecsWorld& /*unused*/)
{
}

void Graphics_Technique::updatePass(const float& /*unused*/)
{
}

void Graphics_Technique::renderTechnique(const float&  /*unused*/, Viewport& /*unused*/, const std::vector<std::pair<int, int>>& /*unused*/)
{
}