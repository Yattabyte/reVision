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

Model_Technique::Model_Technique(EnginePackage * enginePackage, Geometry_FBO * geometryFBO, Shadow_FBO * shadowFBO)
{
	m_enginePackage = enginePackage;
	m_geometryFBO = geometryFBO;
	m_shadowFBO = shadowFBO;

	m_updateQuality = m_enginePackage->addPrefCallback(PreferenceState::C_SHADOW_QUALITY, this, [&](const float &f) {m_updateQuality = f; });

	Asset_Loader::load_asset(m_shaderGeometry, "Geometry\\geometry");
	Asset_Loader::load_asset(m_shaderDirectional_Shadow, "Geometry\\geometryShadowDir");
	Asset_Loader::load_asset(m_shaderPoint_Shadow, "Geometry\\geometryShadowPoint");
	Asset_Loader::load_asset(m_shaderSpot_Shadow, "Geometry\\geometryShadowSpot");
}

void Model_Technique::updateData(const Visibility_Token & vis_token)
{
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
		for each (const auto &component in vis_token.getTypeList<Anim_Model_Component>("Anim_Model")) {
			geoArray[count] = component->getBufferIndex();
			const ivec2 drawInfo = component->getDrawInfo();
			drawData[count++] = DrawData(drawInfo.y, drawInfo.x);
		}
		m_visGeoUBO.write(0, sizeof(GLuint) * geoArray.size(), geoArray.data());
		m_indirectGeo.write(0, sizeof(DrawData) * m_size, drawData.data());
	}
}

void Model_Technique::renderGeometry(const Visibility_Token & vis_token)
{
	const size_t size = vis_token.specificSize("Anim_Model");
	if (size) {
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		m_shaderGeometry->bind();
		m_geometryFBO->bindForWriting();
		m_visGeoUBO.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_indirectGeo.bindBuffer(GL_DRAW_INDIRECT_BUFFER);

		glBindVertexArray(Asset_Manager::Get_Model_Manager()->getVAO());
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, size, 0);

		m_geometryFBO->applyAO();
	}
}

void Model_Technique::renderShadows(const Visibility_Token & vis_token)
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
				if (outList.size() > m_quality)
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

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_BACK);

	m_shadowFBO->bindForWriting(SHADOW_LARGE);
	m_shaderDirectional_Shadow->bind();
	for each (auto &component in queueDir.toList())
		component->shadowPass();

	m_shadowFBO->bindForWriting(SHADOW_REGULAR);
	m_shaderPoint_Shadow->bind();
	for each (auto &component in queuePoint.toList())
		component->shadowPass();

	m_shaderSpot_Shadow->bind();
	for each (auto &component in queueSpot.toList())
		component->shadowPass();
}
