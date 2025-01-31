#pragma once

#include "ZapEditor.h"

#include "Zap/Zap.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Transform.h"

#include "ViewLayer.h"

namespace editor {
	class SceneHierarchyView : public ViewLayer
	{
	public:
		SceneHierarchyView(EditorData* pEditorData, Zap::Scene* pScene);
		~SceneHierarchyView();

		std::string name() override;

		void draw() override;

		ImGuiWindowFlags getWindowFlags() override;

	private:
		EditorData* m_pEditorData = nullptr;
		Zap::Scene* m_pScene;
		uint32_t m_hoveredActorIndex = 0xFFFFFFFF;

		struct ActorCreationData {
			Zap::Actor newActor;
			bool createName = true;
			char nameInputBuffer[50] = "";
			std::string name = {};
			bool createTransform = true;
			Zap::Transform transform = {};
		} m_actorCreationData;

		struct ActorSaveData {
			Zap::Actor actor;
			static const size_t pathInputSize = 256;
			char pathInputBuffer[pathInputSize] = "";
		} m_actorSaveData;
	};
}

