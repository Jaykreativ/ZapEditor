#define ZP_ENTITY_COMPONENT_SYSTEM_ACCESS

#include "SceneHierarchy.h"
#include "FileHandling.h"
#include "SceneHandling.h"

#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Model.h"

#include "imgui.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

namespace editor {
	SceneHierarchyView::SceneHierarchyView(EditorData* pEditorData, Zap::Scene* pScene)
		: m_pEditorData(pEditorData), m_pScene(pScene)
	{}

	SceneHierarchyView::~SceneHierarchyView(){}

	std::string SceneHierarchyView::name() {
		return "SceneHierarchyView";
	}

	void SceneHierarchyView::draw() {
		if(!ImGui::IsPopupOpen("SceneEdit##Popup"))
			m_hoveredActorIndex = 0xFFFFFFFF;

		uint32_t i = 0;
		for (Zap::Actor actor : m_pEditorData->actors) {
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
			for (auto selectedActor : m_pEditorData->selectedActors) {
				if (selectedActor == actor) {
					selected = true;
				}
			}

			if (selected) {
				ImGui::PushStyleColor(ImGuiCol_Button, { 218 / 255.0, 59 / 255.0, 1 / 255.0, 1 });
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 230 / 255.0, 100 / 255.0, 60 / 255.0, 1 });
			}

			if (m_renameActorIndex == i) {
				const size_t renameBufSize = 50;
				static char buf[renameBufSize] = "";
				memcpy(buf, actorName.c_str(), std::min<size_t>(actorName.size(), renameBufSize));
				if (ImGui::InputText("##ActorRenameInput", buf, renameBufSize, ImGuiInputTextFlags_EnterReturnsTrue)) {
					m_pEditorData->actorNameMap[actor] =  buf;
					m_renameActorIndex = 0xFFFFFFFF;
				}
				ImGui::SetItemDefaultFocus();
				ImGui::SetKeyboardFocusHere(-1);
				//if (!ImGui::IsItemFocused())
				//	m_renameActorIndex = 0xFFFFFFFF;
			}
			else {
				if (ImGui::Button(actorName.c_str())) { // Actor selection button
					m_pEditorData->selectedActors.clear();
					m_pEditorData->selectedActors.push_back(actor);
				}
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
				m_pEditorData->selectedActors.clear();
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				ImGui::OpenPopup("SceneEdit##Popup");
		}

		// SceneEditPopup
		bool shouldSceneEditPopupClose = false;
		if (ImGui::BeginPopup("SceneEdit##Popup")) {

			// Create Actor
			if (ImGui::Button("Add")) {
				m_actorCreationData = {};
				m_actorCreationData.newActor = Zap::Actor();// setup one time data for actor creation
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

				if (ImGui::Button("Done") || ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
					Zap::Actor actor = m_actorCreationData.newActor;
					if (!m_actorCreationData.createName)
						m_actorCreationData.name = "";
					m_pScene->attachActor(actor);
					scene::createActor(*m_pEditorData, actor, m_actorCreationData.name);
						m_pEditorData->actorNameMap[actor] = m_actorCreationData.name;
					if (m_actorCreationData.createTransform)
						actor.addTransform(m_actorCreationData.transform);

					m_actorCreationData = {};
					ImGui::CloseCurrentPopup();
					shouldSceneEditPopupClose = true;
				}
				ImGui::EndPopup();
			}

			// Delete Actor
			if (m_hoveredActorIndex < 0xFFFFFFFF) {
				if (ImGui::Button("Delete")) {
					//delete custom data
					if(m_pEditorData->actorNameMap.count(m_pEditorData->actors[m_hoveredActorIndex]))
						m_pEditorData->actorNameMap.erase(m_pEditorData->actors[m_hoveredActorIndex]);
					//delete actor
					m_pEditorData->actors[m_hoveredActorIndex].destroy();
					m_pEditorData->actors.erase(m_pEditorData->actors.begin() + m_hoveredActorIndex);
					ImGui::CloseCurrentPopup();
				}
			}

			if (m_hoveredActorIndex < 0xFFFFFFFF) {
				if (ImGui::Button("Save")) {
					Zap::Actor actor = m_pEditorData->actors[m_hoveredActorIndex];
					if (m_pEditorData->actorPathMap.count(actor))
						saveActorFile(m_pEditorData->actorPathMap.at(actor), actor, *m_pEditorData);
					else {
						m_actorSaveData = {};
						m_actorSaveData.actor = actor;
						ImGui::OpenPopup("ActorSave##Popup");
					}
				}
			}

			if (ImGui::BeginPopup("ActorSave##Popup")) {
				ImGui::InputText("filepath", m_actorSaveData.pathInputBuffer, m_actorSaveData.pathInputSize);
				if (ImGui::Button("Done") || ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
					saveActorFile(m_actorSaveData.pathInputBuffer, m_actorSaveData.actor, *m_pEditorData);
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			if (m_hoveredActorIndex < 0xFFFFFFFF) {
				if (ImGui::Button("Rename")) {
					m_renameActorIndex = m_hoveredActorIndex;
					shouldSceneEditPopupClose = true;
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