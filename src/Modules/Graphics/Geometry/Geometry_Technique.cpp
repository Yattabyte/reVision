#include "Modules/Graphics/Geometry/Geometry_Technique.h"


Geometry_Technique::Geometry_Technique() noexcept : 
	Graphics_Technique(Technique_Category::GEOMETRY) 
{
}

void Geometry_Technique::renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept 
{
	// Forward to geometry rendering
	renderGeometry(deltaTime, viewport, perspectives);
}

void Geometry_Technique::renderGeometry(const float&, const std::shared_ptr<Viewport>&, const std::vector<std::pair<int, int>>&) noexcept 
{
}

void Geometry_Technique::cullShadows(const float&, const std::vector<std::pair<int, int>>&) noexcept 
{
}

void Geometry_Technique::renderShadows(const float&) noexcept 
{
}