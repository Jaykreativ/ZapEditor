#define ZP_ENTITY_COMPONENT_SYSTEM_ACCESS

#include "SceneHierarchy.h"

#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Model.h"

#include "imgui.h"
#include <iostream>
#include <string>

namespace editor {
	SceneHierarchyView::SceneHierarchyView(Zap::Scene* pScene, std::vector<Zap::Actor>& allActors, std::vector<Zap::Actor>& selectedActors)
		: m_pScene(pScene), m_allActors(allActors), m_selectedActors(selectedActors)
	{}

	SceneHierarchyView::~SceneHierarchyView(){}

	std::string SceneHierarchyView::name() {
		return "SceneHierarchyView";
	}

	void SceneHierarchyView::draw() {
		ImGui::Text(std::to_string(m_allActors.size()).c_str());
		
		if(!ImGui::IsPopupOpen("SceneEdit##Popup"))
			m_hoveredActorIndex = 0xFFFFFFFF;

		uint32_t i = 0;
		for (Zap::Actor actor : m_allActors) {
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

			if (ImGui::Button(std::to_string(actor.m_handle).c_str())) {
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
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			ImGui::OpenPopup("SceneEdit##Popup");
		}

		// SceneEditPopup
		bool shouldSceneEditPopupClose = false;
		if (ImGui::BeginPopup("SceneEdit##Popup")) {
			if (ImGui::Button("Add")) {
				ImGui::OpenPopup("ActorCreation##Popup");
			}

			// ActorCreationPopup
			if (ImGui::BeginPopupModal("ActorCreation##Popup")) {
				ImGui::Checkbox("Create Transform", &m_actorCreationData.createTransform);
				if (m_actorCreationData.createTransform) {
					glm::vec3 pos = m_actorCreationData.transform.transform[3];
					ImGui::DragFloat3("Pos", (float*)&pos, 0.1);
					m_actorCreationData.transform.transform[3] = glm::vec4(pos, 1);
				}
				if (ImGui::Button("Done")) {
					m_allActors.push_back(Zap::Actor());
					Zap::Actor* pActor = &m_allActors.back();
					m_pScene->attachActor(*pActor);
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