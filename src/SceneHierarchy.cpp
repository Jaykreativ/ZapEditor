#define ZP_ENTITY_COMPONENT_SYSTEM_ACCESS

#include "SceneHierarchy.h"

#include "Zap/Scene/Scene.h"

#include "imgui.h"
#include <iostream>
#include <string>

namespace editor {
	SceneHierarchyView::SceneHierarchyView(std::vector<Zap::Actor>& actors)
		: m_actors(actors)
	{}

	SceneHierarchyView::~SceneHierarchyView(){}

	void SceneHierarchyView::draw() {
		ImGui::Text(std::to_string(m_actors.size()).c_str());
		for (Zap::Actor actor : m_actors) {
			if (ImGui::Button(std::to_string(actor.m_handle).c_str())) {
				if (m_selected) delete m_selected;
				m_selected = new Zap::Actor(actor);
			}
		}
	}

	Zap::Actor* SceneHierarchyView::getSelectedActor() {
		return m_selected;
	}

}