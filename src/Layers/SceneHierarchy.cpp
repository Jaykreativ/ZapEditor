#define ZP_ENTITY_COMPONENT_SYSTEM_ACCESS

#include "SceneHierarchy.h"
#include "FileHandling.h"
#include "SceneHandling.h"

#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Components.h"

#include "imgui.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

namespace editor {
	SceneHierarchyView::SceneHierarchyView(EditorData* pEditorData)
		: SceneAccessLayer(*pEditorData->pSceneHandler), m_pEditorData(pEditorData)
	{}

	SceneHierarchyView::~SceneHierarchyView(){}

	std::string SceneHierarchyView::name() {
		return "SceneHierarchyView";
	}

	void SceneHierarchyView::draw() {
		if (ImGui::BeginMenuBar()) {
			SceneAccessLayer::draw();
			ImGui::EndMenuBar();
		}
		if(!ImGui::IsPopupOpen("SceneEdit##Popup"))
			m_hoveredActorIndex = 0xFFFFFFFF;

		uint32_t i = 0;
		for (Zap::Actor actor : scene().actors()) {
			ImGui::PushID(i);

			auto actorName = actor.name();

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
				memset(buf, 0, renameBufSize);
				memcpy(buf, actorName.c_str(), std::min<size_t>(actorName.size(), renameBufSize));
				if (ImGui::InputText("##ActorRenameInput", buf, renameBufSize, ImGuiInputTextFlags_EnterReturnsTrue)) {
					actor.rename(buf);
					m_renameActorIndex = 0xFFFFFFFF;
				}
				ImGui::SetItemDefaultFocus();
				ImGui::SetKeyboardFocusHere(-1);
				if(
					(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemClicked(ImGuiMouseButton_Left)) ||
					(ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !ImGui::IsItemClicked(ImGuiMouseButton_Right))
					)
					m_renameActorIndex = 0xFFFFFFFF;
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
					auto mesh = *reinterpret_cast<Zap::AssetHandle<Zap::Mesh>*>(payload->Data);
					if (mesh) {
						if (!actor.hasModel()) {
							actor.addModel({ {mesh}, {m_pEditorData->defaultMaterial}, {glm::mat4(1)} });
						}
						else {
							actor.cmpModel_addMesh(mesh, m_pEditorData->defaultMaterial, glm::mat4(1));
						}
					}
				}
				ImGui::EndDragDropTarget();
			}

			if (selected) ImGui::PopStyleColor(2);

			if (ImGui::IsItemHovered())
				m_hoveredActorIndex = i;
			i++;

			ImGui::PopID();
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
				m_actorCreationData = {}; // setup one time data for actor creation
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
					if (!m_actorCreationData.createName)
						m_actorCreationData.name = "";
					Zap::Actor actor = scene().createActor(m_actorCreationData.name);
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
					scene().destroyActor(m_hoveredActorIndex);
					ImGui::CloseCurrentPopup();
				}
			}

			if (m_hoveredActorIndex < 0xFFFFFFFF) {
				if (ImGui::Button("Save")) {
					Zap::Actor actor = scene().actors()[m_hoveredActorIndex];
					//if (m_pEditorData->actorPathMap.count(actor))
					//	saveActorFile(m_pEditorData->actorPathMap.at(actor), actor, *m_pEditorData);
					//else {
						m_actorSaveData = {};
						m_actorSaveData.actor = actor;
						ImGui::OpenPopup("ActorSave##Popup");
					//}
				}
			}

			if (ImGui::BeginPopup("ActorSave##Popup")) {
				ImGui::InputText("filepath", m_actorSaveData.pathInputBuffer, m_actorSaveData.pathInputSize);
				if (ImGui::Button("Done") || ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
					//saveActorFile(m_actorSaveData.pathInputBuffer, m_actorSaveData.actor, *m_pEditorData);
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

	void SceneHierarchyView::changeScene(SceneReference& lastScene){}

	ImGuiWindowFlags SceneHierarchyView::getWindowFlags() {
		return 0;
	}
}