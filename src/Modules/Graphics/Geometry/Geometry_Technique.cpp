#include "Modules/Graphics/Geometry/Geometry_Technique.h"


Geometry_Technique::Geometry_Technique() noexcept : 
	Graphics_Technique(Technique_Category::GEOMETRY) 
{
}

void Geometry_Technique::renderTechnique(const float&, Viewport&, const std::vector<std::pair<int, int>>&)
{
}

void Geometry_Technique::cullShadows(const float&, const std::vector<std::pair<int, int>>&) 
{
}

void Geometry_Technique::renderShadows(const float&) 
{
}