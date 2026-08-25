#include "MainMenuBar.h"

#include "Viewport.h"
#include "SceneHierarchy.h"
#include "ComponentView.h"
#include "AssetBrowser.h"
#include "Settings.h"
#include "HitboxEditor.h"
#include "ProjectHandling.h"
#include "FileHandling.h"

#include "Zap/Serializer.h"

#include "imgui.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <chrono>

namespace editor {
	MainMenuBar::MainMenuBar(
		EditorData* pEditorData,
		std::vector<ViewLayer*>& layers,
		Zap::Window* pWindow,
		Zap::Renderer* pRenderer,
		std::vector<Zap::Actor>& selectedActors
	)
		:
		m_pEditorData(pEditorData),
		m_layers(layers),
		m_pWindow(pWindow),
		m_pRenderer(pRenderer), 
		m_selectedActors(selectedActors)
	{}

	MainMenuBar::~MainMenuBar() {}

	void MainMenuBar::draw() {
		ImGui::BeginMainMenuBar();
		if (m_shouldSimulate) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5, 0, 0, 0.5));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5, 0, 0, 1));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9, 0.1, 0.1, 1));
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0.5, 0, 0.5));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0.5, 0, 1));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1, 0.9, 0.1, 1));
		}
		if (ImGui::Button("Simulate")) {
			m_shouldSimulate = !m_shouldSimulate;
		}
		ImGui::PopStyleColor(3);
		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("SceneHierarchy")) {
				m_layers.push_back(new SceneHierarchyView(m_pEditorData));
			}
			if (ImGui::MenuItem("ComponentView")) {
				m_layers.push_back(new ComponentView(m_pEditorData, m_layers, m_selectedActors));
			}
			if (ImGui::MenuItem("Viewport")) {
				m_layers.push_back(new Viewport(*m_pEditorData, m_pWindow));
			}
			if (ImGui::MenuItem("AssetBrowser")) {
				m_layers.push_back(new AssetBrowser(*m_pEditorData));
			}
			if (ImGui::MenuItem("HitboxEditor")) {
				m_layers.push_back(new HitboxEditor(m_pEditorData));
			}
			if (ImGui::MenuItem("Settings")) {
				m_layers.push_back(new Settings(m_pEditorData));
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Project")) {
			if(m_pEditorData->project.name != "")
				ImGui::Text(m_pEditorData->project.name.c_str());
			if (!m_pEditorData->project.rootPath.empty())
				ImGui::Text(m_pEditorData->project.projectFile.string().c_str());
			if (!m_pEditorData->project.editorFile.empty())
				ImGui::Text(m_pEditorData->project.editorFile.string().c_str());

			// Create Project
			if (ImGui::Button("Create")) {
				ImGui::OpenPopup("ProjectCreationPopup");
			}

			if (ImGui::BeginPopup("ProjectCreationPopup")) {
				const size_t nameBufferSize = 50;
				static char nameBuffer[nameBufferSize] = {};
				ImGui::InputText("Name", nameBuffer, nameBufferSize);

				const size_t dirBufferSize = 150;
				static char dirBuffer[dirBufferSize] = {};
				ImGui::InputText("Directory", dirBuffer, dirBufferSize);

				if (ImGui::Button("Done")) {
					//project::create(*m_pEditorData, nameBuffer, dirBuffer);
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			//Open Project
			if (ImGui::Button("Open")) {
				ImGui::OpenPopup("ProjectOpenPopup");
			}

			if (ImGui::BeginPopup("ProjectOpenPopup")) {
				const size_t fileBufferSize = 150;
				static char fileBuffer[fileBufferSize] = {};
				ImGui::InputText("Filepath", fileBuffer, fileBufferSize);

				std::filesystem::path filepath = fileBuffer;
				std::string directory = filepath.parent_path().string();
				std::string name = filepath.filename().string();
				std::string extension = filepath.extension().string();

				bool isValidPath =
					extension == projectFileExtension ||
					extension == projectEditorFileExtension;

				if (FILE* file = fopen((directory + "/" + name + "." + extension).c_str(), "r")) {
					fclose(file);
				}
				else
					isValidPath = false;

				if (!isValidPath)
					ImGui::BeginDisabled();

				if (ImGui::Button("Done")) {
					//project::open(*m_pEditorData, name, directory);
					ImGui::CloseCurrentPopup();
				}

				if (!isValidPath)
					ImGui::EndDisabled();

				ImGui::EndPopup();
			}

			bool disabled = !m_pEditorData->project.isOpen;
			if (disabled)
				ImGui::BeginDisabled();
			//Close Project
			if (ImGui::Button("Close")) {
				//project::close(*m_pEditorData);
			}

			// Save Project
			if (ImGui::Button("Save")) {
				//project::save(*m_pEditorData);
			}
			if (disabled)
				ImGui::EndDisabled();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Scene")) {
			bool closePopupTree = false;
			if (ImGui::Button("Load"))
				ImGui::OpenPopup("LoadScene");
			if (ImGui::BeginPopup("LoadScene")) {
				static char buf[150];
				ImGui::InputText("path", buf, 150);
				if (ImGui::Button("Load")) {
					SceneHandler::LoadFailFlags fail;
					auto sceneRef = m_pEditorData->pSceneHandler->load(buf, &fail);
					if (!fail) {
						sceneRef->updatePxPoses();
						sceneRef->update();
					}
					if (ZP_IS_FLAG_ENABLED(fail, SceneHandler::eInvalidFilepath))
						ImGui::OpenPopup("InvalidSceneFilepath");
					if (ZP_IS_FLAG_ENABLED(fail, SceneHandler::eDuplicate))
						ImGui::OpenPopup("DuplicateSceneLoad");
				}
				if (ImGui::BeginPopupModal("InvalidSceneFilepath", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::Text("filepath cannot be opened");
					ImGui::Text("path: \"%s\" is invalid", buf);
					if (ImGui::Button("Back"))
						ImGui::CloseCurrentPopup();
					ImGui::SameLine();
					if (ImGui::Button("Cancel"))
						closePopupTree = true;
					if (closePopupTree) ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
				}
				if (ImGui::BeginPopupModal("DuplicateSceneLoad", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::Text("this scene is already loaded");
					ImGui::Text("create a new copy of the scene?");
					if (ImGui::Button("Back"))
						ImGui::CloseCurrentPopup();
					ImGui::SameLine();
					if (ImGui::Button("Cancel"))
						closePopupTree = true;
					ImGui::SameLine();
					if (ImGui::Button("Copy(WIP)")); // TODO add scene copy functionality
					if (closePopupTree) ImGui::CloseCurrentPopup(); 
					ImGui::EndPopup();
				}
				if (closePopupTree) ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
			if (ImGui::Button("Save"))
				ImGui::OpenPopup("SaveAllScenes");
			if (ImGui::BeginPopup("SaveAllScenes")) {
				ImGui::Text("WIP save all");
				ImGui::EndPopup();
			}
			ImGui::Separator();
			for (auto it = m_pEditorData->pSceneHandler->begin(); it != m_pEditorData->pSceneHandler->end(); it++) {
				ImGui::PushID(it);
				if ((int)it == m_renameIndex) {
					static const size_t renameBufSize = 50;
					static char buf[renameBufSize] = "";
					memset(buf, 0, renameBufSize);
					std::string name = m_pEditorData->pSceneHandler->getName(it);
					memcpy(buf, name.c_str(), std::min<size_t>(renameBufSize-1, name.size()));
					if (ImGui::InputText("##RenameInput", buf, renameBufSize, ImGuiInputTextFlags_EnterReturnsTrue)) {
						m_pEditorData->pSceneHandler->getReference(it)->rename(buf);
						m_renameIndex = -1;
					}
					ImGui::SetItemDefaultFocus();
					ImGui::SetKeyboardFocusHere(-1);
					if (
						(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemClicked(ImGuiMouseButton_Left)) ||
						(ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !ImGui::IsItemClicked(ImGuiMouseButton_Right))
						)
						m_renameIndex = -1;
				}
				else if (ImGui::MenuItem(m_pEditorData->pSceneHandler->getName(it).c_str(), nullptr, m_pEditorData->pSceneHandler->isActive(it))) {
					m_pEditorData->pSceneHandler->activate(it);
				}
				ImGui::SetItemTooltip("use this scene as the shared active scene");
				if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					ImGui::OpenPopup("SceneSelectionPopup");
				if (ImGui::BeginPopup("SceneSelectionPopup")) {
					if (ImGui::Button("Save"))
						ImGui::OpenPopup("SaveScene");
					if (ImGui::BeginPopup("SaveScene")) {
						static char buf[150];
						ImGui::InputText("path", buf, 150);
						if (ImGui::Button("Save")) {
							m_pEditorData->pSceneHandler->save(buf, it);
							closePopupTree = true;
						}
						if (closePopupTree) ImGui::CloseCurrentPopup();
						ImGui::EndPopup();
					}
					if (ImGui::Button("Rename")) {
						m_renameIndex = it;
						closePopupTree = true;
					}
					if (closePopupTree) ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
				}
				ImGui::PopID();
			}
			if (ImGui::Button("+")) {
				m_pEditorData->pSceneHandler->create("NewScene");
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	bool MainMenuBar::shouldSimulate() {
		return m_shouldSimulate;
	}
}