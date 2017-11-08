#include "Rendering\Test_Scene.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Shader.h"
#include "Entities\Prop.h"

static Shared_Asset_Primitive quad;
static Shared_Asset_Shader shader;
static Prop *prop;

// Shutdown
Test_Scene::~Test_Scene()
{
}

// Startup
Test_Scene::Test_Scene()
{
	Asset_Manager::load_asset(quad, "sphere");
	Asset_Manager::load_asset(shader, "testShader");
	prop = new Prop("Test\\ChamferedCube.obj");
}

void Test_Scene::RenderFrame()
{
	glViewport(0, 0, 512, 512);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	shader->Bind();
	prop->geometryPass();
	shader->Release();
}
