#include "ViewLayer.h"

namespace editor {
	SceneAccessLayer::SceneAccessLayer(SceneHandler& sceneHandler)
		: m_sceneHandler(sceneHandler)
	{
		m_sceneRef = std::make_unique<ActiveSceneReference>(m_sceneHandler.getActiveReference());
	}

	void SceneAccessLayer::draw() {
		if (ImGui::BeginMenu("Scene")) {
			bool isActive = m_isActive;
			if (ImGui::MenuItem(("active(" + m_sceneHandler.getActiveName() + ")").c_str(), nullptr, isActive)) {
				m_sceneRef = std::make_unique<ActiveSceneReference>(m_sceneHandler.getActiveReference());
				m_isActive = true;
			}
			ImGui::SetItemTooltip("use the shared active scene of the editor set in the main menu bar");
			ImGui::Separator();
			for (SceneIterator it = m_sceneHandler.begin(); it != m_sceneHandler.end(); it++) {
				ImGui::PushID(it);
				bool isSelected = m_sceneHandler.getReference(it) == *m_sceneRef && !m_isActive;
				if (ImGui::MenuItem(m_sceneHandler.getName(it).c_str(), nullptr, isSelected)) {
					m_sceneRef = std::make_unique<CustomSceneReference>(m_sceneHandler.getReference(it));
					m_isActive = false;
				}
				ImGui::SetItemTooltip("use this scene regardless of what scene is currently active");
				ImGui::PopID();
			}
			ImGui::EndMenu();
		}
	}

	SceneReference& SceneAccessLayer::scene() {
		return *m_sceneRef;
	}
}