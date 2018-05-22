#include "Systems\Graphics\Resources\Geometry Techniques\Model_Techniques.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\World\ECS\Components\Anim_Model_Component.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Systems\World\Camera.h"


Model_Technique::~Model_Technique()
{
}

Model_Technique::Model_Technique(Geometry_FBO * geometryFBO) 
{
	// FBO's
	m_geometryFBO = geometryFBO;
	
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
		m_geometryFBO->bindDepthWriting();

		// Render bounding boxes for all models using last frame's depth buffer
		glBindVertexArray(m_cubeVAO);
		m_shaderCull->bind();
		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		// Draw bounding boxes for each camera
		// Main geometry UBO bound to '5', the buffer below indexes into that
		camera.getVisibleIndexBuffer().bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		camera.getCullingBuffer().bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		camera.getRenderBuffer().bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, size, 0);

		// Undo state changes
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
}

void Model_Technique::writeCameraBuffers(Camera & camera, const unsigned int & instanceCount)
{
	const Visibility_Token vis_token = camera.getVisibilityToken();
	const size_t size = vis_token.specificSize("Anim_Model");
	if (size) {
		// Draw parameter order = { COUNT, INSTANCE_COUNT, FIRST, BASE_INSTANCE }
		vector<ivec4> cullingDrawData(size);
		vector<ivec4> renderingDrawData(size);
		vector<uint> visibleIndices(size);

		unsigned int count = 0;
		for each (const auto &component in vis_token.getTypeList<Anim_Model_Component>("Anim_Model")) {
			const ivec2 drawInfo = component->getDrawInfo();
			visibleIndices[count] = component->getBufferIndex();
			// Check mesh complexity and if viewer not within BSphere
			if ((component->getMeshSize() >= 100) && !(component->containsPoint(camera.getPosition()))) { // Allow
				cullingDrawData[count] = ivec4(36, instanceCount, 0, 1);
				renderingDrawData[count++] = ivec4(drawInfo.y, 0, drawInfo.x, 1);
			}
			else { // Skip				
				cullingDrawData[count] = ivec4(36, 0, 0, 1);
				renderingDrawData[count++] = ivec4(drawInfo.y, instanceCount, drawInfo.x, 1);
			}
		}

		camera.getVisibleIndexBuffer().write(0, sizeof(uint) * size, visibleIndices.data());
		camera.getCullingBuffer().write(0, sizeof(ivec4) * size, cullingDrawData.data());
		camera.getRenderBuffer().write(0, sizeof(ivec4) * size, renderingDrawData.data());
	}
}
