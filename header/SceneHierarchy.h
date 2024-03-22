#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Actor.h"

#include "ViewLayer.h"

namespace editor {
	class SceneHierarchyView : public ViewLayer
	{
	public:
		SceneHierarchyView(std::vector<Zap::Actor>& allActors, std::vector<Zap::Actor>& selectedActors);
		~SceneHierarchyView();

		std::string name();

		void draw();

		ImGuiWindowFlags getWindowFlags();

	private:
		std::vector<Zap::Actor>& m_allActors;
		std::vector<Zap::Actor>& m_selectedActors;
	};
}

