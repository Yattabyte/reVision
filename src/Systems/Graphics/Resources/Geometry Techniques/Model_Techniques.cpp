#include "Systems\Graphics\Resources\Geometry Techniques\Model_Techniques.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Shadow_FBO.h"
#include "Systems\World\ECS\Components\Anim_Model_Component.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"


Model_Technique::~Model_Technique()
{
}

Model_Technique::Model_Technique(
	Geometry_FBO * geometryFBO, Shadow_FBO * shadowFBO,
	VectorBuffer<Directional_Struct> * lightDirSSBO, VectorBuffer<Point_Struct> *lightPointSSBO, VectorBuffer<Spot_Struct> *lightSpotSSBO
) {
	// FBO's
	m_geometryFBO = geometryFBO;
	m_shadowFBO = shadowFBO;

	// SSBO's
	m_lightDirSSBO = lightDirSSBO;
	m_lightPointSSBO = lightPointSSBO;
	m_lightSpotSSBO = lightSpotSSBO;
	
	// Asset Loading
	Asset_Loader::load_asset(m_shaderCull, "Geometry\\culling");
	Asset_Loader::load_asset(m_shaderGeometry, "Geometry\\geometry");

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

void Model_Technique::applyPrePass(const Visibility_Token & vis_token)
{
	if (vis_token.specificSize("Anim_Model") && m_cubeVAOLoaded) {
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
		m_indirectGeo.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6); // Read from this buffer
		m_indirectGeo2.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7); // Write visible elements into this buffer
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Undo state changes
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glPolygonOffset(0, 0);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

void Model_Technique::renderGeometry(const Visibility_Token & vis_token)
{
	const size_t size = vis_token.specificSize("Anim_Model");
	if (size && m_cubeVAOLoaded) {
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		m_geometryFBO->clear();
		m_geometryFBO->bindForWriting();
		m_visGeoUBO.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);

		// Render only the objects that passed the previous depth test (modified indirect draw buffer)
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
		glBindVertexArray(Asset_Manager::Get_Model_Manager()->getVAO());
		m_shaderGeometry->bind();
		m_indirectGeo2.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, size, 0);
		m_geometryFBO->applyAO();
	}
}