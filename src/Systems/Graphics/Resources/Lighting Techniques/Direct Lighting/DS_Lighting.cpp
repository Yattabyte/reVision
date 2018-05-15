#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\DS_Lighting.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Lighting_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Shadow_FBO.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Utilities\EnginePackage.h"

// Begin includes for specific lighting techniques
#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\Types\Directional.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\Types\Directional_Cheap.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\Types\Spot.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\Types\Point.h"
// End includes for specific lighting techniques

DS_Lighting::~DS_Lighting()
{
	if (m_shapeCube.get()) m_shapeCube->removeCallback(this);

	m_enginePackage->removePrefCallback(PreferenceState::C_SHADOW_QUALITY, this);
}

DS_Lighting::DS_Lighting(
	EnginePackage * enginePackage,
	Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Shadow_FBO *shadowFBO,
	Light_Buffers * lightBuffers
)
{
	m_enginePackage = enginePackage;
	m_updateQuality = m_enginePackage->addPrefCallback(PreferenceState::C_SHADOW_QUALITY, this, [&](const float &f) {m_updateQuality = f; });

	// FBO's
	m_geometryFBO = geometryFBO;
	m_lightingFBO = lightingFBO;
	m_shadowFBO = shadowFBO;

	// Load Assets
	Asset_Loader::load_asset(m_shapeCube, "box");
	m_cubeVAOLoaded = false;
	m_cubeVAO = Asset_Primitive::Generate_VAO();

	// Primitive Loading
	m_shapeCube->addCallback(this, [&]() {
		m_shapeCube->updateVAO(m_cubeVAO);
		m_cubeVAOLoaded = true;
	});

	m_techniques.push_back(new Directional_Tech(shadowFBO, lightBuffers));
	m_techniques.push_back(new Directional_Tech_Cheap(lightBuffers));
	m_techniques.push_back(new Spot_Tech(shadowFBO, lightBuffers));
	m_techniques.push_back(new Point_Tech(shadowFBO, lightBuffers));
}

void DS_Lighting::updateData(const Visibility_Token & vis_token)
{
	// Quit early if we don't have models
	if (!vis_token.find("Anim_Model")) return;

	for each (auto technique in m_techniques)
		technique->updateData(vis_token, m_updateQuality, m_enginePackage->m_Camera.getCameraBuffer().EyePosition);
}

void DS_Lighting::applyPrePass(const Visibility_Token & vis_token)
{ 
	/******************** 
	  Occlusion Culling 
	********************/
	if (m_cubeVAOLoaded) {
		// Set up state
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		// Draw model bounding boxes
		glBindVertexArray(m_cubeVAO);

		for each (auto technique in m_techniques)
			technique->renderOcclusionCulling();

		// Undo state changes
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
	}
	

	/******************** 
		Render Shadows 
	********************/
	glDisable(GL_BLEND);
	glCullFace(GL_BACK);

	// Bind Geometry VAO once
	glBindVertexArray(Asset_Manager::Get_Model_Manager()->getVAO());

	for each (auto technique in m_techniques)
		technique->renderShadows();	
}

void DS_Lighting::applyLighting(const Visibility_Token & vis_token)
{
	// Setup common state for all lights
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	m_geometryFBO->bindForReading();
	m_lightingFBO->bindForWriting();

	for each (auto technique in m_techniques)
		technique->renderLighting();

	// Revert State
	glCullFace(GL_BACK);
	glDisable(GL_STENCIL_TEST);
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);
}
