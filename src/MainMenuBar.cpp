#include "MainMenuBar.h"

#include "Viewport.h"
#include "SceneHierarchy.h"
#include "ComponentView.h"
#include "AssetBrowser.h"
#include "HitboxEditor.h"
#include "ProjectHandling.h"

#include "Zap/Serializer.h"

#include "imgui.h"

#include <chrono>

namespace editor {
	MainMenuBar::MainMenuBar(
		EditorData* pEditorData,
		std::vector<ViewLayer*>& layers,
		Zap::Window* pWindow,
		Zap::Renderer* pRenderer,
		Zap::Gui* pGui,
		Zap::Scene* pScene,
		std::vector<Zap::Actor>& actors,
		std::vector<Zap::Actor>& selectedActors
	)
		:
		m_pEditorData(pEditorData),
		m_layers(layers),
		m_pWindow(pWindow),
		m_pRenderer(pRenderer), 
		m_pGui(pGui), 
		m_pScene(pScene), 
		m_actors(actors),
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
				m_layers.push_back(new SceneHierarchyView(m_pEditorData, m_pScene, m_actors, m_selectedActors));
			}
			if (ImGui::MenuItem("ComponentView")) {
				m_layers.push_back(new ComponentView(m_pEditorData, m_layers, m_selectedActors));
			}
			if (ImGui::MenuItem("Viewport")) {
				m_layers.push_back(new Viewport(*m_pEditorData, m_pScene, m_pWindow));
			}
			if (ImGui::MenuItem("AssetBrowser")) {
				m_layers.push_back(new AssetBrowser(m_pWindow, m_pGui));
			}
			if (ImGui::MenuItem("HitboxEditor")) {
				m_layers.push_back(new HitboxEditor(m_pEditorData));
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Project")) {
			if(m_pEditorData->project.name != "")
				ImGui::Text(m_pEditorData->project.name.c_str());
			if (m_pEditorData->project.fileDir != "")
				ImGui::Text(m_pEditorData->project.fileDir.c_str());
			if (m_pEditorData->project.editorFileDir != "")
				ImGui::Text(m_pEditorData->project.editorFileDir.c_str());

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
					project::create(*m_pEditorData, nameBuffer, dirBuffer);
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

				std::string filepath = fileBuffer;
				auto end = filepath.size();
				auto dot = std::min<size_t>(filepath.find_first_of('.'), end);
				auto slash = filepath.find_last_of('/');
				auto backslash = filepath.find_last_of('\\');
				if (slash >= end) slash = 0;
				if (backslash >= end) backslash = 0;
				auto nameFirst = std::min<size_t>(std::max<size_t>(slash, backslash) + 1, dot);
				auto dirLast = std::max<size_t>(nameFirst - 1, 0);

				std::string directory = filepath.substr(0, dirLast);
				std::string name = filepath.substr(nameFirst, dot - nameFirst);
				std::string type = filepath.substr(dot, end - dot);

				std::cout << "dir: " << directory << "\n";
				std::cout << "name: " << name << "\n";
				std::cout << "type: " << type << "\n";

				bool isValidPath =
					type == ".zproj" ||
					type == ".zproj.edit";

				if (FILE* file = fopen((directory + "/" + name + type).c_str(), "r")) {
					fclose(file);
				}
				else
					isValidPath = false;

				if (!isValidPath)
					ImGui::BeginDisabled();

				if (ImGui::Button("Done")) {
					project::open(*m_pEditorData, name, directory);
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
				project::close(*m_pEditorData);
			}

			// Save Project
			if (ImGui::Button("Save")) {
				project::save(*m_pEditorData);
			}
			if (disabled)
				ImGui::EndDisabled();
			
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	bool MainMenuBar::shouldSimulate() {
		return m_shouldSimulate;
	}
}