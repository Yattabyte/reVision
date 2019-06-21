#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Engine.h"

/* Rendering Techniques Used */
#include "Modules/Graphics/Geometry/Prop_View.h"
#include "Modules/Graphics/Lighting/Directional_Lighting.h"
#include "Modules/Graphics/Lighting/Point_Lighting.h"
#include "Modules/Graphics/Lighting/Spot_Lighting.h"
#include "Modules/Graphics/Lighting/Reflector_Lighting.h"
#include "Modules/Graphics/Effects/Skybox.h"
#include "Modules/Graphics/Effects/Radiance_Hints.h"
#include "Modules/Graphics/Effects/SSAO.h"
#include "Modules/Graphics/Effects/SSR.h"
#include "Modules/Graphics/Effects/Join_Reflections.h"
#include "Modules/Graphics/Effects/Bloom.h"
#include "Modules/Graphics/Effects/HDR.h"
#include "Modules/Graphics/Effects/FXAA.h"
#include "Modules/Graphics/Effects/To_Screen.h"


Graphics_Pipeline::Graphics_Pipeline(Engine * engine)
	: m_engine(engine)
{
	auto propView = new Prop_View(m_engine);
	m_techniques = {
		propView,
		new Directional_Lighting(m_engine, propView),
		new Point_Lighting(m_engine, propView),
		new Spot_Lighting(m_engine, propView),
		new Reflector_Lighting(m_engine),
		new Radiance_Hints(m_engine),
		new Skybox(m_engine),
		new SSAO(m_engine),
		new SSR(m_engine),
		new Join_Reflections(m_engine),
		new Bloom(m_engine),
		new HDR(m_engine),
		new FXAA(m_engine),
		new To_Screen(m_engine)
	};
}

void Graphics_Pipeline::beginFrame(const float & deltaTime)
{
	for each (auto * tech in m_techniques)
		tech->beginFrame(deltaTime);
}

void Graphics_Pipeline::endFrame(const float & deltaTime)
{
	for each (auto * tech in m_techniques)
		tech->endFrame(deltaTime);
}

void Graphics_Pipeline::render(const float & deltaTime, const std::shared_ptr<CameraBuffer> & cameraBuffer, const std::shared_ptr<Graphics_Framebuffers> & gfxFBOS, const std::shared_ptr<RH_Volume> & rhVolume, const unsigned int & allowedCategories)
{
	auto & world = m_engine->getModule_World();
	for each (auto * tech in m_techniques) {
		if (allowedCategories & tech->getCategory()) {
			// Provide rendering variables
			tech->setViewingParameters(cameraBuffer, gfxFBOS, rhVolume);
			// Render Technique
			tech->renderTechnique(deltaTime);
		}
	}
}