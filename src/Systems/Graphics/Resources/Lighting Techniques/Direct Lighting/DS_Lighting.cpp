#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\DS_Lighting.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Lighting_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Shadow_FBO.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Utilities\EnginePackage.h"
#include "Utilities\PriorityList.h"


DS_Lighting::~DS_Lighting()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
	if (m_shapeCone.get()) m_shapeCone->removeCallback(this);
	if (m_shapeSphere.get()) m_shapeSphere->removeCallback(this);

	m_enginePackage->removePrefCallback(PreferenceState::C_SHADOW_QUALITY, this);
}

DS_Lighting::DS_Lighting(
	EnginePackage * enginePackage,
	Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Shadow_FBO *shadowFBO,
	VectorBuffer<Directional_Struct> * lightDirSSBO, VectorBuffer<Point_Struct> *lightPointSSBO, VectorBuffer<Spot_Struct> *lightSpotSSBO
)
{
	m_enginePackage = enginePackage;
	m_updateQuality = m_enginePackage->addPrefCallback(PreferenceState::C_SHADOW_QUALITY, this, [&](const float &f) {m_updateQuality = f; });

	// FBO's
	m_geometryFBO = geometryFBO;
	m_lightingFBO = lightingFBO;
	m_shadowFBO = shadowFBO;

	// SSBO's
	m_lightDirSSBO = lightDirSSBO;
	m_lightPointSSBO = lightPointSSBO;
	m_lightSpotSSBO = lightSpotSSBO;

	// Load Assets
	Asset_Loader::load_asset(m_shaderDirectional, "Lighting\\Direct Lighting\\directional");
	Asset_Loader::load_asset(m_shaderPoint, "Lighting\\Direct Lighting\\point");
	Asset_Loader::load_asset(m_shaderSpot, "Lighting\\Direct Lighting\\spot");
	Asset_Loader::load_asset(m_shaderDirectional_Shadow, "Geometry\\geometryShadowDir");
	Asset_Loader::load_asset(m_shaderPoint_Shadow, "Geometry\\geometryShadowPoint");
	Asset_Loader::load_asset(m_shaderSpot_Shadow, "Geometry\\geometryShadowSpot");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	Asset_Loader::load_asset(m_shapeCone, "cone");
	Asset_Loader::load_asset(m_shapeSphere, "sphere");
	m_quadVAOLoaded = false; 
	m_coneVAOLoaded = false;
	m_sphereVAOLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_coneVAO = Asset_Primitive::Generate_VAO();
	m_sphereVAO = Asset_Primitive::Generate_VAO();

	// Asset-Loaded Callbacks
	m_shapeQuad->addCallback(this, [&]() { 
		m_shapeQuad->updateVAO(m_quadVAO); 
		m_quadVAOLoaded = true;  
		GLuint data[4] = { m_shapeQuad->getSize(), 0, 0, 0 }; // count, primCount, first, reserved
		m_indirectDir = StaticBuffer(sizeof(GLuint) * 4, data);
	});
	m_shapeCone->addCallback(this, [&]() { 
		m_shapeCone->updateVAO(m_coneVAO); 
		m_coneVAOLoaded = true;  
		GLuint data[4] = { m_shapeCone->getSize(), 0, 0, 0 }; // count, primCount, first, reserved
		m_indirectSpot = StaticBuffer(sizeof(GLuint) * 4, data);
	});
	m_shapeSphere->addCallback(this, [&]() { 
		m_shapeSphere->updateVAO(m_sphereVAO); 
		m_sphereVAOLoaded = true;
		GLuint data[4] = { m_shapeSphere->getSize(), 0, 0, 0 }; // count, primCount, first, reserved
		m_indirectPoint = StaticBuffer(sizeof(GLuint) * 4, data);
	});

	// Draw Buffers
	//m_visDirs = DynamicBuffer(sizeof(GLuint) * 10, 0); We always render ALL directional lights
	m_visPoints = DynamicBuffer(sizeof(GLuint) * 10, 0);
	m_visSpots = DynamicBuffer(sizeof(GLuint) * 10, 0);
}

void DS_Lighting::updateData(const Visibility_Token & vis_token)
{
	/** This is used to prioritize the oldest AND closest lights to the viewer (position) **/
	class PriorityLightList
	{
	public:
		// (de)Constructors
		/** Default destructor. */
		~PriorityLightList() {}
		/** Construct a priority light list with the given quality and position.
		* @param	quality		the max number of final elements
		* @param	position	the position of the viewer */
		PriorityLightList(const unsigned int & quality, const vec3 & position) : m_quality(quality), m_oldest(quality), m_position(position) {}


		// Public Methods
		/** Fill the oldest light list with a new light, and have it sorted.
		* @param	light		the light to insert */
		void insert(Lighting_Component * light) {
			m_oldest.insert(light->getShadowUpdateTime(), light);
		}
		/** Return a list composed of the oldest and the closest lights.
		* @return				a double sorted list with the oldest lights and closest lights */
		const vector<Lighting_Component*> toList() const {
			PriorityList<float, Lighting_Component*, greater<float>> m_closest(m_quality / 2);
			vector<Lighting_Component*> outList;
			outList.reserve(m_quality);

			for each (const auto &element in m_oldest.toList()) {
				if (outList.size() < (m_quality / 2))
					outList.push_back(element);
				else
					m_closest.insert(element->getImportance(m_position), element);
			}

			for each (const auto &element in m_closest.toList()) {
				if (outList.size() >= m_quality)
					break;
				outList.push_back(element);
			}
			return outList;
		}


	private:
		// Private Attributes
		unsigned int m_quality;
		vec3 m_position;
		PriorityList<float, Lighting_Component*, less<float>> m_oldest;
	};

	// Quit early if we don't have models, or we don't have any lights 
	// Logically equivalent to continuing while we have at least 1 model and 1 light
	if ((!vis_token.find("Anim_Model")) ||
		(!vis_token.find("Light_Directional")) &&
		(!vis_token.find("Light_Point")) &&
		(!vis_token.find("Light_Spot")))
		return;

	// Retrieve a sorted list of most important lights to run shadow calc for.
	const vec3 &camPos = m_enginePackage->m_Camera.getCameraBuffer().EyePosition;
	PriorityLightList queueDir(m_updateQuality, camPos), queuePoint(m_updateQuality, camPos), queueSpot(m_updateQuality, camPos);
	for each (const auto &component in vis_token.getTypeList<Lighting_Component>("Light_Directional"))
		queueDir.insert(component);
	for each (const auto &component in vis_token.getTypeList<Lighting_Component>("Light_Point"))
		queuePoint.insert(component);
	for each (const auto &component in vis_token.getTypeList<Lighting_Component>("Light_Spot"))
		queueSpot.insert(component);
	m_queueDir = queueDir.toList();
	m_queuePoint = queuePoint.toList();
	m_queueSpot = queueSpot.toList();
	for each (const auto &c in m_queueDir)
		c->update();
	for each (const auto &c in m_queuePoint)
		c->update();
	for each (const auto &c in m_queueSpot)
		c->update();

	const GLuint dirSize = vis_token.getTypeList<Lighting_Component>("Light_Directional").size();
	const GLuint pointSize = vis_token.getTypeList<Lighting_Component>("Light_Point").size();
	const GLuint spotSize = vis_token.getTypeList<Lighting_Component>("Light_Spot").size();

	if (dirSize && m_quadVAOLoaded)
		m_indirectDir.write(sizeof(GLuint), sizeof(GLuint), &dirSize);
	if (pointSize && m_sphereVAOLoaded) {
		vector<GLuint> visArray(pointSize);
		unsigned int count = 0;
		for each (const auto &component in vis_token.getTypeList<Lighting_Component>("Light_Point"))
			visArray[count++] = component->getBufferIndex();
		m_visPoints.write(0, sizeof(GLuint)*visArray.size(), visArray.data());
		m_indirectPoint.write(sizeof(GLuint), sizeof(GLuint), &pointSize);
	}
	if (spotSize && m_coneVAOLoaded) {
		vector<GLuint> visArray(spotSize);
		unsigned int count = 0;
		for each (const auto &component in vis_token.getTypeList<Lighting_Component>("Light_Spot"))
			visArray[count++] = component->getBufferIndex();
		m_visSpots.write(0, sizeof(GLuint)*visArray.size(), visArray.data());
		m_indirectSpot.write(sizeof(GLuint), sizeof(GLuint), &spotSize);
	}
}

void DS_Lighting::applyPrePass(const Visibility_Token & vis_token)
{ 
	// Render Shadows
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_BACK);

	// Bind Geometry VAO once
	glBindVertexArray(Asset_Manager::Get_Model_Manager()->getVAO());

	// Directional lights
	m_shadowFBO->bindForWriting(SHADOW_LARGE);
	m_shaderDirectional_Shadow->bind();
	m_lightDirSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
	for each (auto &component in m_queueDir)
		component->shadowPass();

	// Point Lights
	m_shadowFBO->bindForWriting(SHADOW_REGULAR);
	m_shaderPoint_Shadow->bind();
	m_lightPointSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
	for each (auto &component in m_queuePoint)
		component->shadowPass();

	// Spot Lights
	m_shaderSpot_Shadow->bind();
	m_lightSpotSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
	for each (auto &component in m_queueSpot)
		component->shadowPass();
}

void DS_Lighting::applyLighting(const Visibility_Token & vis_token)
{
	// Setup common state for all lights
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	m_geometryFBO->bindForReading();
	m_lightingFBO->bindForWriting();

	// Directional Lighting
	if (vis_token.find("Light_Directional") && m_shaderDirectional->existsYet() && m_shapeQuad->existsYet() && m_quadVAOLoaded) {
		m_lightDirSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);	// SSBO light attribute array (directional)
		m_shadowFBO->bindForReading(SHADOW_LARGE, 3);					// Shadow maps (large maps)
		m_shaderDirectional->bind();									// Shader (directional)
		m_indirectDir.bindBuffer(GL_DRAW_INDIRECT_BUFFER);				// Draw call buffer
		glBindVertexArray(m_quadVAO);									// Quad VAO
		glDrawArraysIndirect(GL_TRIANGLES, 0);							// Now draw
	}

	// Setup common state for next 2 lights
	glEnable(GL_STENCIL_TEST);
	glCullFace(GL_FRONT);
	m_shadowFBO->bindForReading(SHADOW_REGULAR, 3);

	// Point Lighting
	if (vis_token.find("Light_Point") && m_shaderPoint->existsYet() && m_shapeSphere->existsYet() && m_sphereVAOLoaded) {
		m_visPoints.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_lightPointSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);	// SSBO light attribute array (points)
		m_shaderPoint->bind();											// Shader (points)
		m_indirectPoint.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_sphereVAO);									// Sphere VAO

		// Draw only into depth-stencil buffer
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_ALWAYS, 0, 0);
		m_shaderPoint->Set_Uniform(0, true);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Now draw into color buffers
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF); 
		m_shaderPoint->Set_Uniform(0, false);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}

	// Spot Lighting
	if (vis_token.find("Light_Spot") && m_shaderSpot->existsYet() && m_shapeCone->existsYet() && m_coneVAOLoaded) {
		m_visSpots.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);			// SSBO visible light indices
		m_lightSpotSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);	// SSBO light attribute array (spots)
		m_shaderSpot->bind();											// Shader (spots)
		m_indirectSpot.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_coneVAO);									// Cone VAO

		// Draw only into depth-stencil buffer
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_ALWAYS, 0, 0);
		m_shaderSpot->Set_Uniform(0, true);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Now draw into color buffers
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
		m_shaderSpot->Set_Uniform(0, false);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}

	// Revert State
	glCullFace(GL_BACK);
	glDisable(GL_STENCIL_TEST);
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);
}
