#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Engine.h"


Graphics_Pipeline::Graphics_Pipeline(Engine * engine, const std::vector<Graphics_Technique*> techniques)
	: m_engine(engine), m_techniques(techniques)
{
}

void Graphics_Pipeline::render(const float & deltaTime, const std::shared_ptr<CameraBuffer> & cameraBuffer, const std::shared_ptr<Graphics_Framebuffers> & gfxFBOS, const std::shared_ptr<RH_Volume> & rhVolume)
{
	auto & world = m_engine->getModule_World();
	for each (auto * tech in m_techniques) {
		// Provide rendering variables
		tech->setViewingParameters(cameraBuffer, gfxFBOS, rhVolume);
		// Update With Components
		world.updateSystem(tech, deltaTime);
		// Render Technique
		tech->applyTechnique(deltaTime);
	}
}
