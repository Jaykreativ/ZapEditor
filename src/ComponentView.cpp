#define ZP_ENTITY_COMPONENT_SYSTEM_ACCESS

#include "ComponentView.h"

#include "Zap/Scene/Scene.h"

#include "imgui.h"

namespace editor {
	void TransformEditor::draw(Zap::Actor selectedActor) {
		ImGui::Text("Transform Editor");
	}

	ComponentView::ComponentView(){}
	ComponentView::~ComponentView(){}

	void ComponentView::draw(Zap::Actor selectedActor) {
		if (selectedActor.m_pScene->m_transformComponents.count(selectedActor.m_handle)) 
			if(ImGui::Button("Transform")) {
				delete m_selectedEditor;
				m_selectedEditor = new TransformEditor();
			}
		if (selectedActor.m_pScene->m_modelComponents.count(selectedActor.m_handle)) 
			if (ImGui::Button("Model")) {
				delete m_selectedEditor;
				m_selectedEditor = nullptr;
			}
		if (selectedActor.m_pScene->m_lightComponents.count(selectedActor.m_handle)) 
			if(ImGui::Button("Light")){
				delete m_selectedEditor;
				m_selectedEditor = nullptr;
			}
		if (selectedActor.m_pScene->m_rigidDynamicComponents.count(selectedActor.m_handle)) 
			if(ImGui::Button("RigidDynamic")){
				delete m_selectedEditor;
				m_selectedEditor = nullptr;
			}
		if (selectedActor.m_pScene->m_rigidStaticComponents.count(selectedActor.m_handle)) 
			if(ImGui::Button("RigidStatic")){
				delete m_selectedEditor;
				m_selectedEditor = nullptr;
			}
		if (selectedActor.m_pScene->m_cameraComponents.count(selectedActor.m_handle)) 
			if(ImGui::Button("Camera")){
				delete m_selectedEditor;
				m_selectedEditor = nullptr;
			}
	}

	ComponentEditor* ComponentView::getSelectedEditor() {
		return m_selectedEditor;
	}
}