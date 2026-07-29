#pragma once

#include "ZapEditor.h"

#include "Zap/Zap.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Components/Transform.h"

#include "ViewLayer.h"

namespace editor {
	class SceneHierarchyView : public SceneAccessLayer
	{
	public:
		SceneHierarchyView(EditorData* pEditorData);
		~SceneHierarchyView();

		virtual std::string name() override;

		virtual void draw() override;

		virtual void changeScene(SceneReference& lastScene) override;

		ImGuiWindowFlags getWindowFlags() override;

	private:
		EditorData* m_pEditorData = nullptr;

		uint32_t m_hoveredActorIndex = 0xFFFFFFFF;
		uint32_t m_renameActorIndex = 0xFFFFFFFF;

		struct ActorCreationData {
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

