#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Engine.h"


Graphics_Pipeline::Graphics_Pipeline(Engine * engine, const std::vector<Graphics_Technique*> techniques)
	: m_engine(engine), m_techniques(techniques)
{}

void Graphics_Pipeline::render(const float & deltaTime) 
{
	auto & world = m_engine->getModule_World();
	for each (auto * tech in m_techniques) {
		// Update With Components
		world.updateSystem(tech, deltaTime);
		// Render Technique
		tech->applyEffect(deltaTime);
	}
}
