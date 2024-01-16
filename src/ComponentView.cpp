#define ZP_ENTITY_COMPONENT_SYSTEM_ACCESS

#include "ComponentView.h"

#include "Zap/Scene/Scene.h"

#include "imgui.h"

void deleteSave(void* data) {
	if (data) delete data;
}

namespace editor {
	void TransformEditor::draw(Zap::Actor selectedActor) {
		glm::vec3 pos = selectedActor.cmpTransform_getPos();
		ImGui::DragFloat3("Position", (float*)&pos, 0.1);
		selectedActor.cmpTransform_setPos(pos);
		if (selectedActor.hasRigidDynamic()) {
			selectedActor.cmpRigidDynamic_updatePose();
		}
	}

	void RigidDynamicEditor::draw(Zap::Actor selectedActor) {
		if (ImGui::Button("update")) {
			selectedActor.cmpRigidDynamic_updatePose();
		}
	}

	void LightEditor::draw(Zap::Actor selectedActor) {
		glm::vec3 color = selectedActor.cmpLight_getColor();
		ImGui::ColorPicker3("color", (float*)&color);
		selectedActor.cmpLight_setColor(color);
	}

	ComponentView::ComponentView(){}
	ComponentView::~ComponentView(){}

	void ComponentView::draw(Zap::Actor selectedActor) {
		if (selectedActor.m_pScene->m_transformComponents.count(selectedActor.m_handle)) 
			if(ImGui::Button("Transform")) {
				deleteSave(m_selectedEditor);
				m_selectedEditor = new TransformEditor();
			}
		if (selectedActor.m_pScene->m_modelComponents.count(selectedActor.m_handle)) 
			if (ImGui::Button("Model")) {
				deleteSave(m_selectedEditor);
				m_selectedEditor = nullptr;
			}
		if (selectedActor.m_pScene->m_lightComponents.count(selectedActor.m_handle)) 
			if(ImGui::Button("Light")){
				deleteSave(m_selectedEditor);
				m_selectedEditor = new LightEditor();
			}
		if (selectedActor.m_pScene->m_rigidDynamicComponents.count(selectedActor.m_handle)) 
			if(ImGui::Button("RigidDynamic")){
				deleteSave(m_selectedEditor);
				m_selectedEditor = new RigidDynamicEditor();
			}
		if (selectedActor.m_pScene->m_rigidStaticComponents.count(selectedActor.m_handle)) 
			if(ImGui::Button("RigidStatic")){
				deleteSave(m_selectedEditor);
				m_selectedEditor = nullptr;
			}
		if (selectedActor.m_pScene->m_cameraComponents.count(selectedActor.m_handle)) 
			if(ImGui::Button("Camera")){
				deleteSave(m_selectedEditor);
				m_selectedEditor = nullptr;
			}
	}

	ComponentEditor* ComponentView::getSelectedEditor() {
		return m_selectedEditor;
	}
}