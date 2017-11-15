/*#include "Utilities\VisualFX.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Shader.h"
#include "Systems\Config_Manager.h"

static Shared_Asset_Primitive shape_quad;
static Shared_Asset_Shader CF_SHADER, GB_SHADER, GB_A_SHADER;
static GLuint CF_FBO = 0, GB_FBO = 0;

void InitCubeFilter()
{
	CF_FBO = 0;
	glGenFramebuffers(1, &CF_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, CF_FBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Asset_Manager::load_asset(CF_SHADER, "FX\\cube_filter", false, false);
}

void InitGaussianBlurFilter()
{
	float	WindowWidth = CFG::getPreference(CFG_ENUM::C_WINDOW_WIDTH),
			WindowHeight = CFG::getPreference(CFG_ENUM::C_WINDOW_HEIGHT);	
	GB_FBO = 0;
	glGenFramebuffers(1, &GB_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, GB_FBO);	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Asset_Manager::load_asset(GB_SHADER, "FX\\gaussianblur", false, false);
	Asset_Manager::load_asset(GB_A_SHADER, "FX\\gaussianblur_alpha", false, false);
}

namespace VisualFX {
	void Initialize()
	{
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		Asset_Manager::load_asset(shape_quad, "quad"); 
		InitCubeFilter();
		InitGaussianBlurFilter();
	}
	void APPLY_FX_CUBE_FILTER(const GLuint &sourceTexture, const GLuint & destinationTexture, const float &texture_size)
	{
		if (sourceTexture) {
			glDisable(GL_BLEND);

			CF_SHADER->Bind();

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, CF_FBO);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, sourceTexture);

			shape_quad->Bind();
			const int quad_size = shape_quad->GetSize();
			for (int roughness = 0; roughness < CF_MIP_LODS; ++roughness) {
				const int size = max(1, int(floor(texture_size / pow(2, roughness))));
				glViewport(0, 0, size, size);
				CF_SHADER->setLocationValue(6, (float(roughness) / 5.0f));
				for (int face = 0; face < 6; ++face) {
					glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, destinationTexture, roughness);
					CF_SHADER->setLocationValue(5, face);

					glDrawArrays(GL_TRIANGLES, 0, quad_size);
				}
			}
			CF_SHADER->Release();
			shape_quad->Unbind();
		}
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void APPLY_FX_GAUSSIAN_BLUR(const GLuint & desiredTexture, const GLuint *flipTextures, const vec2 &size, const int &amount)
	{
		if (desiredTexture) {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GB_FBO);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, flipTextures[0], 0);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, flipTextures[1], 0);
			glViewport(0, 0, size.x, size.y);

			// Read from desired texture, blur into this frame buffer
			GLboolean horizontal = false;
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, desiredTexture);
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			GB_SHADER->Bind();
			GB_SHADER->setLocationValue(0, horizontal);

			shape_quad->Bind();
			const int quad_size = shape_quad->GetSize();
			glDrawArrays(GL_TRIANGLES, 0, quad_size);

			// Blur remainder of the times minus 1
			for (int i = 1; i < amount - 1; i++) {
				horizontal = !horizontal;
				glBindTexture(GL_TEXTURE_2D, flipTextures[!horizontal]);
				glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
				GB_SHADER->setLocationValue(0, horizontal);

				glDrawArrays(GL_TRIANGLES, 0, quad_size);
			}

			// Last blur back into desired texture
			horizontal = !horizontal;
			glBindTexture(GL_TEXTURE_2D, flipTextures[!horizontal]);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, desiredTexture, 0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			GB_SHADER->setLocationValue(0, horizontal);

			glDrawArrays(GL_TRIANGLES, 0, quad_size);

			shape_quad->Unbind();
			GB_SHADER->Release();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	void APPLY_FX_GAUSSIAN_BLUR_ALPHA(const GLuint & desiredTexture, const GLuint * flipTextures, const vec2 & size, const int & amount)
	{
		if (desiredTexture) {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GB_FBO);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, flipTextures[0], 0);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, flipTextures[1], 0);
			glViewport(0, 0, size.x, size.y);

			// Read from desired texture, blur into this frame buffer
			GLboolean horizontal = false;
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, desiredTexture);
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
			GB_A_SHADER->Bind();
			GB_A_SHADER->setLocationValue(0, horizontal);

			shape_quad->Bind();
			const int quad_size = shape_quad->GetSize();
			glDrawArrays(GL_TRIANGLES, 0, quad_size);

			// Blur remainder of the times minus 1
			for (int i = 1; i < amount - 1; i++) {
				horizontal = !horizontal;
				glBindTexture(GL_TEXTURE_2D, flipTextures[!horizontal]);
				glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
				GB_A_SHADER->setLocationValue(0, horizontal);

				glDrawArrays(GL_TRIANGLES, 0, quad_size);
			}

			// Last blur back into desired texture
			horizontal = !horizontal;
			glBindTexture(GL_TEXTURE_2D, flipTextures[!horizontal]);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, desiredTexture, 0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			GB_A_SHADER->setLocationValue(0, horizontal);

			glDrawArrays(GL_TRIANGLES, 0, quad_size);

			shape_quad->Unbind();
			GB_A_SHADER->Release();
			glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ONE);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

}*/