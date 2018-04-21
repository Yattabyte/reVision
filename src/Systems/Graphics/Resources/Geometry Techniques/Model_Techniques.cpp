#include "Systems\Graphics\Resources\Geometry Techniques\Model_Techniques.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Shadow_FBO.h"
#include "Systems\World\ECS\Components\Anim_Model_Component.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Utilities\EnginePackage.h"
#include "Utilities\PriorityList.h"


Model_Technique::~Model_Technique()
{
}

Model_Technique::Model_Technique(
	EnginePackage * enginePackage, Geometry_FBO * geometryFBO, Shadow_FBO * shadowFBO,
	VectorBuffer<Directional_Struct> * lightDirSSBO, VectorBuffer<Point_Struct> *lightPointSSBO, VectorBuffer<Spot_Struct> *lightSpotSSBO
) {
	m_enginePackage = enginePackage;

	// FBO's
	m_geometryFBO = geometryFBO;
	m_shadowFBO = shadowFBO;

	// SSBO's
	m_lightDirSSBO = lightDirSSBO;
	m_lightPointSSBO = lightPointSSBO;
	m_lightSpotSSBO = lightSpotSSBO;

	m_updateQuality = m_enginePackage->addPrefCallback(PreferenceState::C_SHADOW_QUALITY, this, [&](const float &f) {m_updateQuality = f; });

	// Asset Loading
	Asset_Loader::load_asset(m_shaderCull, "Geometry\\culling");
	Asset_Loader::load_asset(m_shaderGeometry, "Geometry\\geometry");
	Asset_Loader::load_asset(m_shaderDirectional_Shadow, "Geometry\\geometryShadowDir");
	Asset_Loader::load_asset(m_shaderPoint_Shadow, "Geometry\\geometryShadowPoint");
	Asset_Loader::load_asset(m_shaderSpot_Shadow, "Geometry\\geometryShadowSpot");

	// Cube Loading
	Asset_Loader::load_asset(m_shapeCube, "box");
	m_cubeVAOLoaded = false;
	m_cubeVAO = Asset_Primitive::Generate_VAO();
	m_shapeCube->addCallback(this, [&]() {
		m_shapeCube->updateVAO(m_cubeVAO);
		m_cubeVAOLoaded = true;
		GLuint data[4] = { m_shapeCube->getSize(), 0, 0, 0 }; // count, primCount, first, reserved
		m_cubeIndirect = StaticBuffer(sizeof(GLuint) * 4, data);
	});
}

void Model_Technique::updateData(const Visibility_Token & vis_token)
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

	const size_t m_size = vis_token.specificSize("Anim_Model");
	if (m_size) {
		unsigned int count = 0;
		struct DrawData {
			GLuint count;
			GLuint instanceCount = 1;
			GLuint first;
			GLuint baseInstance = 1;
			DrawData(const GLuint & c = 0, const GLuint & f = 0) : count(c), first(f) {}
		};

		vector<uint> geoArray(m_size);
		vector<DrawData> drawData(m_size);
		vector<DrawData> emptyDrawData(m_size);
		for each (const auto &component in vis_token.getTypeList<Anim_Model_Component>("Anim_Model")) {
			geoArray[count] = component->getBufferIndex();
			const ivec2 drawInfo = component->getDrawInfo();
			drawData[count++] = DrawData(drawInfo.y, drawInfo.x);
		}
		m_visGeoUBO.write(0, sizeof(GLuint) * geoArray.size(), geoArray.data());
		m_indirectGeo.write(0, sizeof(DrawData) * m_size, drawData.data());
		m_cubeIndirect.write(sizeof(GLuint), sizeof(GLuint), &m_size);		
		m_indirectGeo2.write(0, sizeof(DrawData) * m_size, emptyDrawData.data());
	}
}

void Model_Technique::renderGeometry(const Visibility_Token & vis_token)
{
	const size_t size = vis_token.specificSize("Anim_Model");
	if (size && m_cubeVAOLoaded) {
		// Set up state
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL);
		m_geometryFBO->bindForWriting();
		m_visGeoUBO.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);

		// Render bounding boxes for all models using last frame's depth buffer
		glBindVertexArray(m_cubeVAO);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-1, -1);
		m_shaderCull->bind();
		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		m_cubeIndirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		m_indirectGeo.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
		m_indirectGeo2.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Allow depth/color writes, clear out gbuffer for new geometry pass
		glDepthMask(GL_TRUE);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		m_geometryFBO->clear();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);   
		glPolygonOffset(0, 0);
		glDisable(GL_POLYGON_OFFSET_FILL);

		// Render only the objects that passed the previous depth test (modified indirect draw buffer)
		glBindVertexArray(Asset_Manager::Get_Model_Manager()->getVAO());
		m_shaderGeometry->bind();
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);
		m_indirectGeo2.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, size, 0);
		m_geometryFBO->applyAO();
	}
}

void Model_Technique::renderShadows(const Visibility_Token & vis_token)
{
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
