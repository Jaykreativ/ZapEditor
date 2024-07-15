#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Transform.h"

#include "ViewLayer.h"

namespace editor {
	class SceneHierarchyView : public ViewLayer
	{
	public:
		SceneHierarchyView(Zap::Scene* pScene,std::vector<Zap::Actor>& allActors, std::vector<Zap::Actor>& selectedActors);
		~SceneHierarchyView();

		std::string name() override;

		void draw() override;

		ImGuiWindowFlags getWindowFlags();

	private:
		Zap::Scene* m_pScene;
		std::vector<Zap::Actor>& m_allActors;
		uint32_t m_hoveredActorIndex = 0xFFFFFFFF;
		std::vector<Zap::Actor>& m_selectedActors;

		struct ActorCreationData {
			bool createTransform = true;
			Zap::Transform transform = {};
		};
		ActorCreationData m_actorCreationData = {};
	};
}

