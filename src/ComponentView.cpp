#define ZP_ENTITY_COMPONENT_SYSTEM_ACCESS

#include "ComponentView.h"

#include "Zap/Scene/Scene.h"

#include "imgui.h"
#include "imgui_internal.h"

void deleteSave(void* data) {
	if (data) delete data;
}

namespace editor {
	ImGuiWindowFlags ComponentEditor::getWindowFlags() {
		return 0;
	}

	TransformEditor::TransformEditor(std::vector<Zap::Actor>& selectedActors)
		: m_selectedActors(selectedActors)
	{}

	TransformEditor::~TransformEditor(){}

	std::string TransformEditor::name() {
		return "TransformEditor";
	}

	void TransformEditor::draw() {
		if (!isValid()) {
			delete this;
			return;
		}
		if (m_selectedActors.empty()) {
			ImGui::BeginDisabled();
			ImGui::Text("No Actor selected");
			ImGui::EndDisabled();
			return;
		}
		Zap::Actor selectedActor = m_selectedActors[0];
		glm::vec3 pos = selectedActor.cmpTransform_getPos();
		ImGui::DragFloat3("Position", (float*)&pos, 0.1);
		selectedActor.cmpTransform_setPos(pos);
		if (selectedActor.hasRigidDynamic()) {
			selectedActor.cmpRigidDynamic_updatePose();
		}
	}

	bool TransformEditor::isValid() {
		if (!m_selectedActors.empty())
			return m_selectedActors[0].hasTransform();
		return false;
	}

	RigidDynamicEditor::RigidDynamicEditor(std::vector<Zap::Actor>& selectedActors)
		: m_selectedActors(selectedActors)
	{}

	RigidDynamicEditor::~RigidDynamicEditor() {}

	std::string RigidDynamicEditor::name() {
		return "RigidDynamicEditor";
	}

	void RigidDynamicEditor::draw() {
		if (!isValid()) {
			delete this;
			return;
		}
		if (m_selectedActors.empty()) {
			ImGui::BeginDisabled();
			ImGui::Text("No Actor selected");
			ImGui::EndDisabled();
			return;
		}
		Zap::Actor selectedActor = m_selectedActors[0];
		if (ImGui::Button("update")) {
			selectedActor.cmpRigidDynamic_updatePose();
		}
	}

	bool RigidDynamicEditor::isValid() {
		if (!m_selectedActors.empty())
			return m_selectedActors[0].hasRigidDynamic();
		return false;
	}

	LightEditor::LightEditor(std::vector<Zap::Actor>& selectedActors)
		: m_selectedActors(selectedActors)
	{}

	LightEditor::~LightEditor() {}

	std::string LightEditor::name() {
		return "LightEditor";
	}

	void LightEditor::draw() {
		if (!isValid()) {
			delete this;
			return;
		}
		if (m_selectedActors.empty()) {
			ImGui::BeginDisabled();
			ImGui::Text("No Actor selected");
			ImGui::EndDisabled();
			return;
		}
		Zap::Actor selectedActor = m_selectedActors[0];

		glm::vec3 color = selectedActor.cmpLight_getColor();
		ImGui::ColorPicker3("color", (float*)&color);
		selectedActor.cmpLight_setColor(color);

		float strength = selectedActor.cmpLight_getStrength();
		ImGui::DragFloat("strength", &strength, 0.1);
		selectedActor.cmpLight_setStrength(strength);
	}

	bool LightEditor::isValid() {
		if (!m_selectedActors.empty())
			return m_selectedActors[0].hasLight();
		return false;
	}

	ComponentView::ComponentView(std::vector<Zap::Actor>& selectedActors)
		: m_selectedActors(selectedActors)
	{}

	ComponentView::~ComponentView(){}

	std::string ComponentView::name() {
		if (m_selectedActors.empty()) {
			return "ComponentView";
		}
		else {
			return "ComponentView - " + std::to_string(m_selectedActors[0].m_handle);
		}
	}

	void ComponentView::draw() {
		if (m_selectedActors.empty()) {
			ImGui::BeginDisabled();
			ImGui::Text("No Actor selected");
			ImGui::EndDisabled();
			return;
		}
		Zap::Actor selectedActor = m_selectedActors[0];

		ImGui::BeginChild("Selection", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX);
		if (selectedActor.hasTransform()) 
			if(ImGui::Button("Transform")) {
				deleteSave(m_selectedEditor);
				m_selectedEditor = new TransformEditor(m_selectedActors);
			}
		if (selectedActor.hasModel()) 
			if (ImGui::Button("Model")) {
				deleteSave(m_selectedEditor);
				m_selectedEditor = nullptr;
			}
		if (selectedActor.hasLight()) 
			if(ImGui::Button("Light")){
				deleteSave(m_selectedEditor);
				m_selectedEditor = new LightEditor(m_selectedActors);
			}
		if (selectedActor.hasRigidDynamic()) 
			if(ImGui::Button("RigidDynamic")){
				deleteSave(m_selectedEditor);
				m_selectedEditor = new RigidDynamicEditor(m_selectedActors);
			}
		if (selectedActor.hasRigidStatic()) 
			if(ImGui::Button("RigidStatic")){
				deleteSave(m_selectedEditor);
				m_selectedEditor = nullptr;
			}
		if (selectedActor.hasCamera()) 
			if(ImGui::Button("Camera")){
				deleteSave(m_selectedEditor);
				m_selectedEditor = nullptr;
			}
		ImGui::EndChild();

		ImGui::SameLine();
		if (m_selectedEditor && m_selectedEditor->isValid()) {
			ImGui::BeginChild(m_selectedEditor->name().c_str());
			m_selectedEditor->draw();
		}
		else {
			if (m_selectedEditor && !m_selectedEditor->isValid()) {
				delete m_selectedEditor;
				m_selectedEditor = nullptr;
			}
			ImGui::BeginChild("DefaultComponentEditor");
			ImGui::BeginDisabled();
			ImGui::Text("No editor available");
			ImGui::EndDisabled();
		}
		ImGui::EndChild();
	}

	ImGuiWindowFlags ComponentView::getWindowFlags() {
		return 0;
	}
}