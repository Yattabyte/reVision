#include "Rendering\Test_Scene.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Shader.h"
static Shared_Asset_Primitive quad;
static Shared_Asset_Shader shader;

// Shutdown
Test_Scene::~Test_Scene()
{
}

// Startup
Test_Scene::Test_Scene()
{
	Asset_Manager::load_asset(quad, "sphere");
	Asset_Manager::load_asset(shader, "testShader");
}

void Test_Scene::RenderFrame()
{
	glViewport(0, 0, 512, 512);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	shader->Bind();
	quad->Bind();
	quad->Draw();
	quad->Unbind();
	shader->Release();
}
