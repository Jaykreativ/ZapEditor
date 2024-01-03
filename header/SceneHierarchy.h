#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Actor.h"

namespace editor {
	class SceneHierarchyView {
	public:
		SceneHierarchyView(std::vector<Zap::Actor>& actors);
		~SceneHierarchyView();

		void draw();

		Zap::Actor* getSelectedActor();

	private:
		std::vector<Zap::Actor>& m_actors;
		Zap::Actor* m_selected = nullptr;
	};
}

