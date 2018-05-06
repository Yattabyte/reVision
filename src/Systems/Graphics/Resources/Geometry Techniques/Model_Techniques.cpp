#include "Systems\Graphics\Resources\Geometry Techniques\Model_Techniques.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Shadow_FBO.h"
#include "Systems\World\ECS\Components\Anim_Model_Component.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Systems\World\Camera.h"


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
	});
}

void Model_Technique::updateData(const vector<Camera*> & viewers)
{
	

}

void Model_Technique::renderGeometry(Camera & camera)
{
	const size_t size = camera.getVisibilityToken().specificSize("Anim_Model");
	if (size && m_shaderGeometry->existsYet()) {
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		m_geometryFBO->clear();
		m_geometryFBO->bindForWriting();
		m_shaderGeometry->bind();
		glBindVertexArray(Asset_Manager::Get_Model_Manager()->getVAO());

		// Render only the objects that passed the previous depth test (modified indirect draw buffer)      
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
		camera.getVisibleIndexBuffer().bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		camera.getRenderBuffer().bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, size, 0);
		m_geometryFBO->applyAO();
	}
}

void Model_Technique::occlusionCullBuffers(Camera & camera)
{
	const size_t size = camera.getVisibilityToken().specificSize("Anim_Model");
	if (m_shaderCull->existsYet() && m_cubeVAOLoaded && size) {
		// Set up state
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL);
		m_geometryFBO->bindForWriting();

		// Render bounding boxes for all models using last frame's depth buffer
		glBindVertexArray(m_cubeVAO);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-1, -1);
		m_shaderCull->bind();
		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		// Draw bounding boxes for each camera
		// Main geometry UBO bound to '5', the buffer below indexes into that
		camera.getVisibleIndexBuffer().bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		
		/* Copy draw parameters for visible fragments */
		// Write to this buffer
		camera.getRenderBuffer().bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
		// Draw 'size' number of bounding boxes
		glDrawArraysInstanced(GL_TRIANGLES, 0, 36, size);
		

		// Undo state changes
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glPolygonOffset(0, 0);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

void Model_Technique::writeCameraBuffers(Camera & camera)
{
	struct DrawData {
		GLuint count;
		GLuint instanceCount = 1;
		GLuint first;
		GLuint baseInstance = 1;
		DrawData(const GLuint & c = 0, const GLuint & f = 0, const GLuint & i = 1) : count(c), instanceCount(i), first(f) {}
	};

	const Visibility_Token vis_token = camera.getVisibilityToken();
	const size_t size = vis_token.specificSize("Anim_Model");
	if (size) {
		vector<uint> geoArray(size);
		vector<DrawData> emptyDrawData(size);

		unsigned int count = 0;
		for each (const auto &component in vis_token.getTypeList<Anim_Model_Component>("Anim_Model")) {
			geoArray[count] = component->getBufferIndex();
			const ivec2 drawInfo = component->getDrawInfo();
			emptyDrawData[count++] = DrawData(drawInfo.y, drawInfo.x, 0);
		}

		camera.getVisibleIndexBuffer().write(0, sizeof(GLuint) * geoArray.size(), geoArray.data());
		camera.getRenderBuffer().write(0, sizeof(DrawData) * size, emptyDrawData.data());
	}
}
