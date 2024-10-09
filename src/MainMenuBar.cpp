#include "MainMenuBar.h"

#include "Viewport.h"
#include "SceneHierarchy.h"
#include "ComponentView.h"
#include "AssetBrowser.h"
#include "HitboxEditor.h"

#include "imgui.h"

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
				m_layers.push_back(new Viewport(m_pScene, m_pWindow, m_selectedActors));
			}
			if (ImGui::MenuItem("AssetBrowser")) {
				m_layers.push_back(new AssetBrowser(m_pWindow, m_pGui));
			}
			if (ImGui::MenuItem("HitboxEditor")) {
				m_layers.push_back(new HitboxEditor(m_pEditorData));
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	bool MainMenuBar::shouldSimulate() {
		return m_shouldSimulate;
	}
}