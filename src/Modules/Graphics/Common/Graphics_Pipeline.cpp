#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Engine.h"

/* Rendering Techniques Used */
#include "Modules/Graphics/Geometry/Prop_Renderer.h"
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
	auto directionalLighting = new Directional_Lighting(m_engine, propView);
	auto pointLighting = new Point_Lighting(m_engine, propView);
	auto spotLighting = new Spot_Lighting(m_engine, propView);
	auto reflectorLighting = new Reflector_Lighting(m_engine);
	auto radianceHints = new Radiance_Hints(m_engine);
	auto skybox = new Skybox(m_engine);
	auto ssao = new SSAO(m_engine);
	auto ssr = new SSR(m_engine);
	auto joinReflections = new Join_Reflections(m_engine);
	auto bloom = new Bloom(m_engine);
	auto hdr = new HDR(m_engine);
	auto fxaa = new FXAA(m_engine);
	auto toScreen = new To_Screen(m_engine);

	// Join all techniques into a single list
	m_techniques = {
		propView, directionalLighting,	pointLighting, spotLighting,
		reflectorLighting, radianceHints, skybox, ssao, ssr,
		joinReflections, bloom, hdr, fxaa, toScreen
	};

	// Each graphics technique is also an "ECS System"
	auto & gfx = m_engine->getModule_Graphics();
	for each (const auto & tech in m_techniques) 
		gfx.addPerViewportSystem(tech);	
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

void Graphics_Pipeline::update(const float & deltaTime)
{
	for each (auto * tech in m_techniques)
		tech->updateTechnique(deltaTime);
}

void Graphics_Pipeline::render(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const unsigned int & allowedCategories)
{
	for each (auto * tech in m_techniques) {
		if (allowedCategories & tech->getCategory())
			tech->renderTechnique(deltaTime, viewport);
	}
}