#define ZP_ENTITY_COMPONENT_SYSTEM_ACCESS
#include "ComponentView.h"

#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "glm/gtx/matrix_decompose.hpp"

#include <string>

void deleteSave(void* data) {
	if (data) delete data;
}

namespace editor {
	ImGuiWindowFlags ComponentEditor::getWindowFlags() {
		return 0;
	}

	TransformEditor::TransformEditor(std::vector<Zap::Actor> selectedActors)
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

		glm::mat4 transform = selectedActor.cmpTransform_getTransform();
		glm::vec3 scale;
		glm::quat quat;
		glm::vec3 pos;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(transform, scale, quat, pos, skew, perspective);
		glm::vec3 rot = glm::degrees(glm::eulerAngles(quat));

		ImGui::SeparatorText("Position");
		ImGui::DragFloat3("##Position", (float*)&pos, 0.1);

		ImGui::SeparatorText("Rotation");
		ImGui::DragFloat3("##Rotation", (float*)&rot, 0.1);

		ImGui::SeparatorText("Scale");
		ImGui::DragFloat3("##Scale", (float*)&scale, 0.1);
		if (scale.x < 0.01) scale.x = 0.01;
		if (scale.y < 0.01) scale.y = 0.01;
		if (scale.z < 0.01) scale.z = 0.01;

		transform = glm::mat4(1);
		rot = glm::radians(rot);
		glm::mat4 rotMatX = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));
		glm::mat4 rotMatY = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::mat4 rotMatZ = glm::rotate(glm::mat4(1), rot.z, glm::vec3(0, 0, 1));

		transform[0] = rotMatX * transform[0];//TODO improve by adding custom component to actors via seperate hashmap
		transform[1] = rotMatX * transform[1];
		transform[2] = rotMatX * transform[2];

		transform[0] = rotMatY * transform[0];
		transform[1] = rotMatY * transform[1];
		transform[2] = rotMatY * transform[2];

		transform[0] = rotMatZ * transform[0];
		transform[1] = rotMatZ * transform[1];
		transform[2] = rotMatZ * transform[2];

		transform = glm::scale(transform, glm::max(scale, glm::vec3(0.001)));
		transform[3] = glm::vec4(pos, 1);

		selectedActor.cmpTransform_setTransform(transform);

		if (selectedActor.hasRigidDynamic()) {
			selectedActor.cmpRigidDynamic_updatePose();
		}
		if (selectedActor.hasRigidStatic()) {
			selectedActor.cmpRigidStatic_updatePose();
		}
	}

	bool TransformEditor::isValid() {
		if (!m_selectedActors.empty())
			return m_selectedActors[0].hasTransform();
		return false;
	}

	void ShapeCreateSection::drawCreateMaterialPopup(EditorData& editorData) {
		if (ImGui::BeginPopupModal("MaterialCreation##Popup")) {
			ImGui::DragFloat("static friction", &m_materialCreationInfo.staticFriction, 0.01, 0, 1);
			ImGui::DragFloat("dynamic friction", &m_materialCreationInfo.dynamicFriction, 0.01, 0, 1);
			ImGui::DragFloat("restitution", &m_materialCreationInfo.restitution, 0.01, 0, 1);
			if (ImGui::Button("Done")) {
				editorData.physicsMaterials.push_back(Zap::PhysicsMaterial(m_materialCreationInfo.staticFriction, m_materialCreationInfo.dynamicFriction, m_materialCreationInfo.restitution));
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

	}

	void ShapeCreateSection::drawCreateShapePopup(EditorData& editorData, Zap::Actor selectedActor, bool isDynamic, bool isExclusive) {
		const char* popupID;
		if (isExclusive)
			popupID = "CreateExclusive##Popup";
		else
			popupID = "CreateShape##Popup";
		if (ImGui::BeginPopupModal(popupID)) {
			static const int typeCount = 5;
			static std::string typeStrings[typeCount] = {
				"None",
				"Sphere",
				"Capsule",
				"Box",
				"Plane"
				//"Convex Mesh",
				//"Triangle Mesh",
				//"Height Field"
			};
			ImGui::SeparatorText("Geometry");
			if (ImGui::BeginCombo("Type", typeStrings[m_shapeCreationInfo.geometryType].c_str()))
			{
				for (int i = 1; i < typeCount; i++)
				{
					const bool is_selected = (m_shapeCreationInfo.geometryType == i);
					if (ImGui::Selectable(typeStrings[i].c_str(), is_selected)) {
						m_shapeCreationInfo.geometryType = i;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			switch (m_shapeCreationInfo.geometryType) // draw settings for geometry
			{
			case Zap::eGEOMETRY_TYPE_SPHERE: {
				ImGui::DragFloat("radius", &m_shapeCreationInfo.sphereRadius, 0.01);
				break;
			}
			case Zap::eGEOMETRY_TYPE_CAPSULE: {
				ImGui::DragFloat("radius", (float*)&m_shapeCreationInfo.capsuleRadius, 0.01);
				ImGui::DragFloat("half height", (float*)&m_shapeCreationInfo.capsuleHalfHeight, 0.01);
				break;
			}
			case Zap::eGEOMETRY_TYPE_BOX: {
				ImGui::DragFloat3("half extents", (float*)&m_shapeCreationInfo.boxExtent, 0.01);
				break;
			}
			default: {
				ImGui::Text("No settings available");
			}
			}

			ImGui::SeparatorText("Physics material");
			if (ImGui::BeginCombo("Material selection", ("Material" + std::to_string(m_shapeCreationInfo.materialIndex)).c_str())) {
				uint32_t i = 0;
				for (auto& material : editorData.physicsMaterials) {
					if (ImGui::Selectable(("Material" + std::to_string(i)).c_str(), i == m_shapeCreationInfo.materialIndex)) {
						m_shapeCreationInfo.materialIndex = i;
					}
					i++;
				}
				ImGui::EndCombo();
			}
			if (ImGui::Button("+")) {
				ImGui::OpenPopup("MaterialCreation##Popup");
			}

			drawCreateMaterialPopup(editorData);

			if (ImGui::Button("Done")) {
				Zap::PhysicsGeometry* pGeometry = nullptr;
				switch (m_shapeCreationInfo.geometryType)
				{
				case Zap::eGEOMETRY_TYPE_NONE: {
					ZP_ASSERT(false, "Shape can't be created with eGEOMETRY_TYPE_NONE");
					break;
				}
				case Zap::eGEOMETRY_TYPE_SPHERE: {
					pGeometry = new Zap::SphereGeometry(m_shapeCreationInfo.sphereRadius);
					break;
				}
				case Zap::eGEOMETRY_TYPE_CAPSULE: {
					pGeometry = new Zap::CapsuleGeometry(m_shapeCreationInfo.capsuleRadius, m_shapeCreationInfo.capsuleHalfHeight);
					break;
				}
				case Zap::eGEOMETRY_TYPE_BOX: {
					pGeometry = new Zap::BoxGeometry(m_shapeCreationInfo.boxExtent);
					break;
				}
				case Zap::eGEOMETRY_TYPE_PLANE: {
					pGeometry = new Zap::PlaneGeometry();
					break;
				}
				default: {
					ZP_ASSERT(false, "Shape must be created with known geometry type");
					break;
				}
				}
				Zap::Shape shape(*pGeometry, editorData.physicsMaterials[m_shapeCreationInfo.materialIndex], isExclusive);
				if (isDynamic)
					selectedActor.cmpRigidDynamic_attachShape(shape);
				else
					selectedActor.cmpRigidStatic_attachShape(shape);
				if (isExclusive)
					shape.release();
				else
					editorData.physicsShapes.push_back(shape);
				delete pGeometry;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	void ShapeCreateSection::drawAddShapePopup(EditorData& editorData, Zap::Actor selectedActor, bool isDynamic) {
		if (ImGui::BeginPopup("AddShape##Popup")) {
			uint32_t i = 0;
			for (auto& shape : editorData.physicsShapes) {
				if (ImGui::Button(("shape" + std::to_string(i)).c_str())) {
					if (isDynamic)
						selectedActor.cmpRigidDynamic_attachShape(shape);
					else
						selectedActor.cmpRigidStatic_attachShape(shape);
				}
				i++;
			}
			if (ImGui::Button("+")) {
				ImGui::OpenPopup("CreateShape##Popup");
			}

			drawCreateShapePopup(editorData, selectedActor, false);

			ImGui::EndPopup();
		}
	}

	void ShapeCreateSection::drawAddExclusivePopup(EditorData& editorData, Zap::Actor selectedActor, bool isDynamic) {
		drawCreateShapePopup(editorData, selectedActor, isDynamic, true);
	}

	void ShapeCreateSection::draw(EditorData& editorData, Zap::Actor selectedActor, bool isDynamic) {
		std::vector<Zap::Shape> shapes;
		if (isDynamic)
			shapes = selectedActor.cmpRigidDynamic_getShapes();
		else
			shapes = selectedActor.cmpRigidStatic_getShapes();
		uint32_t i = 0;
		for (auto& shape : shapes) {
			if (ImGui::Button(("Shape" + std::to_string(i)).c_str())) {

			}
			i++;
		}
		if (ImGui::Button("Add")) {
			ImGui::OpenPopup("AddShape##Popup");
		}
		ImGui::SameLine();
		if (ImGui::Button("Add exclusive")) {
			ImGui::OpenPopup("CreateExclusive##Popup");
		}

		drawAddShapePopup(editorData, selectedActor, isDynamic);

		drawAddExclusivePopup(editorData, selectedActor, isDynamic);
	}

	RigidDynamicEditor::RigidDynamicEditor(EditorData* pEditorData, std::vector<Zap::Actor>& selectedActors)
		: m_pEditorData(pEditorData), m_selectedActors(selectedActors)
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

		m_shapeCreateSection.draw(*m_pEditorData, selectedActor, true);
	}

	bool RigidDynamicEditor::isValid() {
		if (!m_selectedActors.empty())
			return m_selectedActors[0].hasRigidDynamic();
		return false;
	}

	RigidStaticEditor::RigidStaticEditor(EditorData* pEditorData, std::vector<Zap::Actor>& selectedActors)
		: m_pEditorData(pEditorData), m_selectedActors(selectedActors)
	{}

	RigidStaticEditor::~RigidStaticEditor() {}

	std::string RigidStaticEditor::name() {
		return "RigidStaticEditor";
	}

	void RigidStaticEditor::draw() {
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

		m_shapeCreateSection.draw(*m_pEditorData, selectedActor, false);
	}

	bool RigidStaticEditor::isValid() {
		if (!m_selectedActors.empty())
			return m_selectedActors[0].hasRigidStatic();
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

		ImGui::SeparatorText("Color");
		glm::vec3 color = selectedActor.cmpLight_getColor();
		ImGui::ColorPicker3("Color", (float*)&color);
		selectedActor.cmpLight_setColor(color);

		ImGui::SeparatorText("Strength");
		float strength = selectedActor.cmpLight_getStrength();
		ImGui::DragFloat("Strength", &strength, 0.1);
		strength = std::max<float>(strength, 0.0);
		selectedActor.cmpLight_setStrength(strength);

		ImGui::SeparatorText("Radius");
		float radius = selectedActor.cmpLight_getRadius();
		ImGui::DragFloat("Radius", &radius, 0.01);
		radius = std::max<float>(radius, 0.0);
		selectedActor.cmpLight_setRadius(radius);
	}

	bool LightEditor::isValid() {
		if (!m_selectedActors.empty())
			return m_selectedActors[0].hasLight();
		return false;
	}

	ComponentView::ComponentView(EditorData* pEditorData, std::vector<ViewLayer*>& layers, std::vector<Zap::Actor>& selectedActors)
		: m_pEditorData(pEditorData), m_layers(layers), m_selectedActors(selectedActors)
	{}

	ComponentView::~ComponentView(){}

	std::string ComponentView::name() {
		if (m_selectedActors.empty()) {
			return "ComponentView";
		}
		else {
			return "ComponentView - " + std::to_string(m_selectedActors[0].getHandle());
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
		if (!selectedActor.isValid()) {
			ImGui::BeginDisabled();
			ImGui::Text("Invalid Actor selected");
			if (!selectedActor.getScene())
				ImGui::Text("Not attached to any scene");
			ImGui::EndDisabled();
			return;
		}

		ImGui::BeginChild("Selection", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX);
		if (selectedActor.hasTransform())
			if (ImGui::Button("Transform")) {
				deleteSave(m_selectedEditor);
				m_selectedEditor = new TransformEditor(m_selectedActors);
			}
		if (selectedActor.hasModel())
			if (ImGui::Button("Model")) {
				deleteSave(m_selectedEditor);
				m_selectedEditor = nullptr;
			}
		if (selectedActor.hasLight())
			if (ImGui::Button("Light")) {
				deleteSave(m_selectedEditor);
				m_selectedEditor = new LightEditor(m_selectedActors);
			}
		if (selectedActor.hasRigidDynamic())
			if (ImGui::Button("RigidDynamic")) {
				deleteSave(m_selectedEditor);
				m_selectedEditor = new RigidDynamicEditor(m_pEditorData, m_selectedActors);
			}
		if (selectedActor.hasRigidStatic())
			if (ImGui::Button("RigidStatic")) {
				deleteSave(m_selectedEditor);
				m_selectedEditor = new RigidStaticEditor(m_pEditorData, m_selectedActors);
			}
		if (selectedActor.hasCamera())
			if (ImGui::Button("Camera")) {
				deleteSave(m_selectedEditor);
				m_selectedEditor = nullptr;
			}
		if (ImGui::Button("+")) {
			ImGui::OpenPopup("NewComponent##Popup");
		}
		if (ImGui::BeginPopup("NewComponent##Popup")) {
			if (ImGui::Button("Transform")) {
				m_selectedActors.back().addTransform();
			}
			if (ImGui::Button("Model")) {
				ImGui::OpenPopup("ModelCreation##Popup");
			}
			if (ImGui::BeginPopup("ModelCreation##Popup")) {
				ImGui::Text("add new models through drag and dropping meshes from assetbrowser");
				if (ImGui::Button("Done"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
			if (ImGui::Button("Light")) {
				m_selectedActors.back().addLight();
			}
			if (ImGui::Button("RigidDynamic")) {
				m_selectedActors.back().addRigidDynamic();
			}
			if (ImGui::Button("RigidStatic")) {
				m_selectedActors.back().addRigidStatic();
			}
			if (ImGui::Button("Camera")) {
				m_selectedActors.back().addCamera(Zap::Camera());
			}
			ImGui::EndPopup();
		}
		ImGui::EndChild();


		ImGui::SameLine();
		if (m_selectedEditor && m_selectedEditor->isValid()) {
			ImGui::BeginChild(m_selectedEditor->name().c_str());
			if (ImGui::Button("unlock")) {
				m_layers.push_back(m_selectedEditor);
				m_selectedEditor = nullptr;
			}
			else {
				m_selectedEditor->draw();
			}
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