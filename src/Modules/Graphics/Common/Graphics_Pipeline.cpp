#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Engine.h"

/* Rendering Techniques Used */
#include "Modules/Graphics/Geometry/Prop/Prop_Technique.h"
#include "Modules/Graphics/Lighting/Shadow/Shadow_Technique.h"
#include "Modules/Graphics/Lighting/Directional/Directional_Technique.h"
#include "Modules/Graphics/Lighting/Point/Point_Technique.h"
#include "Modules/Graphics/Lighting/Spot/Spot_Technique.h"
#include "Modules/Graphics/Lighting/Reflector/Reflector_Technique.h"
#include "Modules/Graphics/Effects/Skybox.h"
#include "Modules/Graphics/Effects/Radiance_Hints.h"
#include "Modules/Graphics/Effects/SSAO.h"
#include "Modules/Graphics/Effects/SSR.h"
#include "Modules/Graphics/Effects/Join_Reflections.h"
#include "Modules/Graphics/Effects/Bloom.h"
#include "Modules/Graphics/Effects/HDR.h"
#include "Modules/Graphics/Effects/FXAA.h"


Graphics_Pipeline::Graphics_Pipeline(Engine * engine, const std::shared_ptr<CameraBuffer> & clientCamera, const std::shared_ptr<std::vector<std::shared_ptr<CameraBuffer>>> & cameras, ECSSystemList & auxilliarySystems)
	: m_engine(engine)
{
	auto propView = new Prop_Technique(m_engine, cameras, auxilliarySystems);
	auto shadowing = new Shadow_Technique(m_engine, auxilliarySystems);
	auto directionalLighting = new Directional_Technique(m_engine, shadowing->getShadowData(), clientCamera, cameras, auxilliarySystems);
	auto pointLighting = new Point_Technique(m_engine, shadowing->getShadowData(), cameras, auxilliarySystems);
	auto spotLighting = new Spot_Technique(m_engine, shadowing->getShadowData(), cameras, auxilliarySystems);
	auto reflectorLighting = new Reflector_Technique(m_engine, cameras, auxilliarySystems);
	auto radianceHints = new Radiance_Hints(m_engine);
	auto skybox = new Skybox(m_engine);
	auto ssao = new SSAO(m_engine);
	auto ssr = new SSR(m_engine);
	auto joinReflections = new Join_Reflections(m_engine);
	auto bloom = new Bloom(m_engine);
	auto hdr = new HDR(m_engine);
	auto fxaa = new FXAA(m_engine);

	m_geometryTechniques = {
		propView
	};
	m_lightingTechniques = {
		shadowing, directionalLighting, pointLighting, spotLighting, reflectorLighting
	};
	m_effectTechniques = {
		radianceHints, skybox, ssao, ssr, joinReflections, bloom, hdr, fxaa
	};

	// Join All
	m_allTechniques.reserve(m_geometryTechniques.size() + m_lightingTechniques.size() + m_effectTechniques.size());
	for each (const auto & tech in m_geometryTechniques)
		m_allTechniques.push_back(tech);
	for each (const auto & tech in m_lightingTechniques)
		m_allTechniques.push_back(tech);
	for each (const auto & tech in m_effectTechniques)
		m_allTechniques.push_back(tech);
}

void Graphics_Pipeline::prepareForNextFrame(const float & deltaTime)
{
	for each (auto * tech in m_allTechniques)
		tech->prepareForNextFrame(deltaTime);
}

void Graphics_Pipeline::update(const float & deltaTime)
{
	for each (auto * tech in m_allTechniques)
		tech->updateTechnique(deltaTime);
}

void Graphics_Pipeline::render(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::shared_ptr<CameraBuffer> & camera, const unsigned int & allowedCategories)
{
	for each (auto * tech in m_allTechniques)
		if (allowedCategories & tech->getCategory())
			tech->renderTechnique(deltaTime, viewport, camera);	
}

void Graphics_Pipeline::cullShadows(const float & deltaTime, const std::vector<std::pair<std::shared_ptr<CameraBuffer>, int>>& perspectives)
{
	for each (auto * tech in m_geometryTechniques)
		tech->cullShadows(deltaTime, perspectives);
}

void Graphics_Pipeline::renderShadows(const float & deltaTime)
{
	for each (auto * tech in m_geometryTechniques)
		tech->renderShadows(deltaTime);
}
