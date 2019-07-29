#include "Modules/Editor/UI/Prefabs.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/World/ECS/components.h"
#include "Modules/World/World_M.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"


Prefabs::~Prefabs()
{
	// Update Indicator
	*m_aliveIndicator = false;
}

Prefabs::Prefabs(Engine * engine, LevelEditor_Module * editor)
	: m_engine(engine), m_editor(editor)
{
	// Update Indicator
	*m_aliveIndicator = true;

	// Preferences
	auto & preferences = engine->getPreferenceState();
	preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {
		m_renderSize.x = (int)f;
	});
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize.y = (int)f;
	});

	// Load prefabs
	populatePrefabs();
}

void Prefabs::tick(const float & deltaTime)
{
	bool t = true;
	ImGui::SetNextWindowSize({ 300.0f, m_renderSize.y - 18.0f }, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos({ 0, 18.0f }, ImGuiCond_Appearing);
	if (ImGui::Begin("Prefabs", NULL)) {
		// Loop over all prefabs
		if (ImGui::Button("Fire Hydrant")) {
			spawnPrefab(0);
		}
	}
	ImGui::End();
}

void Prefabs::populatePrefabs()
{
}

void Prefabs::spawnPrefab(const int & index)
{
	auto & world = m_engine->getModule_World();

	// Dynamic Prop Entity
	Renderable_Component renderable;
	BoundingSphere_Component bsphere;
	Prop_Component prop;
	prop.m_modelName = "FireHydrant\\FireHydrantMesh.obj";
	prop.m_skin = 0;

	// Later, we must perform some check for a transform component, adding it if its missing
	// Similarly, later I'd like to do a check for a prop component, if missing add a sprite billboard component (renders only in the editor)
	Transform_Component trans;
	trans.m_transform.m_position = m_editor->getGizmoPosition();
   	trans.m_transform.update();
	BaseECSComponent * entityComponents[] = { &renderable, &bsphere, &prop, &trans };
	const int types[] = { Renderable_Component::ID, BoundingSphere_Component::ID, Prop_Component::ID, Transform_Component::ID };
	world.makeEntity(entityComponents, types, 4ull);
}