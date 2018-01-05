#include "Systems\Graphics\Graphics_PBR.h"
#include "Utilities\Engine_Package.h"
#include "Rendering\Camera.h"
#include "Entities\Components\Geometry_Component.h"
#include "Entities\Components\Lighting_Component.h"
#include <random>


class Primitivee_Observer : Asset_Observer
{
public:
	Primitivee_Observer(Shared_Asset_Primitive &asset, const GLuint vao) : Asset_Observer(asset.get()), m_vao_id(vao), m_asset(asset) {};
	virtual ~Primitivee_Observer() {
		m_asset->RemoveObserver(this); 
	};
	virtual void Notify_Finalized() {
		if (m_asset->ExistsYet()) // in case this gets used more than once by mistake
			m_asset->UpdateVAO(m_vao_id);
	}

	GLuint m_vao_id;
	Shared_Asset_Primitive m_asset;
};
class SSAO_Callback : public Callback_Container {
public:
	~SSAO_Callback() {};
	SSAO_Callback(System_Graphics_PBR *graphics) : m_Graphics(graphics) {}
	void Callback(const float &value) {
		m_Graphics->SetSSAO((bool)value);
	}
private:
	System_Graphics_PBR *m_Graphics;
};
class SSAO_Samples_Callback : public Callback_Container {
public:
	~SSAO_Samples_Callback() {};
	SSAO_Samples_Callback(System_Graphics_PBR *graphics) : m_Graphics(graphics) {}
	void Callback(const float &value) {
		m_Graphics->SetSSAOSamples((int)value);
	}
private:
	System_Graphics_PBR *m_Graphics;
};
class SSAO_Strength_Callback : public Callback_Container {
public:
	~SSAO_Strength_Callback() {};
	SSAO_Strength_Callback(System_Graphics_PBR *graphics) : m_Graphics(graphics) {}
	void Callback(const float &value) {
		m_Graphics->SetSSAOStrength((int)value);
	}
private:
	System_Graphics_PBR *m_Graphics;
};
class SSAO_Radius_Callback : public Callback_Container {
public:
	~SSAO_Radius_Callback() {};
	SSAO_Radius_Callback(System_Graphics_PBR *graphics) : m_Graphics(graphics) {}
	void Callback( const float &value) {
		m_Graphics->SetSSAORadius(value);
	}
private:
	System_Graphics_PBR *m_Graphics;
};
class Bloom_StrengthChangeCallback : public Callback_Container {
public:
	~Bloom_StrengthChangeCallback() {};
	Bloom_StrengthChangeCallback(Lighting_Buffer *lBuffer) : m_LBuffer(lBuffer) {}
	void Callback(const float &value) {
		m_LBuffer->SetBloomStrength((int)value);
	}
private:
	Lighting_Buffer *m_LBuffer;
};
class Cam_WidthChangeCallback : public Callback_Container {
public:
	~Cam_WidthChangeCallback() {};
	Cam_WidthChangeCallback(System_Graphics_PBR *graphics) : m_Graphics(graphics) {}
	void Callback(const float &value) {
		m_Graphics->Resize(vec2(value, m_preferenceState->GetPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT)));
	}
private:
	System_Graphics_PBR *m_Graphics;
};
class Cam_HeightChangeCallback : public Callback_Container {
public:
	~Cam_HeightChangeCallback() {};
	Cam_HeightChangeCallback(System_Graphics_PBR *lBuffer) : m_Graphics(lBuffer) {}
	void Callback(const float &value) {
		m_Graphics->Resize(vec2(m_preferenceState->GetPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH), value));
	}
private:
	System_Graphics_PBR *m_Graphics;
}; 

System_Graphics_PBR::~System_Graphics_PBR()
{
	if (!m_Initialized) {
		m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_SSAO, m_ssaoCallback);
		m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_SSAO_SAMPLES, m_ssaoSamplesCallback);
		m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_SSAO_BLUR_STRENGTH, m_ssaoStrengthCallback);
		m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_SSAO_RADIUS, m_ssaoRadiusCallback);
		m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_WINDOW_HEIGHT, m_bloomStrengthChangeCallback);
		m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_WINDOW_WIDTH, m_widthChangeCallback);
		m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_WINDOW_HEIGHT, m_heightChangeCallback);
		delete m_observer;
		delete m_ssaoCallback;
		delete m_ssaoSamplesCallback;
		delete m_ssaoStrengthCallback;
		delete m_ssaoRadiusCallback;
		delete m_bloomStrengthChangeCallback;
		delete m_widthChangeCallback;
		delete m_heightChangeCallback;
	}
}

System_Graphics_PBR::System_Graphics_PBR() :
	m_visualFX(), m_gbuffer(), m_lbuffer(), m_hdrbuffer()
{
	m_quadVAO = 0;
	m_attribID = 0;
	m_renderSize = vec2(1);
}

void System_Graphics_PBR::Initialize(Engine_Package * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		Asset_Loader::load_asset(m_shaderGeometry, "Geometry\\geometry");
		Asset_Loader::load_asset(m_shaderGeometryShadow, "Geometry\\geometry_shadow");
		Asset_Loader::load_asset(m_shaderLighting, "Lighting\\lighting");
		Asset_Loader::load_asset(m_shaderHDR, "FX\\HDR"); 
		Asset_Loader::load_asset(m_shaderFXAA, "FX\\FXAA"); 
		Asset_Loader::load_asset(m_shapeQuad, "quad");
		Asset_Loader::load_asset(m_shaderSky, "skybox");
		Asset_Loader::load_asset(m_textureSky, "sky\\");
		m_quadVAO = Asset_Primitive::GenerateVAO();
		m_observer = (void*)(new Primitivee_Observer(m_shapeQuad, m_quadVAO));
		m_gbuffer.End();
		
		m_ssaoCallback = new SSAO_Callback(this);
		m_ssaoSamplesCallback = new SSAO_Samples_Callback(this);
		m_ssaoStrengthCallback = new SSAO_Strength_Callback(this);
		m_ssaoRadiusCallback = new SSAO_Radius_Callback(this);
		m_widthChangeCallback = new Cam_WidthChangeCallback(this);
		m_heightChangeCallback = new Cam_HeightChangeCallback(this);
		m_bloomStrengthChangeCallback = new Bloom_StrengthChangeCallback(&m_lbuffer);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_SSAO, m_ssaoCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_SSAO_SAMPLES, m_ssaoSamplesCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_SSAO_BLUR_STRENGTH, m_ssaoStrengthCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_SSAO_RADIUS, m_ssaoRadiusCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_WINDOW_WIDTH, m_widthChangeCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_WINDOW_HEIGHT, m_heightChangeCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_BLOOM_STRENGTH, m_bloomStrengthChangeCallback);
		m_attribs.m_ssao_radius = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_SSAO_RADIUS);
		m_attribs.m_ssao_strength = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_SSAO_BLUR_STRENGTH);
		m_attribs.m_aa_samples = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_SSAO_SAMPLES);
		m_attribs.m_ssao = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_SSAO);
		m_renderSize = vec2(m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH), m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT));
		glGenBuffers(1, &m_attribID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_attribID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_attribs), &m_attribs, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_attribID);
		GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
		memcpy(p, &m_attribs, sizeof(m_attribs));
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		GenerateKernal();

		m_visualFX.Initialize(m_enginePackage);
		m_gbuffer.Initialize(m_renderSize, &m_visualFX);
		m_lbuffer.Initialize(m_renderSize, &m_visualFX, m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_BLOOM_STRENGTH), m_gbuffer.m_depth_stencil);
		m_hdrbuffer.Initialize(m_renderSize);

		m_Initialized = true;
	}
}

inline GLfloat lerp(GLfloat a, GLfloat b, GLfloat f)
{
	return a + f * (b - a);
}

void System_Graphics_PBR::GenerateKernal() 
{
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
	std::default_random_engine generator;
	for (int i = 0, t = 0; i < MAX_KERNEL_SIZE; i++, t++) {
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		GLfloat scale = GLfloat(i) / (GLfloat)(MAX_KERNEL_SIZE);
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		m_attribs.kernel[t] = vec4(sample, 1);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, m_attribID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Renderer_Attribs), &m_attribs);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void System_Graphics_PBR::SetSSAO(const bool & ssao)
{
	m_attribs.m_ssao = ssao;
	glBindBuffer(GL_UNIFORM_BUFFER, m_attribID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Renderer_Attribs), &m_attribs);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void System_Graphics_PBR::SetSSAOSamples(const int & samples)
{
	m_attribs.m_aa_samples = samples;
	glBindBuffer(GL_UNIFORM_BUFFER, m_attribID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Renderer_Attribs), &m_attribs);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void System_Graphics_PBR::SetSSAOStrength(const int & strength)
{
	m_attribs.m_ssao_strength = strength;
	glBindBuffer(GL_UNIFORM_BUFFER, m_attribID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Renderer_Attribs), &m_attribs);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void System_Graphics_PBR::SetSSAORadius(const float & radius)
{
	m_attribs.m_ssao_radius = radius;
	glBindBuffer(GL_UNIFORM_BUFFER, m_attribID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Renderer_Attribs), &m_attribs);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void System_Graphics_PBR::Resize(const vec2 & size)
{
	m_renderSize = size;
	m_gbuffer.Resize(size);
	m_lbuffer.Resize(size);
	m_hdrbuffer.Resize(size);
}

void System_Graphics_PBR::Update(const float & deltaTime)
{
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);

	shared_lock<shared_mutex> read_guard(m_enginePackage->m_Camera.getDataMutex());
	Visibility_Token &vis_token = m_enginePackage->m_Camera.GetVisibilityToken();
	
	if (vis_token.size() && 
		m_shapeQuad->ExistsYet() &&
		m_textureSky->ExistsYet() && 
		m_shaderSky->ExistsYet() &&
		m_shaderGeometry->ExistsYet() &&
		m_shaderLighting->ExistsYet()) 
	{
		RegenerationPass(vis_token);
		GeometryPass(vis_token);
		SkyPass();
		LightingPass(vis_token);
		HDRPass();
		FinalPass();
	}
}

void System_Graphics_PBR::RegenerationPass(const Visibility_Token & vis_token)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_BACK);

	//Shadowmap_Manager::BindForWriting(SHADOW_LARGE);

	/*m_shaderGeometryShadow->Bind();
	if (vis_token.visible_lights.size())
	for each (const auto *light in vis_token.visible_lights.at(Light_Directional::GetLightType()))
	light->shadowPass(vis_token);
	m_shaderGeometryShadow->Release();*/

	//Shadowmap_Manager::BindForWriting(SHADOW_REGULAR);

	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
}

void System_Graphics_PBR::GeometryPass(const Visibility_Token & vis_token)
{
	if (vis_token.find("Anim_Model") != vis_token.end()) {
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		m_gbuffer.Clear();
		m_gbuffer.BindForWriting();
		m_shaderGeometry->Bind();

		for each (auto &component in *((vector<Geometry_Component*>*)(&vis_token.at("Anim_Model"))))
			component->Draw();

		Asset_Shader::Release();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_attribID);
		m_gbuffer.ApplyAO();
	}
}

void System_Graphics_PBR::SkyPass()
{
	const size_t &quad_size = m_shapeQuad->GetSize();
	m_lbuffer.Clear();
	m_lbuffer.BindForWriting();

	glDisable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_TEST);

	m_textureSky->Bind(GL_TEXTURE0);
	m_shaderSky->Bind();
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);
	glBindVertexArray(0);
	Asset_Shader::Release();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
}

void System_Graphics_PBR::LightingPass(const Visibility_Token & vis_token)
{
	const size_t &quad_size = m_shapeQuad->GetSize();
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	m_gbuffer.BindForReading();

	if (vis_token.find("Light_Directional") != vis_token.end()) {
		m_shaderLighting->Bind();
		glBindVertexArray(m_quadVAO);
		for each (auto &component in *((vector<Lighting_Component*>*)(&vis_token.at("Light_Directional"))))
			component->directPass(quad_size);
	}

	Asset_Shader::Release();
	glBindVertexArray(0);
	m_lbuffer.ApplyBloom();
}

void System_Graphics_PBR::HDRPass()
{
	const size_t &quad_size = m_shapeQuad->GetSize();
	m_lbuffer.BindForReading();
	m_hdrbuffer.Clear();
	m_hdrbuffer.BindForWriting();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);
	//glDisable(GL_CULL_FACE);

	m_shaderHDR->Bind();
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);
	glBindVertexArray(0);
	Asset_Shader::Release();
}

void System_Graphics_PBR::FinalPass()
{
	const size_t &quad_size = m_shapeQuad->GetSize();
	m_hdrbuffer.BindForReading();
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	m_shaderFXAA->Bind();
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);
	glBindVertexArray(0);
	Asset_Shader::Release();
	m_gbuffer.End();	
}