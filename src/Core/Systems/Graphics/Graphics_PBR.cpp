#include "Systems\Graphics\Graphics_PBR.h"
#include "Systems\Shadows\Shadowmap.h"
#include "Utilities\Engine_Package.h"
#include "Systems\World\Camera.h"
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
class ShadowQualityChangeCallback : public Callback_Container {
public:
	~ShadowQualityChangeCallback() {};
	ShadowQualityChangeCallback(int *pointer) : m_pointer(pointer) {}
	void Callback(const float &value) {
		*m_pointer = (int)value;
	}
private:
	int *m_pointer;
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
		m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_SHADOW_QUALITY, m_QualityChangeCallback);
		delete m_QuadObserver;
		delete m_ConeObserver;
		delete m_SphereObserver;
		delete m_ssaoCallback;
		delete m_ssaoSamplesCallback;
		delete m_ssaoStrengthCallback;
		delete m_ssaoRadiusCallback;
		delete m_bloomStrengthChangeCallback;
		delete m_widthChangeCallback;
		delete m_heightChangeCallback;
		delete m_QualityChangeCallback;
	}
}

System_Graphics_PBR::System_Graphics_PBR() :
	m_visualFX(), m_gbuffer(), m_lbuffer(), m_hdrbuffer()
{
	m_quadVAO = 0;
	m_coneVAO = 0;
	m_sphereVAO = 0;
	m_attribID = 0;
	m_renderSize = vec2(1);
}

void System_Graphics_PBR::Initialize(Engine_Package * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		Asset_Loader::load_asset(m_shaderGeometry, "Geometry\\geometry");
		Asset_Loader::load_asset(m_shaderShadowDir, "Geometry\\geometryShadowDir");
		Asset_Loader::load_asset(m_shaderShadowPoint, "Geometry\\geometryShadowPoint");
		Asset_Loader::load_asset(m_shaderShadowSpot, "Geometry\\geometryShadowSpot");
		Asset_Loader::load_asset(m_shaderDirectional, "Lighting\\directional");
		Asset_Loader::load_asset(m_shaderPoint, "Lighting\\point");
		Asset_Loader::load_asset(m_shaderSpot, "Lighting\\spot"); 
		Asset_Loader::load_asset(m_shaderHDR, "FX\\HDR"); 
		Asset_Loader::load_asset(m_shaderFXAA, "FX\\FXAA"); 
		Asset_Loader::load_asset(m_shapeQuad, "quad");
		Asset_Loader::load_asset(m_shapeCone, "cone");
		Asset_Loader::load_asset(m_shapeSphere, "sphere");
		Asset_Loader::load_asset(m_shaderSky, "skybox");
		Asset_Loader::load_asset(m_textureSky, "sky\\");
		m_quadVAO = Asset_Primitive::GenerateVAO();
		m_coneVAO = Asset_Primitive::GenerateVAO();
		m_sphereVAO = Asset_Primitive::GenerateVAO();
		m_QuadObserver = (void*)(new Primitivee_Observer(m_shapeQuad, m_quadVAO));
		m_ConeObserver = (void*)(new Primitivee_Observer(m_shapeCone, m_coneVAO));
		m_SphereObserver = (void*)(new Primitivee_Observer(m_shapeSphere, m_sphereVAO));
		m_gbuffer.End();
		
		m_ssaoCallback = new SSAO_Callback(this);
		m_ssaoSamplesCallback = new SSAO_Samples_Callback(this);
		m_ssaoStrengthCallback = new SSAO_Strength_Callback(this);
		m_ssaoRadiusCallback = new SSAO_Radius_Callback(this);
		m_widthChangeCallback = new Cam_WidthChangeCallback(this);
		m_heightChangeCallback = new Cam_HeightChangeCallback(this);
		m_bloomStrengthChangeCallback = new Bloom_StrengthChangeCallback(&m_lbuffer);
		m_QualityChangeCallback = new ShadowQualityChangeCallback(&m_updateQuality);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_SSAO, m_ssaoCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_SSAO_SAMPLES, m_ssaoSamplesCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_SSAO_BLUR_STRENGTH, m_ssaoStrengthCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_SSAO_RADIUS, m_ssaoRadiusCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_WINDOW_WIDTH, m_widthChangeCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_WINDOW_HEIGHT, m_heightChangeCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_BLOOM_STRENGTH, m_bloomStrengthChangeCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_SHADOW_QUALITY, m_QualityChangeCallback);
		m_attribs.m_ssao_radius = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_SSAO_RADIUS);
		m_attribs.m_ssao_strength = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_SSAO_BLUR_STRENGTH);
		m_attribs.m_aa_samples = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_SSAO_SAMPLES);
		m_attribs.m_ssao = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_SSAO);
		m_renderSize = vec2(m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH), m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT));
		m_updateQuality = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_SHADOW_QUALITY);
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

		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

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
		m_shaderDirectional->ExistsYet()) 
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
	struct PriorityLightList {
		// Nested Element struct
		struct LightElement {
			double m_updateTime;
			Lighting_Component *m_ptr;
			char* m_lightType;
			LightElement(const double &time, Lighting_Component *ptr, char* type) {
				m_updateTime = time;
				m_ptr = ptr;
				m_lightType = type;
			}
		};

		// Constructor
		PriorityLightList(const int &capacity) {
			m_capacity = capacity;
			m_list.reserve(capacity);
		}

		// Accessors/Updaters
		void add(Lighting_Component *c, char *type) {
			const size_t &listSize = m_list.size();
			double time = c->getShadowUpdateTime(); 
			int insertionIndex = 0;
			if (listSize < m_capacity)
				insertionIndex = listSize;				
			for (int x = 0; x < listSize && x < m_capacity; ++x) {
				if (time < m_list[x].m_updateTime) {
					insertionIndex = x;
					break;
				}
			}
			m_list.insert(m_list.begin() + insertionIndex, LightElement(time, c, type));
			if (m_list.size() > m_capacity) {
				m_list.reserve(m_capacity);
				m_list.shrink_to_fit();
			}			
		}
		Visibility_Token toVisToken() {
			Visibility_Token token;
			token.insert("Light_Directional");
			token.insert("Light_Point");
			token.insert("Light_Spot");
			for (int x = 0, size = m_list.size(); x < size && x < m_capacity; ++x) {
				const auto & element = m_list[x];
				token[element.m_lightType].push_back((Component*)element.m_ptr);
			}
			return token;
		}

		// Members
		int m_capacity;
		vector<LightElement> m_list;
	};

	// Quit early if we don't have models, or we don't have any lights 
	// Logically equivalent to continuing while we have at least 1 model and 1 light
	if ((!vis_token.find("Anim_Model")) ||
		(!vis_token.find("Light_Directional"))  &&
		(!vis_token.find("Light_Point")) &&
		(!vis_token.find("Light_Spot")))
		return;

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_BACK);

	auto m_Shadowmapper = (m_enginePackage->FindSubSystem("Shadows") ? m_enginePackage->GetSubSystem<System_Shadowmap>("Shadows") : nullptr);
	
	PriorityLightList timedList(m_updateQuality);
	for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Directional"))
		timedList.add(component, "Light_Directional");
	for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Point"))
		timedList.add(component, "Light_Point");
	for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Spot"))
		timedList.add(component, "Light_Spot");


	const Visibility_Token &priorityLights = timedList.toVisToken();
	m_Shadowmapper->BindForWriting(SHADOW_LARGE);
	m_shaderShadowDir->Bind();
	for each (auto &component in priorityLights.getTypeList<Lighting_Component>("Light_Directional"))
		component->shadowPass();

	m_Shadowmapper->BindForWriting(SHADOW_REGULAR);
	m_shaderShadowPoint->Bind();
	for each (auto &component in priorityLights.getTypeList<Lighting_Component>("Light_Point"))
		component->shadowPass();

	m_shaderShadowSpot->Bind();
	for each (auto &component in priorityLights.getTypeList<Lighting_Component>("Light_Spot"))
		component->shadowPass();

	Asset_Shader::Release();	
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
}

void System_Graphics_PBR::GeometryPass(const Visibility_Token & vis_token)
{
	if (vis_token.find("Anim_Model")) {
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glCullFace(GL_BACK);
		m_gbuffer.Clear();
		m_gbuffer.BindForWriting();
		m_shaderGeometry->Bind();

		for each (auto &component in vis_token.getTypeList<Geometry_Component>("Anim_Model"))
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
	const size_t &cone_size = m_shapeCone->GetSize();
	const size_t &sphere_size = m_shapeSphere->GetSize();
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	m_gbuffer.BindForReading();

	auto m_Shadowmapper = (m_enginePackage->FindSubSystem("Shadows") ? m_enginePackage->GetSubSystem<System_Shadowmap>("Shadows") : nullptr);
	m_Shadowmapper->BindForReading(SHADOW_LARGE, 3);
	if (vis_token.find("Light_Directional")) {
		m_shaderDirectional->Bind();
		glBindVertexArray(m_quadVAO);
		for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Directional"))
			component->directPass(quad_size);
	}

	glEnable(GL_STENCIL_TEST);
	glCullFace(GL_FRONT);
	m_Shadowmapper->BindForReading(SHADOW_REGULAR, 3);
	if (vis_token.find("Light_Point")) {
		m_shaderPoint->Bind();
		glBindVertexArray(m_sphereVAO);
		for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Point"))
			component->directPass(sphere_size);
	}

	if (vis_token.find("Light_Spot")) {
		m_shaderSpot->Bind();
		glBindVertexArray(m_coneVAO);
		for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Spot"))
			component->directPass(cone_size);
	}

	glCullFace(GL_BACK);
	glDisable(GL_STENCIL_TEST);
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);
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