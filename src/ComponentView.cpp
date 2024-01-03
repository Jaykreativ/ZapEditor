#define ZP_ENTITY_COMPONENT_SYSTEM_ACCESS

#include "ComponentView.h"

#include "Zap/Scene/Scene.h"

#include "imgui.h"

namespace editor {
	ComponentView::ComponentView(){}
	ComponentView::~ComponentView(){}

	void ComponentView::draw(Zap::Actor selectedActor) {
		ImGui::Begin("ComponentView");
		if (selectedActor.m_pScene->m_transformComponents.count(selectedActor.m_handle)) ImGui::Text("Transform");
		if (selectedActor.m_pScene->m_modelComponents.count(selectedActor.m_handle)) ImGui::Text("Model");
		if (selectedActor.m_pScene->m_lightComponents.count(selectedActor.m_handle)) ImGui::Text("Light");
		if (selectedActor.m_pScene->m_rigidDynamicComponents.count(selectedActor.m_handle)) ImGui::Text("RigidDynamic");
		if (selectedActor.m_pScene->m_rigidStaticComponents.count(selectedActor.m_handle)) ImGui::Text("RigidStatic");
		if (selectedActor.m_pScene->m_cameraComponents.count(selectedActor.m_handle)) ImGui::Text("Camera");
		ImGui::End();
	}
}