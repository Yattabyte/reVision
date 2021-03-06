#include "Modules/Graphics/Geometry/Geometry_Technique.h"


Geometry_Technique::Geometry_Technique() noexcept :
	Graphics_Technique(Technique_Category::GEOMETRY)
{
}

void Geometry_Technique::renderTechnique(const float& /*deltaTime*/, Viewport& /*viewport*/, const std::vector<std::pair<int, int>>& /*perspectives*/)
{
}

void Geometry_Technique::cullShadows(const float& /*unused*/, const std::vector<std::pair<int, int>>& /*unused*/)
{
}

void Geometry_Technique::renderShadows(const float& /*unused*/)
{
}