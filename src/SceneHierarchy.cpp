#define ZP_ENTITY_COMPONENT_SYSTEM_ACCESS

#include "SceneHierarchy.h"

#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Model.h"

#include "imgui.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

namespace editor {
	SceneHierarchyView::SceneHierarchyView(EditorData* pEditorData, Zap::Scene* pScene, std::vector<Zap::Actor>& allActors, std::vector<Zap::Actor>& selectedActors)
		: m_pEditorData(pEditorData), m_pScene(pScene), m_allActors(allActors), m_selectedActors(selectedActors)
	{}

	SceneHierarchyView::~SceneHierarchyView(){}

	std::string SceneHierarchyView::name() {
		return "SceneHierarchyView";
	}

	void SceneHierarchyView::draw() {
		if(!ImGui::IsPopupOpen("SceneEdit##Popup"))
			m_hoveredActorIndex = 0xFFFFFFFF;

		uint32_t i = 0;
		for (Zap::Actor actor : m_allActors) {
			std::string actorName;
			if (m_pEditorData->actorNameMap.count(actor))
				actorName = m_pEditorData->actorNameMap.at(actor);
			else {
				std::stringstream stream;
				stream << "Actor_" << std::hex << (Zap::UUID)actor;
				actorName = stream.str();
			}

			//check if actor is selected
			bool selected = false;
			for (auto selectedActor : m_selectedActors) {
				if (selectedActor == actor) {
					selected = true;
				}
			}

			if (selected) {
				ImGui::PushStyleColor(ImGuiCol_Button, { 218 / 255.0, 59 / 255.0, 1 / 255.0, 1 });
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 230 / 255.0, 100 / 255.0, 60 / 255.0, 1 });
			}

			if (ImGui::Button(actorName.c_str())) {
				m_selectedActors.clear();
				m_selectedActors.push_back(actor);
			}

			// DragDrop MeshToActor
			if (ImGui::BeginDragDropTarget()) {
				if (auto* payload = ImGui::AcceptDragDropPayload("MeshToActorPayload")) {
					Zap::Mesh mesh = Zap::Mesh(*((Zap::UUID*)payload->Data));
					if (!actor.hasModel()) {
						actor.addModel({ "", {Zap::Material()}, {mesh} });
					}
					else {
						actor.cmpModel_addMesh(mesh);
					}
				}
				ImGui::EndDragDropTarget();
			}

			if (selected) ImGui::PopStyleColor(2);

			if (ImGui::IsItemHovered())
				m_hoveredActorIndex = i;
			i++;
		}
		if (ImGui::IsWindowHovered()) {
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_hoveredActorIndex == 0xFFFFFFFF)
				m_selectedActors.clear();
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				ImGui::OpenPopup("SceneEdit##Popup");
		}

		// SceneEditPopup
		bool shouldSceneEditPopupClose = false;
		if (ImGui::BeginPopup("SceneEdit##Popup")) {
			if (ImGui::Button("Add")) {
				m_actorCreationData.newActor = Zap::Actor();// setup one time data for actor creation
				char data[50] = "";
				memcpy(m_actorCreationData.nameInputBuffer, data, sizeof(char)*50);
				ImGui::OpenPopup("ActorCreation##Popup");
			}

			// ActorCreationPopup
			if (ImGui::BeginPopupModal("ActorCreation##Popup")) {

				ImGui::Checkbox("Name", &m_actorCreationData.createName);// create name
				if (m_actorCreationData.createName) {
					uint32_t charBufSize = 50;
					ImGui::InputText("##NameInput", m_actorCreationData.nameInputBuffer, charBufSize);
					m_actorCreationData.name = m_actorCreationData.nameInputBuffer;
				}

				ImGui::Checkbox("Transform", &m_actorCreationData.createTransform);// create transform
				if (m_actorCreationData.createTransform) {
					glm::vec3 pos = m_actorCreationData.transform.transform[3];
					ImGui::DragFloat3("Pos", (float*)&pos, 0.1);
					m_actorCreationData.transform.transform[3] = glm::vec4(pos, 1);
				}

				if (ImGui::Button("Done")) {
					m_allActors.push_back(m_actorCreationData.newActor);
					Zap::Actor* pActor = &m_allActors.back();
					m_pScene->attachActor(*pActor);
					if (m_actorCreationData.createName)
						m_pEditorData->actorNameMap[*pActor] = m_actorCreationData.name;
					if (m_actorCreationData.createTransform)
						pActor->addTransform(m_actorCreationData.transform);

					m_actorCreationData = {};
					ImGui::CloseCurrentPopup();
					shouldSceneEditPopupClose = true;
				}
				ImGui::EndPopup();
			}

			if (m_hoveredActorIndex < 0xFFFFFFFF) {
				if (ImGui::Button("Delete")) {
					//delete custom data
					if(m_pEditorData->actorNameMap.count(m_allActors[m_hoveredActorIndex]))
						m_pEditorData->actorNameMap.erase(m_allActors[m_hoveredActorIndex]);
					//delete actor
					m_allActors[m_hoveredActorIndex].destroy();
					m_allActors.erase(m_allActors.begin() + m_hoveredActorIndex);
					ImGui::CloseCurrentPopup();
				}
			}
			if (shouldSceneEditPopupClose)
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}

	ImGuiWindowFlags SceneHierarchyView::getWindowFlags() {
		return 0;
	}
}