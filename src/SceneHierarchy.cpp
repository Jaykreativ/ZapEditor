#define ZP_ENTITY_COMPONENT_SYSTEM_ACCESS

#include "SceneHierarchy.h"

#include "Zap/Scene/Scene.h"

#include "imgui.h"
#include <iostream>
#include <string>

namespace editor {
	SceneHierarchyView::SceneHierarchyView(std::vector<Zap::Actor>& allActors, std::vector<Zap::Actor>& selectedActors)
		: m_allActors(allActors), m_selectedActors(selectedActors)
	{}

	SceneHierarchyView::~SceneHierarchyView(){}

	std::string SceneHierarchyView::name() {
		return "SceneHierarchyView";
	}

	void SceneHierarchyView::draw() {
		ImGui::Text(std::to_string(m_allActors.size()).c_str());
		for (Zap::Actor actor : m_allActors) {
			if (ImGui::Button(std::to_string(actor.m_handle).c_str())) {
				m_selectedActors.clear();
				m_selectedActors.push_back(actor);
			}
		}
	}

	ImGuiWindowFlags SceneHierarchyView::getWindowFlags() {
		return 0;
	}
}